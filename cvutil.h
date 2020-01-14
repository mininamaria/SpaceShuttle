#pragma once
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <utility>

#define SET_PARAM_INT(param) \
			if (kv.count(#param))\
				 params.param = (int)kv.at(#param);\
			 else \
				throw runtime_error(string("Can't find parameter ") + string(#param) + string(" in ") + filename)
#define SET_PARAM_DOUBLE(param) \
			if (kv.count(#param))\
				 params.param = kv.at(#param);\
			 else \
				throw runtime_error("Can't find parameter param in " + filename)


using namespace std;

struct parameters {
	int num_x = 0;	// ���-�� ��������� �� �����������             
	int num_y = 0;	// ���-�� ��������� �� ���������               
	double delta_x = 21.5; // ������ ������ ����������
	double delta_y = 33.0; // ����� ������ ����������                   
	int start_x = 28; // ���������� x ����� ������ ����� ���������         
	int start_y = 23; // ���������� y ����� ������ ����� ���������        
	vector<vector<bool>> positions;
};

void clear_spaces(string& s) {
	s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
}

bool parse_config(parameters& params, string filename) {
	map<string, double> kv;
	filename = filename.substr(0, filename.find(".")) + ".cfg"; // �� ���� ������ ��� �����-�����. �������� ��� �� file.cfg
	ifstream fin(filename.c_str());
	
	if (!fin.is_open()) throw runtime_error("Can't open " + filename); 	// ���� ���� �� �������� - ������ exceprion
	
	std::string line;
	while (getline(fin, line)) {
		//cout << endl << "$debug";
		clear_spaces(line);
		if (line[0] != '/' && !isdigit(line[0]) && line.size() > 1) {
			string key = line.substr(0, line.find("="));
			string value = line.substr(line.find("=") + 1);
			//cout << '"' << key << "\" \"" << value << '"' << endl;
			kv[key] = stod(value);
		}
		else if (line[0] == '0' || line[0] == '1') {
			vector<bool> temp;
			for (auto s : line) {
				if (s == '0') temp.push_back(false);
				else if (s == '1') temp.push_back(true);
				
				else throw runtime_error ("Parse config error. Waiting for 0 or 1. Got "+ s);
				
				//cout << temp.back();
			}
			params.positions.push_back(temp);
		}
	}
	fin.close();
	

	SET_PARAM_DOUBLE(delta_x);
	SET_PARAM_DOUBLE(delta_y);
	SET_PARAM_INT(start_x);
	SET_PARAM_INT(start_y);
	SET_PARAM_INT(num_x);
	SET_PARAM_INT(num_y);


	if ((int)params.positions.size() != params.num_y) 
		throw runtime_error("Parse config error. num_y is " + to_string(params.positions.size()) + " ,but must be " + to_string(params.num_y));
	

	for (auto v : params.positions) {
		if ((int)v.size() != (int)params.num_x) 
			throw runtime_error("Parse config error. num_x is " + to_string(v.size()) + " ,but must be " + to_string(params.num_x)); 
	}
	return true;
}

void draw_greed(parameters params, cv::Mat frame, const cv::Scalar Color) {
	for (int i = 0; i < params.num_y; i++) {
		for (int j = 0; j < params.num_x; j++) {
			cv::Point tl((int)(params.start_x + params.delta_x * j), (int)(params.start_y + params.delta_y * i));											// Top / Left
			cv::Point br((int)(params.start_x + params.delta_x * (j + 1)), (int)(params.start_y + params.delta_y * (i + 1)));									// Bottom / Right
			if (params.positions.at(i).at(j)) {
				cv::rectangle(frame, tl, br, Color, 1, 0);
			}
		}
	}
}

pair<char,char> UTF2OEM(char* src = NULL) {
	int length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, NULL, 0);
	if (length > 0)
	{
		wchar_t* wide = new wchar_t[length];
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, wide, length);
		// convert it to ANSI
		char* ansi = new char[length];
		char* ansi2 = new char[length];
		WideCharToMultiByte(CP_OEMCP, 0, wide, -1, ansi, length, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi2, length, NULL, NULL);
		//delete[] wide;
		return make_pair(ansi[0], ansi2[0]);
	}
	return make_pair(NULL, NULL);;
}

pair<char,char> recog_char(tesseract::TessBaseAPI* api,parameters params,cv::Mat dst, int x,int y) {
	//			api->TesseractRect(dst.data, dst.channels(), dst.step1(), 26, 26, 20, 24);
	api->TesseractRect(dst.data, dst.channels(), (int)dst.step1(), params.start_x+ (int)params.delta_x * x , 
																   params.start_y+ (int)params.delta_y * y ,
															       (int)params.delta_x, (int)params.delta_y);
	api->Recognize(0);
	char* outText;
	outText = api->GetUTF8Text();
	// convert multibyte UTF-8 to wide string UTF-16
	return UTF2OEM(outText);
}

void dump_images(map<char, cv::Mat> samples,string extention) {
	for (pair<char, cv::Mat> img : samples) {
		char chr = img.first;
		string fname = string(1, img.first) + extention;
		imwrite(fname, img.second);
	}
}

int recog_Mat(tesseract::TessBaseAPI* api, parameters &params, cv::Mat dst, map<char, cv::Mat>& samples) {
	int char_counter=0;
	for (int i = 0; i < params.num_y; i++) {
		for (int j = 0; j < params.num_x; j++) {
			if (params.positions[i][j]) {
				char rec_char = recog_char(api, params, dst, j, i).second;
				if (rec_char != NULL) {
					char_counter++;
					cv::Mat imageCut = dst(cv::Rect((int)(params.start_x + params.delta_x * j),
						(int)(params.start_y + params.delta_y * i),
						(int)params.delta_x,
						(int)params.delta_y));
					samples[rec_char].push_back(imageCut);
				}
			}
		}
	}
	return char_counter;
}