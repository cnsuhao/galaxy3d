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
        static void *GetCurent() {return m_cursor_current;}
        static void Done();

    private:
        static std::unordered_map<int, void *> m_cursors;
        static void *m_cursor_current;
    };
}

#endif