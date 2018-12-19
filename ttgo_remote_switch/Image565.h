#ifndef IMAGE565
#define IMAGE565
#include <inttypes.h>
#include <Adafruit_GFX.h>

class Image565
{
public:
    Image565(const uint16_t width, const uint16_t height, const char* image_data);
    ~Image565();

    void draw(Adafruit_GFX& gfx, const uint16_t x_offset, const uint16_t y_offset);
private:
    const uint16_t img_width;
    const uint16_t img_height;
    const uint16_t* img_data;

};
#endif
