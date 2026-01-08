#pragma once
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <new>

// ============================================================
// ObjectPoolTLS.h
// - Template-based fixed object pool with:
//   * Global lock-free stack (InterlockedCompareExchange128, ABA tag)
//   * Per-thread cache (TLS) using a small array
// - Constructor/Destructor are NOT called (by request).
//   -> Use placement new / explicit destructor in user code if needed.
// ============================================================

namespace simplepool
{
    // --------- helpers ----------
    static inline void cpu_relax()
    {
        YieldProcessor();
    }

    template<typename T>
    class ObjectPoolTLS
    {
    private:
        struct Node
        {
            Node* next;
            alignas(T) unsigned char storage[sizeof(T)];
        };

        // Head for 128-bit CAS: {ptr, tag}
        struct alignas(16) Head
        {
            Node* ptr;
            int64_t  tag;
        };

        static_assert(sizeof(Head) == 16, "Head must be 16 bytes for CAS128");

    public:
        // NOTE:
        // totalCount: number of nodes in the pool
        // tlsCapacity: per-thread cache size (default 64)
        // batch: how many nodes to move between TLS and global (default 32)
        ObjectPoolTLS(int totalCount, int tlsCapacity = 64, int batch = 32, bool pageLock = false, int align = 0)
            : m_total(totalCount),
            m_tlsCap((tlsCapacity > 0) ? tlsCapacity : 64),
            m_batch((batch > 0) ? batch : 32),
            m_poolMem(nullptr),
            m_nodeStride(0)
        {
            if (m_total <= 0) crash();

            // Optional stride alignment (similar spirit to your existing code)
            m_nodeStride = (int)sizeof(Node);
            if (align > 0 && (align % 2) == 0) {
                int rem = m_nodeStride % align;
                if (rem != 0) m_nodeStride += (align - rem);
            }

            const SIZE_T bytes = (SIZE_T)m_nodeStride * (SIZE_T)m_total;
            m_poolMem = (unsigned char*)VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (!m_poolMem) crash();

            // page가 내려가지 않도록 하고 싶다면...
            if (pageLock) {
                if (!VirtualLock(m_poolMem, bytes)) {
                    VirtualFree(m_poolMem, 0, MEM_RELEASE);
                    m_poolMem = nullptr;
                    crash();
                }
            }

            m_head.ptr = nullptr;
            m_head.tag = 1;

            // Build initial global free list
            for (int i = 0; i < m_total; ++i) {
                Node* n = nodeAt(i);
                pushGlobal(n);
            }
        }

        ~ObjectPoolTLS()
        {
            if (m_poolMem) {
                VirtualFree(m_poolMem, 0, MEM_RELEASE);
                m_poolMem = nullptr;
            }
        }

        // Acquire raw storage for T (constructor NOT called)
        // Returns: pointer to storage as T*
        T* AcquireRaw()
        {
            Cache& c = tlsCache();

            // Fast path: TLS pop
            if (c.count > 0) {
                Node* n = c.nodes[--c.count];
                return (T*)n->storage;
            }

            // Refill from global in batch
            refillFromGlobal(c);

            if (c.count == 0)
                return nullptr;

            Node* n = c.nodes[--c.count];
            return (T*)n->storage;
        }

        // Release raw storage back to pool (destructor NOT called)
        void ReleaseRaw(T* p)
        {
            if (!p) return;

            Node* n = storageToNode(p);
            Cache& c = tlsCache();

            // TLS push
            if (c.count < m_tlsCap) {
                c.nodes[c.count++] = n;
                return;
            }

            // TLS full -> flush batch to global then push
            flushToGlobal(c);
            c.nodes[c.count++] = n;
        }

        // OPTIONAL convenience wrappers (disabled by request)
        // ---------------------------------------------------
        // template<class... Args>
        // T* New(Args&&... args)
        // {
        //     T* mem = AcquireRaw();
        //     if (!mem) return nullptr;
        //     return new (mem) T((Args&&)args...);
        // }
        //
        // void Delete(T* obj)
        // {
        //     if (!obj) return;
        //     obj->~T();
        //     ReleaseRaw(obj);
        // }

