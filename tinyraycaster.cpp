#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cassert>

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
	return (a << 24) + (b<<16) + (g<<8) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
	r = (color >> 0) & 255;
	g = (color >> 8) & 255;
	b = (color >> 16) & 255;
	a = (color >> 24) & 255;
}

void drop_ppm_image(const std::string filename, const std::vector<uint32_t> &image, const size_t w, const size_t h) {
	assert(image.size() == w*h);
	std::ofstream ofs(filename);
	ofs << "P6\n" << w << " " << h << "\n255\n";
	for(size_t i = 0; i < h*w; ++i) {
		uint8_t r, g, b, a;
		unpack_color(image[i], r, g, b, a);
		ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
	}
	ofs.close();
}

void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
	assert(img.size() == img_w*img_h);
	for(size_t i=0; i<w; i++) {
		for(size_t j=0; j<h; j++) {
			size_t cx = x+i;
			size_t cy = y+j;
			if(cx>=img_w || cy>=img_h) continue;
			img[cx + cy*img_w] = color;
		}
	}
}

int main() {
	const size_t win_w = 1024;
	const size_t win_h = 512;
	std::vector<uint32_t> framebuffer(win_w*win_h, pack_color(255, 255, 255)); // initialize to white

	const size_t map_w = 16;
	const size_t map_h = 16;
    const char map[] = "0000222222220000"\
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
                       "0002222222200000"; // our game map
	assert(sizeof(map) == map_w*map_h+1); // +1 for null-terminator
	float player_x = 3.456;
	float player_y = 2.345;
	float player_a = 1.523;
	const float fov = M_PI/3;

	// generate some random wall colors
	const size_t ncolors = 10;
	std::vector<uint32_t> colors(ncolors);
	for(size_t i=0; i<ncolors; i++) {
		colors[i] = pack_color(rand()%255, rand()%255, rand()%255);
	}

	const size_t rect_w = win_w/(map_w*2); // width of a cell; relation of map to image size
	const size_t rect_h = win_h/map_h; // height of a cell; relation of map to image size

	for(size_t frame=0; frame<360; frame++) {

		std::stringstream ss;
		ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
		player_a += 2*M_PI/360;

		std::vector<uint32_t> framebuffer(win_w*win_h, pack_color(255, 255, 255)); // clear the screen

		// draw the map
		for(size_t j=0; j<map_h; j++) {
			for(size_t i=0; i<map_w; i++) {
				if(map[i+j*map_w]==' ') continue; // skip empty squares
				size_t rect_x = i*rect_w;
				size_t rect_y = j*rect_h;
				size_t icolor = map[i+j*map_w] - '0';
				assert(icolor<ncolors);
				draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, colors[icolor]);
			}
		}

		// draw vision cone and "3D" view
		for(size_t i=0; i<win_w/2; i++) {
			float angle = player_a-fov/2 + fov*i/float(win_w/2);
			for(float t=0; t<20; t+=.01) {
				float cx = player_x + t*cos(angle);
				float cy = player_y + t*sin(angle);

				size_t pix_x = cx*rect_w;
				size_t pix_y = cy*rect_h;
				framebuffer[pix_x + pix_y*win_w] = pack_color(160, 160, 160);

				if(map[int(cx)+int(cy)*map_w] != ' ') { // ray touches wall
					size_t icolor = map[int(cx)+int(cy)*map_w] - '0';
					assert(icolor<ncolors);
					size_t column_height = win_h/t;
					draw_rectangle(framebuffer, win_w, win_h, win_w/2+i, win_h/2-column_height/2, 1, column_height, colors[icolor]);
					break;
				}
			}
		}

		drop_ppm_image(ss.str(), framebuffer, win_w, win_h);

	}

	return 0;
}