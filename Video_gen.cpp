#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <stdio.h>
#include <cassert>
#include <cstring>

typedef unsigned char byte;

using namespace std;

#define W 720
#define H 480

const double frames_per_second = 50;
const int duration_per_second = 6;

unsigned frame[H][W][6];

void black_screen(){ 
	memset(frame, 0 ,sizeof(frame));
	}
	
void rect_figure(int x, int y, int h, int w, byte r, byte g, byte b);

// draws the black frame where rectangle will move on
void frame_drawn(double t){

	black_screen();
	const double pps = 100;
	rect_figure(150 + t * pps, 150 + t * pps, 50, 100, 0xff, 0xff, 0xff);
}

void clamp(int * x, int * y) {

	if (*x < 0) 
		*x = 0; 
		
		else if (*x >= W)
			*x = W - 1;
			
	if (*y < 0)
		*y = 0;
		
		else if (*y >= H)
			*y = H - 1;
}

bool outside_frame(int * x, int * y) {
	return *x < 0 || *x >= W || *y < 0 || *y >= H;
}

// draws the rect
void rect_figure(int x, int y, int h, int w, byte r, byte g, byte b) {
	
	if (outside_frame(&x, &y))
		return;
	int x0 = x;
	int x1 = x + w;
	int y0 = y;
	int y1 = y + h;
	
	clamp(&x0, &y0);
	clamp(&x1, &y1);
	
	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			frame[H][W][0] = r;
			frame[H][W][1] = g;
			frame[H][W][2] = b;
		}
	}
}

int main(int argc, char * argv[]) {
	// Construct the ffmpeg command to run.
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
	int num_frames = duration_per_second * frames_per_second;
	for (int i = 0; i < num_frames; ++i) {
		double time_in_seconds = i / frames_per_second;
		frame_drawn(time_in_seconds);
		fwrite(frame, 3, W * H, pipe);
	}

	fflush(pipe);
	pclose(pipe);
	
	cout << "num_frames: " << num_frames << endl;
	cout << "Done." << endl;

	return 0;
}