    private:
        struct Cache
        {
            Node** nodes;
            int    count;
            Cache() : nodes(nullptr), count(0) {}
        };

        // thread_local cache per T per thread
        static Cache& tlsCache()
        {
            thread_local Cache c;
            thread_local bool  inited = false;

            if (!inited) {
                // allocate TLS array (per-thread) once
                // Use HeapAlloc to avoid STL dependency.
                // NOTE: size depends on pool instance (m_tlsCap), but tlsCache is static.
                // So we store capacity inside Cache via nodes allocation at first use in refillFromGlobal
                inited = true;
            }
            return c;
        }

        void ensureTlsArray(Cache& c)
        {
            if (c.nodes) return;
            // allocate pointer array for TLS cache
            SIZE_T bytes = (SIZE_T)m_tlsCap * sizeof(Node*);
            c.nodes = (Node**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes);
            if (!c.nodes) crash();
            c.count = 0;
        }

        void refillFromGlobal(Cache& c)
        {
            ensureTlsArray(c);

            const int want = (m_batch < m_tlsCap) ? m_batch : m_tlsCap;

            int got = 0;
            while (got < want) {
                Node* n = popGlobal();
                if (!n) break;
                c.nodes[c.count++] = n;
                ++got;
            }
        }

        void flushToGlobal(Cache& c)
        {
            ensureTlsArray(c);

            // Flush at most batch nodes to keep locality
            int flushN = (c.count < m_batch) ? c.count : m_batch;

            for (int i = 0; i < flushN; ++i) {
                Node* n = c.nodes[--c.count];
                pushGlobal(n);
            }
        }

        Node* nodeAt(int i)
        {
            return (Node*)(m_poolMem + (SIZE_T)m_nodeStride * (SIZE_T)i);
        }

        static Node* storageToNode(T* p)
        {
            // storage is inside Node; compute Node* by subtracting offset of storage
            unsigned char* u = (unsigned char*)p;
            Node* n = (Node*)(u - offsetof(Node, storage));
            return n;
        }

        // -------- global stack ops (ABA-safe, CAS128) ----------
        void pushGlobal(Node* n)
        {
            Head oldv;
            Head newv;

            while (true) {
                oldv.ptr = m_head.ptr;
                oldv.tag = m_head.tag;

                n->next = oldv.ptr;
                newv.ptr = n;
                newv.tag = oldv.tag + 1;

                if (casHead(oldv, newv))
                    return;

                cpu_relax();
            }
        }

        Node* popGlobal()
        {
            Head oldv;
            Head newv;

            while (true) {
                oldv.ptr = m_head.ptr;
                oldv.tag = m_head.tag;

                Node* top = oldv.ptr;
                if (!top)
                    return nullptr;

                newv.ptr = top->next;
                newv.tag = oldv.tag + 1;

                if (casHead(oldv, newv)) {
                    top->next = nullptr;
                    return top;
                }

                cpu_relax();
            }
        }

        bool casHead(const Head& expected, const Head& desired)
        {
#if defined(_M_X64) || defined(__x86_64__)
            // InterlockedCompareExchange128 expects: destination points to 128-bit value
            // and comparand in a 128-bit buffer.
            Head cmp = expected;
            return (1 == InterlockedCompareExchange128(
                (volatile long long*)&m_head,
                desired.tag,
                (long long)desired.ptr,
                (long long*)&cmp));
#else
            // x86 build: CAS128 not available. If you need x86, switch to SRWLOCK/mutex here.
            (void)expected; (void)desired;
            crash();
            return false;
#endif
        }

        __declspec(noreturn) static void crash()
        {
            // Intentional crash like your existing code style.
            volatile int* p = nullptr;
            *p = 0;
            for (;;) {}
        }

    private:
        int            m_total;
        int            m_tlsCap;
        int            m_batch;

        unsigned char* m_poolMem;
        int            m_nodeStride;

        volatile Head  m_head;
    };

} // namespace simplepool
