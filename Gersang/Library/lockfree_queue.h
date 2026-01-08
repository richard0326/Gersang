#pragma once

//template<typename T>
class LockFreeQueue {
public:
	LockFreeQueue();
	~LockFreeQueue();
	
	// Producer thread safe    
	void Push(int* item); // 소유권 전달(포인터 기반)   
	
	// Consumer (single thread)    
	int* Pop(); // nullptr: 비어있음    // 비파괴 검사 (optional)    
	bool Empty() const;
};