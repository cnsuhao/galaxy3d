#include "WinGraphicSettings.h"
#include "Transform.h"
#include "GameObject.h"
#include "RenderSettings.h"
#include "TextRenderer.h"
#include "UISlider.h"
#include "UIToggle.h"
#include "UIScrollView.h"
#include "UIScrollBar.h"

using namespace Galaxy3D;

struct GraphicCloseEventListener : public UIEventListener
{
    std::weak_ptr<GameObject> win;

    virtual void OnClick()
    {
        win.lock()->SetActive(false);
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
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        dir->EnableShadow(GetValue());
    }
};

struct CascadeToggleEventListener : public UIToggle
{
    virtual void OnValueChanged()
    {
        auto dir = RenderSettings::GetGlobalDirectionalLight();
        dir->EnableCascade(GetValue());
    }
};

void WinGraphicSettings::Init()
{
    auto close = GetTransform()->Find("Background/Header/close")->GetGameObject()->AddComponent<GraphicCloseEventListener>();
    close->win = GetGameObject();
    
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
        slider->SetAmount(0.08f);
    }
    {
        auto slider = GetTransform()->Find("Background/left tabs/lighting/hilight/scroll view/scroll target/cascade split 1/Slider")->GetGameObject()->AddComponent<CascadeSplit1SliderEventListener>();
        slider->type = UISliderValueType::Float;
        slider->label = slider->GetTransform()->Find("Percent")->GetGameObject()->GetComponent<TextRenderer>()->GetLabel();
        slider->thumb = slider->GetTransform()->Find("Thumb")->GetGameObject()->AddComponent<UISliderThumb>();
        slider->thumb->slider = slider;
        slider->value_min = 0;
        slider->value_max = 1;
        slider->SetAmount(0.2f);
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
}