#pragma once

void* operator new (size_t size);
void* operator new[](size_t size);

void* operator new (size_t size, const char* File, int Line);
void* operator new[](size_t size, const char* File, int Line);

// 할당하지 않으면 문제가 생김...
// 내부적으로는 채워 넣지 말것!!!
void operator delete (void* p, const char* File, int Line);
void operator delete[](void* p, const char* File, int Line);
// 실제로 사용할 delete
void operator delete (void* p);
void operator delete[](void* p);

#define new		new(__FILE__, __LINE__)