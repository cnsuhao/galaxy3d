#include "LauncherDemoTerrain.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
std::string clip_idle = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
std::string clip_move = "Move_Arthas_1586e0d40a0ba4545bd97991164aec42.1.clip";
Vector3 anim_rot(0, 0, 0);

void LauncherDemoTerrain::Start()
{
    Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(g_unit_per_pixel * Screen::GetHeight() / 2);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);
    cam2d->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

    auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
    canvas->GetTransform()->SetParent(cam2d->GetTransform());
    canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * g_unit_per_pixel);
	canvas->SetCamera(cam2d);

    auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
    auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
    tr->SetLabel(label);
    tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
    fps = tr;

    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(30);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(101, 7, 115));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(20, -140, 0));

    std::vector<std::string> terrain_texs;
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t0/1.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t0/2.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t0/3.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t0/4.png");

    GameObject *terrain_obj = GameObject::Create("terrain").get();
    terrain_obj->SetLayer(Layer::Default);

    auto ter = terrain_obj->AddComponent<Terrain>();
    ter->SetCamera(cam3d);
    ter->LoadData(
        513,
        200.0f, 600.0f,
        Application::GetDataPath() + "/Assets/terrain/t0/Terrain.raw",
        Application::GetDataPath() + "/Assets/terrain/t0/Terrain.png",
        terrain_texs, 3);
    auto ter_renderer = terrain_obj->AddComponent<TerrainRenderer>();
    auto tc = terrain_obj->AddComponent<TerrainCollider>();
    tc->SetTerrain(ter);

    auto lightmap_ter = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/t0/Lightmap-1_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    ter->GetSharedMaterial()->SetShader(Shader::Find("Lightmap/Terrain/Diffuse"));
	ter->GetSharedMaterial()->SetTexture("_Lightmap", lightmap_ter);

    auto lightmap = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/t0/Lightmap-0_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.push_back(lightmap);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/t0/Objects/Objects.mesh");
    mesh->SetLayerRecursively(Layer::Default);
    Renderer::BuildOctree(mesh);

    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Arthas/Arthas.anim");
    anim_obj->SetLayerRecursively(Layer::Default);
    anim_obj->GetTransform()->SetPosition(Vector3(91, 0.05f, 103));
    anim_obj->GetTransform()->SetRotation(Quaternion::Euler(anim_rot));
    anim_obj->GetTransform()->SetScale(Vector3(1, 1, 1) * 0.5f);

    cam_offset = cam3d->GetTransform()->GetPosition() - anim_obj->GetTransform()->GetPosition();

    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState(clip_idle)->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState(clip_move)->wrap_mode = WrapMode::Loop;
    anim->Play(clip_idle);

    auto light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(135, 0, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(1, 1, 1, 1) * 0.6f);

    RenderSettings::SetGlobalDirectionalLight(light);
    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;

    //set anim to ground
    Vector3 anim_pos = anim->GetTransform()->GetPosition();
    RaycastHit hit;
    if(Physics::Raycast(Vector3(anim_pos.x, 1000, anim_pos.z), Vector3(0, -1, 0), 2000, hit))
    {
        hit.point.y += 0.05f;
        anim->GetTransform()->SetPosition(hit.point);
    }

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoTerrain::OnTweenPositionSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<LauncherDemoTerrain>(target.lock());
        Vector3 *pos = (Vector3 *) value;
        TweenPosition *tp = (TweenPosition *) tween;

        RaycastHit hit;
        if(Physics::Raycast(Vector3(pos->x, 1000, pos->z), Vector3(0, -1, 0), 2000, hit))
        {
            hit.point.y += 0.05f;

            thiz->anim->GetTransform()->SetPosition(hit.point);
        }
    }
}

void LauncherDemoTerrain::OnTweenPositionFinished(Component *tween, std::weak_ptr<Component> &target)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<LauncherDemoTerrain>(target.lock());
        thiz->anim->CrossFade(clip_idle);
    }
}

void LauncherDemoTerrain::OnTweenRotationSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<LauncherDemoTerrain>(target.lock());
        Vector3 *rot = (Vector3 *) value;
        TweenRotation *tr = (TweenRotation *) tween;
        auto quat = Quaternion::Euler(*rot);

        thiz->anim->GetTransform()->SetRotation(quat);
        anim_rot = *rot;
    }
}

void LauncherDemoTerrain::Update()
{
    fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
        "drawcall:" + GTString::ToString(GTTime::GetDrawCall()));

    if(Input::GetTouchCount() > 0)
    {
        auto t = Input::GetTouch(0);

        if(t->phase == TouchPhase::Began)
        {
            Vector3 pos = t->position;
            Ray ray = cam3d->ScreenPointToRay(pos);

            RaycastHit hit;
            if(Physics::Raycast(ray.origin, ray.GetDirection(), 1000, hit))
            {
                Vector3 pos_old = anim->GetTransform()->GetPosition();
                Vector3 pos_new = hit.point + Vector3(0, 0.05f, 0);
                float move_speed = 3.0f;
                float move_time = (pos_new - pos_old).Magnitude() / move_speed;
                Vector3 dir = pos_new - pos_old;
                dir.y = 0;

                anim->CrossFade(clip_move);

                {
                    // tween position
                    auto tp = anim->GetGameObject()->GetComponent<TweenPosition>();
                    if(tp)
                    {
                        tp->Reset();
                    }
                    else
                    {
                        tp = anim->GetGameObject()->AddComponent<TweenPosition>();
                        tp->is_world = true;
                        tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                        tp->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        tp->target = GetComponentPtr();
                        tp->on_set_value = OnTweenPositionSetValue;
                        tp->on_finished = OnTweenPositionFinished;
                    }

                    tp->from = pos_old;
                    tp->to = pos_new;
                    tp->duration = move_time;
                }

                {
                    float deg = atan2f(dir.x, dir.z) * Mathf::Rad2Deg;
                    if(deg < 0)
                    {
                        deg += 360;
                    }
                    if(deg > 360)
                    {
                        deg -= 360;
                    }
                    if(anim_rot.y < 0)
                    {
                        anim_rot.y += 360;
                    }
                    if(anim_rot.y > 360)
                    {
                        anim_rot.y -= 360;
                    }
                    if(fabs(anim_rot.y - deg) > 180)
                    {
                        if(anim_rot.y > deg)
                        {
                            anim_rot.y -= 360;
                        }
                        else
                        {
                            anim_rot.y += 360;
                        }
                    }
                    float rot_speed = 270.0f;
                    float rot_time = fabs(anim_rot.y - deg) / rot_speed;

                    auto tr = anim->GetGameObject()->GetComponent<TweenRotation>();
                    if(tr)
                    {
                        tr->Reset();
                    }
                    else
                    {
                        tr = anim->GetGameObject()->AddComponent<TweenRotation>();
                        tr->is_world = true;
                        tr->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                        tr->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        tr->target = GetComponentPtr();
                        tr->on_set_value = OnTweenRotationSetValue;
                    }

                    tr->from = anim_rot;
                    tr->to = Vector3(0, deg, 0);
                    tr->duration = rot_time;
                }
            }
        }
        else if(
            t->phase == TouchPhase::Ended ||
            t->phase == TouchPhase::Canceled)
        {
        }
        else if(t->phase == TouchPhase::Moved)
        {
            Vector3 pos = t->position;
        }
    }
}

void LauncherDemoTerrain::LateUpdate()
{
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
}