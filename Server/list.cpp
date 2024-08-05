#include <iostream>

using namespace std;

class A
{
public:
	A(int a) :a(a) {}
	//运算符+重载
	/*A operator+(A a) {
		return A(this->a + a.a);
	}*/
	friend ostream& operator<< (ostream& out, A a);
private:
	int a;
};
//输出重载
ostream& operator<< (ostream& out, A a)
{
	return cout << "(" << a.a << ")" << endl;
}

//函数重载
/*
int add(int a, int b) {
	return a + b;
}
double add(double a, double b) {
	return a + b;
}
*/
/*
正常来说要实现不同类型的相加要写不同的重载函数
模板函数解决了，实现同样功能，又省代码的问题
模板函数，就是一个高级的重载函数，他的返回类型和输入类型是万能的，可以自动匹配类型
*/

template<typename T>
T add(T a, T b)
{
	return a + b;
}
#if 0
int main(int argc, char* argv[])
{
	//正常写法
	/*
	cout << add<int>(1, 1) << endl;
	cout << add<double>(1.1, 1.1) << endl;
	cout << add<A>(A(1), A(1)) << endl;
	*/

	cout << add(1, 1) << endl;
	cout << add(1.1, 1.1) << endl;
	cout << add(A(1), A(1)) << endl;

	return 0;
}
#endif