#include "Guid.h"

#if defined(WINPC) || defined(WINPHONE)
#include <objbase.h>

static std::string new_guid_winpc()
{
    GUID guid;
    CoCreateGuid(&guid);

    char buffer[64];
    sprintf_s(buffer, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    return buffer;
}
#endif

namespace Galaxy3D
{
    std::string Guid::NewGuid()
    {
#if defined(WINPC) || defined(WINPHONE)
        return new_guid_winpc();
#endif
    }
}