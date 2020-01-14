#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "cvutil.h"
using namespace cv;
using namespace std;


int main(int argc, char** argv)
{
	parameters params;
	int counter = 0;
	const cv::Scalar Color(100, 0, 255);
	const String windowName = "The_Window";							// Name of the window
	cv::VideoCapture cap;											// Create an object for getting images
	cv::Mat frame;														// Кадр
	cv::Mat dst;
	std::map<char, cv::Mat> samples;
	// Инициализируем tesseract
	tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
	if (api->Init(NULL, "rus")) {
		cerr << "Could not initialize tesseract" << endl;
		exit(1);
	}
	api->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);                 //Распознаем посимвольно
	api->SetVariable("user_defined_dpi", "96");
	// Если мы здесь, то tesseract инициализирован

	if (argc > 1)
		cap.open(string(argv[1]));
	else {
		cout << "Usage: progname video.mpg" << endl;
		return -1;
	}
	
	try // Попробуем распарсить конфиг. Если не получится - выйдем.
	{
		parse_config(params, string(argv[1]));
	}
	catch (const std::exception& ex)
	{
		cout << "Some shit happens: " << ex.what() << endl;
		exit(-1);
	}

	namedWindow(windowName, cv::WINDOW_AUTOSIZE);					// Create a window
	// treshholds
	int fixed_threshold = 110;													// Наиболее оптимальное значение
	//int block_size = 3;
	//int offset = 0;
	int inv = 1;
	cv::createTrackbar("fix_threshold", windowName, &fixed_threshold, 255, NULL, NULL);
	//cv::createTrackbar("block_size", windowName, &block_size, 255, NULL, NULL);
	//cv::createTrackbar("offset", windowName, &offset, 255, NULL, NULL);
	//cv::createTrackbar("inv", windowName, &inv, 1, NULL, NULL);
	// end of treshholds

	while (1) {
		cap >> frame;
		if (frame.empty()) break;											// The end of the movie
		
		cv::threshold(frame, dst, double(fixed_threshold), 255, inv);
		//		block_size += 1 - block_size % 2;
		//		cv::adaptiveThreshold(frame, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, inv, block_size, offset);

		//draw_greed(params, dst, Color);									//Рисуем сетку на экране
		// Capturing frames
		cv::imshow(windowName, dst);
		//обработка нажатий клавиш
		int key = cv::waitKey(100);
		if (key == 27) break;
		else if (key == 32) {
			recog_Mat(api, params, dst, samples);		
			}
		} 

		/*
		if (counter % 100 == 0) {
			const string filename = "capture" + to_string(counter / 100) + ".jpg";
			imwrite(filename, frame);
		}
		counter++;
		*/
	
		dump_images(samples, ".jpg");
	
	return 0;
}
