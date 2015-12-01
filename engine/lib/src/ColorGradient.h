#ifndef __ColorGradient_h__
#define __ColorGradient_h__

#include "Color.h"
#include <vector>

namespace Galaxy3D
{
    struct ColorGradient
    {
        struct KeyRGB
        {
            float location;
            Color rgb;

            KeyRGB(float location, const Color &rgb):
                location(location),
                rgb(rgb)
            {}
        };

        struct KeyAlpha
        {
            float location;
            float a;

            KeyAlpha(float location, float a):
                location(location),
                a(a)
            {}
        };

        std::vector<KeyRGB> key_rgbs;
        std::vector<KeyAlpha> key_alphas;

        bool HasKey() const
        {
            return !key_rgbs.empty() && !key_alphas.empty();
        }

        Color GetColor(float location) const
        {
            int left = -1;
            int right = -1;
            for(size_t i=0; i<key_alphas.size(); i++)
            {
                if(key_alphas[i].location <= location)
                {
                    left = i;
                }
                else
                {
                    break;
                }
            }
            for(int i=key_alphas.size()-1; i>=0; i--)
            {
                if(key_alphas[i].location > location)
                {
                    right = i;
                }
                else
                {
                    break;
                }
            }
            if(left < 0)
            {
                left = 0;
            }
            if(right < 0)
            {
                right = key_alphas.size() - 1;
            }

            float a = Mathf::Lerp(key_alphas[left].a, key_alphas[right].a, (location - key_alphas[left].location) / (key_alphas[right].location - key_alphas[left].location));

            left = -1;
            right = -1;
            for(size_t i=0; i<key_rgbs.size(); i++)
            {
                if(key_rgbs[i].location <= location)
                {
                    left = i;
                }
                else
                {
                    break;
                }
            }
            for(int i=key_rgbs.size()-1; i>=0; i--)
            {
                if(key_rgbs[i].location > location)
                {
                    right = i;
                }
                else
                {
                    break;
                }
            }
            if(left < 0)
            {
                left = 0;
            }
            if(right < 0)
            {
                right = key_rgbs.size() - 1;
            }

            Color c = Color::Lerp(key_rgbs[left].rgb, key_rgbs[right].rgb, (location - key_rgbs[left].location) / (key_rgbs[right].location - key_rgbs[left].location));
            c.a = a;

            return c;
        }
    };
}

#endif