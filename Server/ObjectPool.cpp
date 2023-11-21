#include "Object_Pool.h"

#if 0
class ClassA :public ObjectPoolBase<ClassA,1>
{
public:
	ClassA(int n) {
		num = n;
		printf("classA%d\n", num);
	}
	~ClassA() {
		printf("~classA%d\n", num);
	}
private:
	int num;
	char data[1024000];
};

class ClassB :public ObjectPoolBase<ClassB,10>
{
public:
	ClassB(int n,int m) {
		num = n;
		num1 = m;
		printf("classB%d,%d\n", num, num1);
	}
	~ClassB() {
		printf("~classB%d,%d\n", num, num1);
	}
private:
	int num;
	int num1;
};
int main() 
{
	ClassA* a1 = ClassA::createObject(1);
	ClassA* a2 = ClassA::createObject(2);
	ClassA* a3 = ClassA::createObject(3);
	ClassA::destroyObject(a1);
	ClassA::destroyObject(a2);
	ClassA::destroyObject(a3);

	//ClassB* b1 = ClassB::createObject(1, 1);
	//ClassB* b2 = ClassB::createObject(2, 2);
	//ClassB* b3 = ClassB::createObject(3, 3);
	//ClassB::destroyObject(b1);
	//ClassB::destroyObject(b2);
	//ClassB::destroyObject(b3);
	return 0;
}
#endif


