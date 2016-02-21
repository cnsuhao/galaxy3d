#include "pch.h"
#include "LauncherMerged.h"
#include "GameObject.h"
#include "Camera.h"

namespace Galaxy3D
{
    void LauncherMerged::Start()
    {
        auto cam = GameObject::Create("")->AddComponent<Camera>();
    }
}