#ifndef __Thread_h__
#define __Thread_h__

namespace Galaxy3D
{
    typedef void (*ThreadFunc)(void *param);

    struct ThreadParam
    {
        ThreadFunc func;
        void *param;
    };

    class Thread
    {
    public:
        static void Create(ThreadFunc func, void *param);
    };
}

#endif