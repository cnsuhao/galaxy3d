#include "WinGraphicSettings.h"
#include "Transform.h"
#include "GameObject.h"
#include "RenderSettings.h"
#include "ImageEffectGlobalFog.h"
#include "TextRenderer.h"
#include "UISlider.h"
#include "UIToggle.h"
#include "UIScrollView.h"
#include "UIScrollBar.h"
#include "UISelectView.h"
#include "UICanvas.h"

using namespace Galaxy3D;

struct GraphicHeaderEventListener : public UIEventListener
{
    std::weak_ptr<GameObject> win;
    Vector3 down_pos;
    Vector3 down_pos_win;

    virtual void OnDragStart()
    {
        down_pos = UICanvas::GetLastPosition();

        if(!win.expired())
        {
            down_pos_win = win.lock()->GetTransform()->GetLocalPosition();
        }
    }

    virtual void OnDrag(const Vector3 &delta)
    {
        if(!win.expired())
        {
            Vector3 pos = down_pos_win + UICanvas::GetLastPosition() - down_pos;
            pos.x = Mathf::Round(pos.x);
            pos.y = Mathf::Round(pos.y);

            win.lock()->GetTransform()->SetLocalPosition(pos);
        }
    }
};

struct GraphicCloseEventListener : public UIEventListener
{
    std::weak_ptr<GameObject> win;

    virtual void OnClick()
    {
        win.lock()->SetActive(false);
    }
};

struct TabLightingEventListener : public UIEventListener
{
    virtual void OnClick()
    {
        GetTransform()->Find("hilight")->GetGameObject()->SetActive(true);
        GetTransform()->GetParent().lock()->Find("fog/hilight")->GetGameObject()->SetActive(false);
    }
};

struct TabFogEventListener : public UIEventListener
{
    virtual void OnClick()
    {
        GetTransform()->Find("hilight")->GetGameObject()->SetActive(true);
        GetTransform()->GetParent().lock()->Find("lighting/hilight")->GetGameObject()->SetActive(false);
    }
};

struct AmbientRSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        Color color = RenderSettings::light_ambient;
        color.r = GetAmount();
        RenderSettings::light_ambient = color;
    }
};

struct AmbientGSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        Color color = RenderSettings::light_ambient;
        color.g = GetAmount();
        RenderSettings::light_ambient = color;
    }
};

struct AmbientBSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        Color color = RenderSettings::light_ambient;
        color.b = GetAmount();
        RenderSettings::light_ambient = color;
    }
};

struct DirectionalRSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        Color color = dir->GetColor();
        color.r = GetAmount();
        dir->SetColor(color);
    }
};

struct DirectionalGSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        Color color = dir->GetColor();
        color.g = GetAmount();
        dir->SetColor(color);
    }
};

struct DirectionalBSliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        Color color = dir->GetColor();
        color.b = GetAmount();
        dir->SetColor(color);
    }
};

struct DirectionalISliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        dir->SetIntensity(GetValue<float>());
    }
};

struct CascadeSplit0SliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto old = Light::GetCascadeSplits();
        std::vector<float> splits;
        splits.push_back(GetValue<float>());
        splits.push_back(old[1]);
        splits.push_back(old[2]);
        Light::SetCascadeSplits(splits);
    }
};

struct CascadeSplit1SliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto old = Light::GetCascadeSplits();
        std::vector<float> splits;
        splits.push_back(old[0]);
        splits.push_back(GetValue<float>());
        splits.push_back(old[2]);
        Light::SetCascadeSplits(splits);
    }
};

struct CascadeSplit2SliderEventListener : public UISlider
{
    virtual void OnValueChanged()
    {
        auto old = Light::GetCascadeSplits();
        std::vector<float> splits;
        splits.push_back(old[0]);
        splits.push_back(old[1]);
        splits.push_back(GetValue<float>());
        Light::SetCascadeSplits(splits);
    }
};

struct ShadowToggleEventListener : public UIToggle
{
    virtual void OnValueChanged(bool value)
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        dir->EnableShadow(value);
    }
};

struct CascadeToggleEventListener : public UIToggle
{
    virtual void OnValueChanged(bool value)
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        dir->EnableCascade(value);
    }
};

struct FogEnableToggleEventListener : public UIToggle
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged(bool value)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->Enable(value);
            }
        }
    }
};

struct FogExcludeFarPixelsToggleEventListener : public UIToggle
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged(bool value)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->ExcludeFarPixels(value);
            }
        }
    }
};

struct FogRSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                Color c = fog->GetFogColor();
                c.r = GetAmount();
                fog->SetFogColor(c);
            }
        }
    }
};

