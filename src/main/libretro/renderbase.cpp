#include "renderbase.hpp"
#include <iostream>

RenderBase::RenderBase()
{
    screen_pixels = NULL;

    orig_width  = 0;
    orig_height = 0;
}

// Setup screen size
bool RenderBase::sdl_screen_size()
{
#if 0
    if (orig_width == 0 || orig_height == 0)
    {
        orig_width  = info->current_w; 
        orig_height = info->current_h;
    }
#endif

    /* TODO/FIXME */
#if 0
    scn_width  = orig_width;
    scn_height = orig_height;
#else
    scn_width  = 320;
    scn_height = 240;
#endif

    return true;
}

// See: SDL_PixelFormat
#define CURRENT_RGB() (r << Rshift) | (g << Gshift) | (b << Bshift);

void RenderBase::convert_palette(uint32_t adr, uint32_t r, uint32_t g, uint32_t b)
{
    adr >>= 1;

    r = r * 255 / 31;
    g = g * 255 / 31;
    b = b * 255 / 31;

    rgb[adr] = CURRENT_RGB();
      
    // Create shadow / highlight colours at end of RGB array
    // The resultant values are the same as MAME
    r = r * 202 / 256;
    g = g * 202 / 256;
    b = b * 202 / 256;
        
    rgb[adr + S16_PALETTE_ENTRIES] =
    rgb[adr + (S16_PALETTE_ENTRIES * 2)] = CURRENT_RGB();
}
