#include "Image565.h"


Image565::Image565(const uint16_t width, const uint16_t height, const char* image_data):
img_width(width),
img_height(height),
img_data((uint16_t*)image_data)
{

}

Image565::~Image565()
{

}

void Image565::draw(Adafruit_GFX& gfx, const uint16_t x_offset, const uint16_t y_offset)
{
    for(uint16_t y = 0; y < img_height; ++y)
    {
        for(uint16_t x = 0; x < img_width; ++x)
        {
            gfx.drawPixel(x + x_offset, y + y_offset, img_data[img_width*y + x]);
        }
    }
}