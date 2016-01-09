#include "Cursor.h"
#include <Windows.h>

namespace Galaxy3D
{
    std::unordered_map<int, void *> Cursor::m_cursors;
    void *Cursor::m_cursor_current = NULL;

    void Cursor::Load(const std::string &path, int id)
    {
        auto find = m_cursors.find(id);
        if(find != m_cursors.end())
        {
            DestroyCursor((HCURSOR) find->second);
        }

        m_cursors[id] = LoadImageA(NULL, path.c_str(), IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    }

    void Cursor::SetCursor(int id)
    {
        auto find = m_cursors.find(id);
        if(find != m_cursors.end())
        {
            m_cursor_current = find->second;
        }
    }

    void Cursor::Done()
    {
        for(auto &i : m_cursors)
        {
            DestroyCursor((HCURSOR) i.second);
        }
        m_cursors.clear();
    }
}