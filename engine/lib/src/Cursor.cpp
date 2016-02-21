#include "Cursor.h"

#ifdef WINPC
#include <Windows.h>

extern HWND g_hwnd;
#endif

namespace Galaxy3D
{
    std::unordered_map<int, void *> Cursor::m_cursors;
    int Cursor::m_current = -1;

    void Cursor::Load(const std::string &path, int id)
    {
#ifdef WINPC
        auto find = m_cursors.find(id);
        if(find != m_cursors.end())
        {
            DestroyCursor((HCURSOR) find->second);
        }

        m_cursors[id] = LoadImageA(NULL, path.c_str(), IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
#endif
    }

    void Cursor::SetCursor(int id)
    {
#ifdef WINPC
        auto find = m_cursors.find(id);
        if(find != m_cursors.end())
        {
            m_current = id;

            SendMessage(g_hwnd, WM_SETCURSOR, 0, 0);
        }
#endif
    }

    void Cursor::Done()
    {
#ifdef WINPC
        for(auto &i : m_cursors)
        {
            DestroyCursor((HCURSOR) i.second);
        }
        m_cursors.clear();
#endif
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