struct FogGSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                Color c = fog->GetFogColor();
                c.g = GetAmount();
                fog->SetFogColor(c);
            }
        }
    }
};

struct FogBSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                Color c = fog->GetFogColor();
                c.b = GetAmount();
                fog->SetFogColor(c);
            }
        }
    }
};

struct FogDistanceEnableToggleEventListener : public UIToggle
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged(bool value)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->EnableDistance(value);
            }
        }
    }
};

struct FogDistanceRadialToggleEventListener : public UIToggle
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged(bool value)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->UseRadialDistance(value);
            }
        }
    }
};

struct FogDistanceStartSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetStartDistance(GetValue<float>());
            }
        }
    }
};

struct FogHeightEnableToggleEventListener : public UIToggle
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged(bool value)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->EnableHeight(value);
            }
        }
    }
};

struct FogHeightSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetHeight(GetValue<float>());
            }
        }
    }
};

struct FogHeightDensitySliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetHeightDensity(GetValue<float>());
            }
        }
    }
};

struct FogDensitySliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetFogDensity(GetValue<float>());
            }
        }
    }
};

struct FogModeSelectEventListener : public UISelectView
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnSelected(int index)
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetFogMode((FogMode::Enum) (index + 1));
            }
        }
    }
};

struct FogLinearStartSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetLinearStart(GetValue<float>());
            }
        }
    }
};

struct FogLinearEndSliderEventListener : public UISlider
{
    std::weak_ptr<Camera> cam3d;

    virtual void OnValueChanged()
    {
        auto cam = cam3d.lock();
        if(cam)
        {
            auto fog = cam->GetGameObject()->GetComponent<ImageEffectGlobalFog>();
            if(fog)
            {
                fog->SetLinearEnd(GetValue<float>());
            }
        }
    }
};

