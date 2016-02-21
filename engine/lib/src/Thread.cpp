#include "Thread.h"

#if defined(WINPC) || defined(WINPHONE)
#include <Windows.h>
#endif

namespace Galaxy3D
{
    struct ThreadParam
    {
        ThreadFunc func;
        void *param;
    };

#if defined(WINPC) || defined(WINPHONE)
    static DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
    {
        ThreadParam *p = (ThreadParam *) lpParameter;
        ThreadFunc func = p->func;
        void *param = p->param;
        delete p;

        func(param);

        return 0;
    }
#endif

    void Thread::Create(ThreadFunc func, void *param)
    {
        ThreadParam *p = new ThreadParam();
        p->func = func;
        p->param = param;

        auto thread = CreateThread(NULL, 0, ThreadProc, p, 0, NULL);
        CloseHandle(thread);
    }
}