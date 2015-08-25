#ifndef __WrapMode_h__
#define __WrapMode_h__

namespace Galaxy3D
{
    struct WrapMode
    {
        enum Enum
        {
            Default = 0,
            Once = 1,
            Clamp = 1,
            Loop = 2,
            PingPong = 4,
            ClampForever = 8,
        };
    };
}

#endif