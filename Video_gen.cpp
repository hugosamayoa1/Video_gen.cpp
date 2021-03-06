#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <stdio.h>
#include <cassert>
#include <cstring>

#define W 720
#define H 480


unsigned char frame[H][W][3];
const double frames_per_second = 30; 
const int duration_in_seconds = 3;

typedef unsigned char byte;

class Rectangle{
	public:
		Rectangle(){}
		void clamp(int * x, int * y);
		void blank_frame() { memset(frame, 0, sizeof(frame)); }
		void draw_rect(int x, int y, int w, int h, byte r, byte g, byte b);
		void draw_frame(double t);
		bool outside_frame(int * x, int * y);	
};

using namespace std;

// Main drawing code.
// Expand this function to add content to the video.
void Rectangle::draw_frame(double t) {
	blank_frame();
	const double pps = 80; // pixels per second
	draw_rect(0 + t * pps, 0 + t * pps, 20, 10, 0x00, 0xff, 0x00);
	draw_rect(150 + t * pps, 150 + t * pps, 50, 100, 0x00, 0x00, 0xff);
	draw_rect(-50 + t * pps, -50 + t * pps, 10, 5, 0xff, 0x00, 0x00);
	draw_rect(100 + t * pps, 100 + t * pps, 2, 4, 0xff, 0xff, 0xff);
	draw_rect(-100 + t * pps, -100 + t * pps, 15, 50, 0xff, 0xff, 0x00);
}

// Constrain point to frame.
void Rectangle::clamp(int * x, int * y) {
	if (*x < 0) *x = 0; else if (*x >= W) *x = W - 1;
	if (*y < 0) *y = 0; else if (*y >= H) *y = H - 1;
}

bool Rectangle::outside_frame(int * x, int * y) {
	return *x < 0 or *x >= W or *y < 0 or *y >= H;
}

// Draw a solid rectangle at given location, with given width and height
// and with given RGB color value.
void Rectangle::draw_rect(int x, int y, int w, int h, byte r, byte g, byte b) {
	if (outside_frame(&x, &y)) {
		return;
 	}
	int x0 = x;
	int x1 = x + w;
	int y0 = y;
	int y1 = y + h;
	clamp(&x0, &y0);
	clamp(&x1, &y1);
	for (int y = y0; y < y1; ++y) {
		for (int x = x0; x < x1; ++x) {
			if (!outside_frame(&x, &y)){
				frame[y][x][0] = r;
				frame[y][x][1] = g;
				frame[y][x][2] = b;
			}
		}
	}
}


int main(int argc, char * argv[]) {
	// Construct the ffmpeg command to run.
	Rectangle r;
	const char * cmd = 
		"ffmpeg              "
		"-y                  "
		"-hide_banner        "
		"-f rawvideo         " // input to be raw video data
		"-pixel_format rgb24 "
		"-video_size 720x480 "
		"-r 60               " // frames per second
		"-i -                " // read data from the standard input stream
		"-pix_fmt yuv420p    " // to render with Quicktime
		"-vcodec mpeg4       "
		"-an                 " // no audio
		"-q:v 5              " // quality level; 1 <= q <= 32
		"output.mp4          ";

	// Run the ffmpeg command and get pipe to write into its standard input stream.
	FILE * pipe = popen(cmd, "w");
	if (pipe == 0) {
		cout << "error: " << strerror(errno) << endl;
		return 1;
	}

	// Write video frames into the pipe.
	int num_frames = duration_in_seconds * frames_per_second;
	for (int i = 0; i < num_frames; ++i) {
		double time_in_seconds = i / frames_per_second;
		r.draw_frame(time_in_seconds);
		fwrite(frame, 3, W * H, pipe);
	}

	fflush(pipe);
	pclose(pipe);

	cout << "num_frames: " << num_frames << endl;
	cout << "Done." << endl;

	return 0;
}
