#include "renderbase.hpp"
#include <iostream>

RenderBase::RenderBase()
{
    screen_pixels = NULL;
}

#define RSHIFT 16
#define GSHIFT 8
#define BSHIFT 0
#define ASHIFT 24

// See: SDL_PixelFormat
#define CURRENT_RGB() (r << RSHIFT) | (g << GSHIFT) | (b << BSHIFT);

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
