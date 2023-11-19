/*
*Cell_Server 由主Server创建出的线程 子服务端
*用于消费从Server生产出的客户端（处理客户端的消息）
*/
#ifndef CELL_SERVER_H
#define CELL_SERVER_H

#include "Public_Include.h"
#include <mutex>
#include <thread>
#include <map>
#include <vector>

#include "Cell_Task.h"
#include "Cell_Client.h"
#include "Net_Event.h"
#include "Timestamp.h"
#include "Cell_Semaphore.h"
#include "Cell_Log.h"



class Cell_Server
{
private:
	std::mutex _mutex;			//针对缓冲队列的锁
	SOCKET _max_sock;			//最大socket描述符
	fd_set _fd_Read_bak;		//伯克利socket描述符合集备份

	std::map<SOCKET, Cell_Client*> _clients;			//正式客户端队列
	std::vector<Cell_Client*> _clientsBuff;			//缓存客户端队列
	NetEvent* _pNetEvent;							//网络事件

	time_t _oldTime;		//心跳检测、定时发送

	CellTaskServer _taskServer;						//任务事件
	Cell_Thread _thread;						//线程
	int _id;
	
	bool _client_change;		//伯克利socket描述符合集 修改标志位
public:
	Cell_Server(NetEvent* pNetEvent,int id)
	{
		_id = id;
		_pNetEvent = pNetEvent;
		_oldTime = Cell_Time::getNowMilliSec();
		_client_change = true;
	}
	~Cell_Server()
	{
		Cell_Log::Info(Info_Msg,"Cell_Server%d.~Cell_Server exit begin\n", _id);
		Close();
		Cell_Log::Info(Info_Msg, "Cell_Server%d.~Cell_Server exit end\n", _id);
	}
	//关闭socket
	int Close()
	{
		Cell_Log::Info(Info_Msg, "Cell_Server%d.Close() begin\n",_id);
		_taskServer.Close();
		_thread.Close();
		Cell_Log::Info(Info_Msg, "Cell_Server%d.Close() end\n", _id);
		return 0;
	}
	void ClearClients()
	{
		printf("Cell_Server%d.ClearClients() end\n", _id);
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}

	void CheckTime()
	{
		auto nowTime = Cell_Time::getNowMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			//心跳检测
			if (iter->second->checkHeart(dt))
			{
				Cell_Log::Info(Debug_Msg,  "socket<%d> Heart dead\n", iter->second->sockfd());
				OnClientLeave(iter->second);
				auto iterOld = iter++;			//先使用在自增
				_clients.erase(iterOld);
				continue;
			}
			//定时发送
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	//客户端退出
	void OnClientLeave(Cell_Client* pClient)
	{
		//Cell_Log::Info(Info_Msg,"socket<%d> OnClientLeave() %x\n", pClient->sockfd(),pClient);
		if (_pNetEvent)
			_pNetEvent->OnNetLeave();	//触发退出事件
		_client_change = true;			//当有客户端退出，就修改标志位
		delete pClient;
	}
	//写入消息（上层函数）
	void writeData(fd_set& fd_Write)
	{
#ifdef _WIN32
		for (int n = 0; n < (int)fd_Write.fd_count; n++)
		{
			auto iter = _clients.find(fd_Write.fd_array[n]);
			if (iter != _clients.end())
			{
				//定时发送 在异步情况下，看业务需求
				//iter->second->checkSend(dt);
				if (iter->second->sendDataReal() == -1)
				{
					Cell_Log::Info(Error_Msg, "socket<%d> Leave sendDataReal()\n", iter->second->sockfd());
					OnClientLeave(iter->second);
					_clients.erase(iter);			//从map中删除
				}
			}
		}

#else
		for (auto iter = _clients.begin(); iter!=_clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fd_Write))
			{
				if (iter->second->sendDataReal() == -1)
				{
					Cell_Log::Info(Warning_Msg,"socket<%d> Leave sendDataReal()\n", iter->second->sockfd());
					OnClientLeave(iter->second);
					auto iterOld = iter++;
					_clients.erase(iterOld);			//从map中删除
				}
			}
			iter++;
		}

#endif // !_WIN32
	}
	//读取消息（上层函数）
	void readData(fd_set& fd_Read)
	{
#ifdef _WIN32
		for (int n = 0; n < (int)fd_Read.fd_count; n++)
		{
			auto iter = _clients.find(fd_Read.fd_array[n]);
			if (iter != _clients.end())
			{
				if (recvData(iter->second) == -1)
				{
					Cell_Log::Info(Error_Msg,"socket<%d> Leave recvData()\n", iter->second->sockfd());
					OnClientLeave(iter->second);
					_clients.erase(iter);			//从map中删除
				}
			}
		}

#else
		for (auto iter = _clients.begin(); iter!=_clients.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fd_Read))
			{
				if (recvData(iter->second) == -1)
				{
					Cell_Log::Info(Warning_Msg,"socket<%d> Leave recvData()\n", iter->second->sockfd());
					OnClientLeave(iter->second);
					auto iterOld = iter++;
					_clients.erase(iterOld);			//从map中删除
				}
			}
			iter++;
		}

