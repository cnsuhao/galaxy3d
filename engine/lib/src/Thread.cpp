#include "Thread.h"

#ifdef WINPC
#include <Windows.h>

static DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter)
{
    Galaxy3D::ThreadParam *p = (Galaxy3D::ThreadParam *) lpParameter;
    Galaxy3D::ThreadFunc func = p->func;
    void *param = p->param;
    delete p;

    func(param);

    return 0;
}
#endif

namespace Galaxy3D
{
    void Thread::Create(ThreadFunc func, void *param)
    {
        ThreadParam *p = new ThreadParam();
        p->func = func;
        p->param = param;

        auto thread = CreateThread(NULL, 0, ThreadProc, p, 0, NULL);
        SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
        CloseHandle(thread);
    }
}