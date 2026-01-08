#include "stdafx.h"
#include "lockfree_queue.h"

int main()
{
	LockFreeQueue q;
	
	for (int i = 0; i < 10000; i++)
	{
		q.Push(i + 1);
		if (rand() % 10 > 5) {
			q.Pop();
		}
	}

	for (int i = 0; i < 10000; i++)
	{
		int a = q.Pop();
		if (a == 0) {
			std::cout << "iterate : " << i + 1 << std::endl;
			break;
		}

		std::cout << a << std::endl;
		
	}
	return 0;
}
