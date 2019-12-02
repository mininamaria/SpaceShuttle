
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	int counter = 0;
	const String windowName = "The_Window";							// Name of the window
	namedWindow(windowName, cv::WINDOW_AUTOSIZE);					// Create a window
	cv::VideoCapture cap;											// Create an object for getting images
	Mat frame;														// Кадр
	if (argc > 1)
		cap.open(string(argv[1]));
	else {
		cout << "Usage: progname video.mpg" << endl;
		return -1;
	}
	while (1) {
		cap >> frame;
		if (frame.empty()) break;									// The end of the movie
		cv::imshow(windowName, frame);
		if (cv::waitKey(33) >= 0) break;	
		if (counter % 10 == 0) {
			const string filename = "capture" + to_string(counter/10) + ".jpg";
			imwrite(filename, frame);
		}
		counter++;
	}
	return 0;
}