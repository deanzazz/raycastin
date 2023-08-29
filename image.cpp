#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

/*
color packing dissection because im not that smart

example color: r = 100, g = 150, b = 200, a = 50
packing ->
            a << 24
            b << 16  
             g << 8   
         +  r = 100
        ------------
 packed = 3355627260
   
*/



uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
    return (a<<24) + (b<<16) + (g<<8) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

void drop_ppm_image(const std::string filename, const std::vector<uint32_t> &image, const size_t w, const size_t h) {
    assert(image.size() == w*h);
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << w << " " << h << "\n255\n";
    for (size_t i = 0; i < h*w; i++) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
    ofs.close();
}

void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size()==img_w*img_h);
    for (size_t i=0; i<w; i++) {
        for (size_t j=0; j<h; j++) {
            size_t cx = x+i;
            size_t cy = y+j;
            if (cx>=img_w || cy>=img_h) continue;
            img[cx + cy*img_w] = color;
        }
    }
}

int main() {
    const size_t win_w = 1024;
    const size_t win_h = 512;
    std::vector<uint32_t> framebuffer(win_w*win_h, pack_color(255, 255, 255));
    
    const size_t map_w = 16; // map width
    const size_t map_h = 16; // map height

    //map layout is temporary, want to strengthen my undersatnding of the original code first
    const char map[] = "0000000000000000"\
                       "1              0"\
                       "1      11111   0"\
                       "1     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "2       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0002222222200000"; 
    assert(sizeof(map) == map_w*map_h+1); 


    //PLAYER CODE
    float xplayer = 4.0;
    float yplayer = 4.0;
    float aplayer = 1.3;
    const float fov = M_PI/3.;

    const size_t rect_w = win_w/(map_w*2);
    const size_t rect_h = win_h/map_h;
    for (size_t j=0; j<map_h; j++) {
        for (size_t i=0; i<map_w; i++) {
            if (map[i+j*map_w]==' ') continue; 
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 0, 0));
        }
    }

    for(size_t i=0; i<win_w/2; i++) {
        float angle = aplayer-fov/2 + fov*i/float(win_w/2);
        for (float t=0; t<20; t+=.05) {
            float cx = xplayer + t*cos(angle);
            float cy = yplayer + t*sin(angle);

            size_t pix_x = cx*rect_w;
            size_t pix_y = cy*rect_h;
            framebuffer[pix_x + pix_y*win_w] = pack_color(160, 160, 160);
            if (map[int(cx)+int(cy)*map_w]!=' ') {
                size_t wall_h = win_h/t;
                draw_rectangle(framebuffer, win_w, win_h, win_w/2+i, win_h/2-wall_h/2, 1, wall_h, pack_color(0, 255, 255));
                break;
            }
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}