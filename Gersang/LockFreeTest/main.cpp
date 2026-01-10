#include "stdafx.h"
#include "lockfree_queue.h"

using namespace std;

LockFreeQueue q;

void PushThread(int a)
{
	for (int i = a; i < a + 100000; i++)
	{
		q.Push(i + 1);
	}
}

void PopThread()
{
	for (int i = 0; i < 100000; i++)
	{
		int a = q.Pop();
		if (a == 0) { // 비어 있는 경우
			i--;
			continue;
		}
	}
}

int main()
{
	thread t1(PushThread, 0);
	thread t2(PushThread, 100000);
	thread t3(PopThread);
	thread t4(PopThread);

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	q.Print();

	return 0;
}
