#ifndef __Cursor_h__
#define __Cursor_h__

#include <string>
#include <unordered_map>

namespace Galaxy3D
{
    class Cursor
    {
    public:
        static void Load(const std::string &path, int id);
        static void SetCursor(int id);
        static int GetCurent() {return m_current;}
        static void *GetCursor(int id);
        static void Done();

    private:
        static std::unordered_map<int, void *> m_cursors;
        static int m_current;
    };
}

#endif