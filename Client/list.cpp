#include <iostream>

using namespace std;

class A
{
public:
	A(int a) :a(a) {}
	//�����+����
	/*A operator+(A a) {
		return A(this->a + a.a);
	}*/
	friend ostream& operator<< (ostream& out, A a);
private:
	int a;
};
//�������
ostream& operator<< (ostream& out, A a)
{
	return cout << "(" << a.a << ")" << endl;
}

//��������
/*
int add(int a, int b) {
	return a + b;
}
double add(double a, double b) {
	return a + b;
}
*/
/*
������˵Ҫʵ�ֲ�ͬ���͵����Ҫд��ͬ�����غ���
ģ�庯������ˣ�ʵ��ͬ�����ܣ���ʡ���������
ģ�庯��������һ���߼������غ��������ķ������ͺ��������������ܵģ������Զ�ƥ������
*/

template<typename T>
T add(T a, T b)
{
	return a + b;
}
#if 0
int main(int argc, char* argv[])
{
	//����д��
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