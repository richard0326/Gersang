#include "stdafx.h"
#include <cstdio>
#include <new>
#include "memorypool.h"

struct TestObj
{
    int a;
    int b;
};

int main()
{
    using namespace simplepool;

    ObjectPoolTLS<TestObj> pool(/*totalCount*/ 100000, /*tlsCapacity*/ 64, /*batch*/ 32, /*pageLock*/ false, /*align*/ 16);

    // 생성자/소멸자 호출은 요청대로 사용자 책임(주석 참고)
    // TestObj* p = pool.AcquireRaw();
    // new (p) TestObj{1,2};
    // p->~TestObj();
    // pool.ReleaseRaw(p);

    const int N = 500000;
    int ok = 0;

    for (int i = 0; i < N; ++i) {
        TestObj* p = pool.AcquireRaw();
        if (!p) break;

        // (주의) 생성자 호출 없이 쓰면 UB 가능.
        // 여기서는 단순히 메모리 접근만 테스트한다고 가정하고 값만 써봄.
        p->a = i;
        p->b = i + 1;

        pool.ReleaseRaw(p);
        ++ok;
    }

    std::printf("done ok=%d\n", ok);
    return 0;
}
