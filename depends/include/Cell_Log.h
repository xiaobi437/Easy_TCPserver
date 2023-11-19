/*
* 任务日志系统
* 
*/
#ifndef CELL_LOG_H
#define CELL_LOG_H

//#include "Public_Include.h"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <chrono>
#include "Cell_Task.h"






constexpr short  Info_Msg = 0;
constexpr short  Debug_Msg = 1;
constexpr short  Warning_Msg = 2;
constexpr short  Error_Msg = 3;

#ifdef _DEBUG
	#ifndef Debug_Printf
	#include<stdio.h>
	#define Debug_Printf(...) printf(__VA_ARGS__)
	#endif
#else
	#ifndef Debug_Printf
	#define Debug_Printf(...)
	#endif // !Debug_Printf
#endif // _DEBUG


class Cell_Log
{

private:
	Cell_Log() 
	{
		_TaskServer.Start();
		_msg_Level = Info_Msg;
		_date = new char[10];
		_time = new char[10];
	}
	~Cell_Log() 
	{
		_TaskServer.Close();
		if (_logFile)
		{
			Info(Info_Msg,"Cell_Log:: ~ Cell_Log end()\n");
			fclose(_logFile);
			fclose(_logErrorFile);
			_logFile = nullptr;
			_logErrorFile = nullptr;
		}
		delete[] _date;
		delete[] _time;
	}
public:
	static Cell_Log& Instance()
	{
		static Cell_Log sLog;
		return sLog;
	}
	char* get_Now_Date()
	{
		//获取系统当前时间
		auto t = std::chrono::system_clock::now();
		auto tNow = std::chrono::system_clock::to_time_t(t);
		std::tm* now = std::gmtime(&tNow);
		sprintf(_date, "%d.%d.%d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
		return _date;
	}
	char* get_Now_Time()
	{
		//获取系统当前时间
		auto t = std::chrono::system_clock::now();
		auto tNow = std::chrono::system_clock::to_time_t(t);
		std::tm* now = std::gmtime(&tNow);
		sprintf(_time, "%d:%d:%d", now->tm_hour, now->tm_min, now->tm_sec);
		return _time;
	}
	bool setLogPath(const char* FilePath,const char* mode)
	{
		if (_logFile)
		{
			Info(Error_Msg,"Cell_Log::_logFile !=nullptr \n");
			fclose(_logFile);
			_logFile = nullptr;
		}	
		_logFile = fopen(FilePath, mode);
		if (!_logFile)
		{
			Info(Error_Msg,"Cell_Log:: %s open %s failed\n", FilePath, mode);
			return false;
		}	
		return true;
	}
	bool setErrorLogPath(const char* FilePath, const char* mode)
	{
		if (_logErrorFile)
		{
			Info(Error_Msg, "Cell_Log::_logErrorFile !=nullptr \n");
			fclose(_logErrorFile);
			_logErrorFile = nullptr;
		}
		_logErrorFile = fopen(FilePath, mode);
		if (!_logErrorFile)
		{
			Info(Error_Msg, "Cell_Log:: %s open %s failed\n", FilePath, mode);
			return false;
		}
		return true;
	}
	//msg_Level:Info_Msg Debug_Msg Warning_Msg Error_Msg
	static void Info(short msg_Level, const char* pStr)
	{
		Cell_Log* pLog = &Instance();
		switch (msg_Level)
		{
			case Info_Msg:
			{
				printf("%s", pStr);
				break;
			}
			case Debug_Msg:
			{
				Debug_Printf("%s", pStr);
				break;
			}
			case Warning_Msg:
			{
				if (pLog->_logFile)
				{
					pLog->_TaskServer.addTask([pLog, pStr]() {
						fprintf(pLog->_logFile, "[%s][%s]<Warning>: ", pLog->get_Now_Date(), pLog->get_Now_Time());
						fprintf(pLog->_logFile, "%s", pStr);
						fflush(pLog->_logFile);
						});
				}
				else
				{
					printf("Cell_Log::_logFile !=nullptr \n");
				}
				break;
			}
			case Error_Msg:
			{
				if (pLog->_logErrorFile)
				{
					pLog->_TaskServer.addTask([pLog, pStr]() {
						fprintf(pLog->_logErrorFile, "[%s][%s]<Error>: ", pLog->get_Now_Date(), pLog->get_Now_Time());
						fprintf(pLog->_logErrorFile, "%s", pStr);
						fflush(pLog->_logErrorFile);
					});
				}
				else
				{
					printf("Cell_Log::_logFile !=nullptr \n");
				}
				break;
			}
			default:
				break;
		}
	}
	//msg_Level:Info_Msg Debug_Msg Warning_Msg Error_Msg
	template<typename ...Args>
	static void Info(short msg_Level, const char* pStr,Args ... args)
	{
		Cell_Log* pLog = &Instance();
		switch (msg_Level)
		{
		case Info_Msg:
		{
			printf(pStr, args...);
			break;
		}
		case Debug_Msg:
		{
			Debug_Printf(pStr, args...);
			break;
		}
		case Warning_Msg:
		{
			if (pLog->_logFile)
			{
				pLog->_TaskServer.addTask([pLog, pStr, args...]() {
					fprintf(pLog->_logFile, "[%s][%s]<Warning>: ", pLog->get_Now_Date(), pLog->get_Now_Time());
					fprintf(pLog->_logFile, pStr, args...);
					fflush(pLog->_logFile);
				});
			}
			else
			{
				printf("Cell_Log::_logFile !=nullptr \n");
			}
			break;
		}
		case Error_Msg:
		{
			if (pLog->_logErrorFile)
			{
				pLog->_TaskServer.addTask([pLog, pStr, args...]() {
					fprintf(pLog->_logErrorFile, "[%s][%s]<Error>: ", pLog->get_Now_Date(), pLog->get_Now_Time());
					fprintf(pLog->_logErrorFile, pStr, args...);
					fflush(pLog->_logErrorFile);
				});
			}
			else
			{
				printf("Cell_Log::_logFile !=nullptr \n");
			}
			break;
		}
		default:
			break;
		}
	}
private:
	CellTaskServer _TaskServer;
	FILE* _logFile = nullptr;
	FILE* _logErrorFile = nullptr;
	short _msg_Level;
	char* _date;		//日期
	char* _time;		//时间
};

#endif // !CELL_LOG_H

