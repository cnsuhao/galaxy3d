#include "Cursor.h"
#include <Windows.h>

extern HWND g_hwnd;

namespace Galaxy3D
{
    std::unordered_map<int, void *> Cursor::m_cursors;
    int Cursor::m_current = -1;

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
            m_current = id;

            SendMessage(g_hwnd, WM_SETCURSOR, 0, 0);
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

    void *Cursor::GetCursor(int id)
    {
        auto find = m_cursors.find(id);
        if(find != m_cursors.end())
        {
            return find->second;
        }

        return NULL;
    }
}