void WinGraphicSettings::Init()
{
    auto header = GetTransform()->Find("Background/Header")->GetGameObject()->AddComponent<GraphicHeaderEventListener>();
    header->win = GetGameObject();

    auto close = GetTransform()->Find("Background/Header/close")->GetGameObject()->AddComponent<GraphicCloseEventListener>();
    close->win = GetGameObject();

    GetTransform()->Find("Background/left tabs/lighting")->GetGameObject()->AddComponent<TabLightingEventListener>();
    GetTransform()->Find("Background/left tabs/fog")->GetGameObject()->AddComponent<TabFogEventListener>();
    
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/ar/Slider")->GetGameObject()->AddComponent<AmbientRSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.263f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/ag/Slider")->GetGameObject()->AddComponent<AmbientGSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.502f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/ab/Slider")->GetGameObject()->AddComponent<AmbientBSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.784f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/dr/Slider")->GetGameObject()->AddComponent<DirectionalRSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.988f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/dg/Slider")->GetGameObject()->AddComponent<DirectionalGSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.71f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/db/Slider")->GetGameObject()->AddComponent<DirectionalBSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->SetAmount(0.349f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/di/Slider")->GetGameObject()->AddComponent<DirectionalISliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 8;
        slider->SetAmount(3.4f / 8);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/shadow/Toggle")->GetGameObject()->AddComponent<ShadowToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->SetValue(true);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/cascade/Toggle")->GetGameObject()->AddComponent<CascadeToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->SetValue(true);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/cascade split 0/Slider")->GetGameObject()->AddComponent<CascadeSplit0SliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 1;
        slider->SetAmount(0.12f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/cascade split 1/Slider")->GetGameObject()->AddComponent<CascadeSplit1SliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 1;
        slider->SetAmount(0.24f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/cascade split 2/Slider")->GetGameObject()->AddComponent<CascadeSplit2SliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 1;
        slider->SetAmount(0.5f);
    }
    {
        auto scroll_view = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view")->GetGameObject()->AddComponent<UIScrollView>();
        scroll_view->scroll_target = scroll_view->GetTransform()->Find("scroll target")->GetGameObject();
        auto scroll_bar = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll bar")->GetGameObject()->AddComponent<UIScrollBar>();
        scroll_bar->scroll_view = scroll_view;
        scroll_bar->background = scroll_bar->GetGameObject()->GetComponent<SpriteNode>();
        auto thumb = scroll_bar->GetTransform()->Find("Foreground")->GetGameObject()->AddComponent<UIScrollBarThumb>();
        thumb->scroll_bar = scroll_bar;
        scroll_bar->thumb = thumb;
        scroll_view->scroll_bar = scroll_bar;
        scroll_view->Init();
    }
    {
        auto scroll_view = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view")->GetGameObject()->AddComponent<UIScrollView>();
        scroll_view->scroll_target = scroll_view->GetTransform()->Find("scroll target")->GetGameObject();
        auto scroll_bar = GetTransform()->Find("Background/left tabs/fog/hilight/scroll bar")->GetGameObject()->AddComponent<UIScrollBar>();
        scroll_bar->scroll_view = scroll_view;
        scroll_bar->background = scroll_bar->GetGameObject()->GetComponent<SpriteNode>();
        auto thumb = scroll_bar->GetTransform()->Find("Foreground")->GetGameObject()->AddComponent<UIScrollBarThumb>();
        thumb->scroll_bar = scroll_bar;
        scroll_bar->thumb = thumb;
        scroll_view->scroll_bar = scroll_bar;
        scroll_view->Init();
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/enable/Toggle")->GetGameObject()->AddComponent<FogEnableToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->cam3d = cam3d;
        toggle->SetValue(false);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/far/Toggle")->GetGameObject()->AddComponent<FogExcludeFarPixelsToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->cam3d = cam3d;
        toggle->SetValue(false);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/r/Slider")->GetGameObject()->AddComponent<FogRSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->cam3d = cam3d;
        slider->SetAmount(0.5f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/g/Slider")->GetGameObject()->AddComponent<FogGSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->cam3d = cam3d;
        slider->SetAmount(0.5f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/b/Slider")->GetGameObject()->AddComponent<FogBSliderEventListener>();
        slider->type = UISliderValueType::Int;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 255;
        slider->cam3d = cam3d;
        slider->SetAmount(0.5f);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/distance enable/Toggle")->GetGameObject()->AddComponent<FogDistanceEnableToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->cam3d = cam3d;
        toggle->SetValue(true);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/distance radial/Toggle")->GetGameObject()->AddComponent<FogDistanceRadialToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->cam3d = cam3d;
        toggle->SetValue(false);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/distance start/Slider")->GetGameObject()->AddComponent<FogDistanceStartSliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 200;
        slider->cam3d = cam3d;
        slider->SetAmount(0);
    }
    {
        auto toggle = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/height enable/Toggle")->GetGameObject()->AddComponent<FogHeightEnableToggleEventListener>();
        toggle->checkmark = toggle->GetTransform()->Find("Checkmark")->GetGameObject();
        toggle->cam3d = cam3d;
        toggle->SetValue(true);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/height/Slider")->GetGameObject()->AddComponent<FogHeightSliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 100;
        slider->cam3d = cam3d;
        slider->SetAmount(0.1f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/height density/Slider")->GetGameObject()->AddComponent<FogHeightDensitySliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 10;
        slider->cam3d = cam3d;
        slider->SetAmount(0.2f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/fog density/Slider")->GetGameObject()->AddComponent<FogDensitySliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 0.05f;
        slider->cam3d = cam3d;
        slider->SetAmount(0.4f);
    }
    {
        auto select_view = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/fog mode/Select Field")->GetGameObject()->AddComponent<FogModeSelectEventListener>();
        select_view->selected_item = select_view->GetTransform()->Find("Label")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        select_view->selected_index = 0;
        select_view->arrow = select_view->GetTransform()->Find("Arrow")->GetGameObject()->GetComponent<SpriteNode>();
        select_view->arrow_down = "SelectField_Arrow_Down";
        select_view->arrow_up = "SelectField_Arrow_Up";
        select_view->list = select_view->GetTransform()->Find("List")->GetGameObject();

        UISelectItem item;
        item.label = select_view->GetTransform()->Find("List/Label 0")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        item.high_light = select_view->GetTransform()->Find("List/Label 0/Hilight")->GetGameObject()->GetComponent<SpriteNode>();
        select_view->items.push_back(item);

        item.label = select_view->GetTransform()->Find("List/Label 1")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        item.high_light = select_view->GetTransform()->Find("List/Label 1/Hilight")->GetGameObject()->GetComponent<SpriteNode>();
        select_view->items.push_back(item);

        item.label = select_view->GetTransform()->Find("List/Label 2")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        item.high_light = select_view->GetTransform()->Find("List/Label 2/Hilight")->GetGameObject()->GetComponent<SpriteNode>();
        select_view->items.push_back(item);

        select_view->cam3d = cam3d;

        select_view->Select(2);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/linear start/Slider")->GetGameObject()->AddComponent<FogLinearStartSliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 500;
        slider->cam3d = cam3d;
        slider->SetAmount(0);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/fog/hilight/scroll view/scroll target/linear end/Slider")->GetGameObject()->AddComponent<FogLinearEndSliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 500;
        slider->cam3d = cam3d;
        slider->SetAmount(0.6f);
    }
}