#endif // !_WIN32
	}
	//接收数据 处理粘包 拆分包
	int recvData(Cell_Client* p_clients)
	{
		
		//接收数据存到接收缓冲区，接收大小缓冲区大小（10KB）
		int nLen = p_clients->RecvData();
		if (nLen < 0) 
		{
			Cell_Log::Info(Error_Msg, "接收客户端<socket:%d>消息失败，客户端已退出....\n", (int)p_clients->sockfd());
			return -1;
		}
		//接收计数
		 _pNetEvent->onRecvCount(p_clients);
		//p_clients->resetDtHeart();

		//循环处理 判断一下消息缓存区的数据是否够一条消息
		while (p_clients->hasMsg())
		{		
			//处理网络消息
			onNetMsg(this, p_clients, p_clients->front_msg());
			//移除消息队列（缓冲区）最前的一条数据
			p_clients->pop_front_msg();
		}
		return 0;
	}
	// 处理包头
	virtual int onNetMsg(Cell_Server* pCell_Server, Cell_Client* p_clients, DataHeader* data_header)
	{
		_pNetEvent->onNetMsg(pCell_Server, p_clients, data_header);
		return 0;
	}
	//运行
	bool onRun(Cell_Thread* pThread)
	{
		Cell_Log::Info(Info_Msg, "Cell_Server%d. onRun() begin\n", _id);
		while (pThread->isRun())
		{

			if (!_clientsBuff.empty()) {
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					pClient->serverId = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_client_change = true;
			}
			//当没有需要处理的客户端，就跳过
			if (_clients.empty()) {
				std::chrono::milliseconds t(1);
				//让线程睡眠1ms
				std::this_thread::sleep_for(t);
				_oldTime = Cell_Time::getNowMilliSec();
				continue;
			}

			//创建 伯克利socket描述符合集
			fd_set fd_Read;		//读
			fd_set fd_Write;		//写
			fd_set fd_Exc;		//异常

			//初始化,清空
			FD_ZERO(&fd_Read);
			if (_client_change)
			{
				//初始化,清空
				FD_ZERO(&fd_Read);
				_client_change = false;
				_max_sock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fd_Read);
					//判断最大的文件描述符
					if (_max_sock < iter.second->sockfd())
					{
						_max_sock = iter.second->sockfd();
					}
				}
				memcpy(&_fd_Read_bak, &fd_Read, sizeof(fd_set));
			}
			else
			{
				memcpy(&fd_Read, &_fd_Read_bak, sizeof(fd_set));
			}
			memcpy(&fd_Write, &_fd_Read_bak, sizeof(fd_set));
			memcpy(&fd_Exc, &_fd_Read_bak, sizeof(fd_set));

			/*select 后 fd_Read 里面的值会改变，所以要备份*/
			//如果客户端数量多，fd_Write不能直接拷贝了，要做优化，可以循环单独找出可操作描述符。
			//fd_Exc一般来说很少使用，心跳包比较靠谱,在实时复杂的网络情况，获取异常不准确
			timeval t{ 0,1 };
			int ret = select(_max_sock + 1, &fd_Read, &fd_Write, &fd_Exc, &t);
			if (ret < 0) {
				Cell_Log::Info(Error_Msg, "Cell_Server%d. onRun() select failed!\n",_id);
				pThread->Exit();
				break;
			}
			//printf("Cell_Server%d. onRun() select client:%d,%d!\n", _id, fd_Read.fd_count, fd_Write.fd_count);
			//else if (ret == 0)   //超时就跳过当前循环
			//{
			//	continue;
			//}

			readData(fd_Read);
			writeData(fd_Write);
			writeData(fd_Exc);		//用读写都行
			/*
			printf("Cell_Server%d. onRun() fd_Read=%d，fd_Write=%d\n", _id, fd_Read.fd_count, fd_Write.fd_count);
			if (fd_Exc.fd_count > 0)
			{
				printf("Cell_Server%d. onRun() fd_Exc=%d###\n", _id, fd_Exc.fd_count);
			}
			*/
			CheckTime();
		}
		Cell_Log::Info(Info_Msg, "Cell_Server%d. onRun() end\n", _id);
		return true;
	}
	//添加客户端socket
	int addClient(Cell_Client* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
		return 0;
	}
	//获取客户端数量
	int getClinetCount()
	{
		//返回正式客户端队列数量+未处理的客户端数量
		return (int)_clients.size() + (int)_clientsBuff.size();
	}
	void Start()
	{
		_taskServer.Start(_id);
		_thread.Start(nullptr, [this](Cell_Thread* pThread) {
				onRun(pThread);
			}, 
			[this](Cell_Thread* pThread) {
				ClearClients();
			});
	}
	void addSendTask(Cell_Client* p_clients, DataHeader* data_header)
	{
		_taskServer.addTask([p_clients, data_header]() {
			p_clients->sendData(data_header);
			delete data_header;
			});	
	}
};

#endif // !CELL_SERVER

