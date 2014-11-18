//============================================================================
// Name        : PVC-PD3.cpp
// Author      : Vitor Coimbra de Oliveira
// Version     :
// Copyright   : MIT
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include "Rectangle.h"
#include "rotate2D.h"
#include "opencv2/opencv.hpp"
#include "opencv2/legacy/legacy.hpp"

enum {
	DRAWING, DRAWN, NOTHING
} state = NOTHING;

Rectangle analysis_box;
cv::Rect temp_box;
bool playing = false;
int lambda_slider = 1;
const int lambda_max = 100;

void mouse_callback(int event, int x, int y, int flags, void* params) {
	if (!playing) {
		switch (state) {
		case DRAWING:
			switch (event) {
			case CV_EVENT_LBUTTONDOWN:
				analysis_box.rect = cv::RotatedRect(
						cv::Point2f(temp_box.x + temp_box.width / 2,
								temp_box.y + temp_box.height / 2),
						cv::Size2f(temp_box.width, temp_box.height), 0);
				state = DRAWN;
				break;
			case CV_EVENT_MOUSEMOVE:
				temp_box.width = x - temp_box.x;
				temp_box.height = y - temp_box.y;
				break;
			}
			break;
		case DRAWN:
			if (event == CV_EVENT_LBUTTONDOWN) {
				analysis_box.rect.center = cv::Point(x, y);
			}
			break;
		case NOTHING:
			if (event == CV_EVENT_LBUTTONDOWN) {
				temp_box = cv::Rect(x, y, 0, 0);
				state = DRAWING;
			}
			break;
		default:
			std::cerr << "wat" << std::endl;
			exit(1);
		}
	}
}

int handle_keyboard(int delay) {
	auto pressed_key = cv::waitKey(delay);

	switch (pressed_key) {
	case 'p':
		playing = !playing;
		break;
	case 'u':
		analysis_box.rect.angle += 3.0;
		break;
	case 'i':
		analysis_box.rect.angle -= 3.0;
		break;
	case 'j':
		analysis_box.rect.size.width -= 2;
		break;
	case 'k':
		analysis_box.rect.size.width += 2;
		break;
	case 'n':
		analysis_box.rect.size.height -= 2;
		break;
	case 'm':
		analysis_box.rect.size.height += 2;
		break;
	case 'z':
		state = NOTHING;
		break;
	}

	return pressed_key;
}

cv::Mat rotate_image(const cv::Mat& image, cv::Point center, double angle) {
	cv::Mat rotated;
	auto M = cv::getRotationMatrix2D(analysis_box.rect.center,
			analysis_box.rect.angle, 1.0);
	cv::warpAffine(image, rotated, M, image.size(), cv::INTER_CUBIC);
	return rotated;
}

cv::Mat get_selected_area(const cv::Mat& image) {
	cv::Mat cropped;
	auto rotated = rotate_image(image, analysis_box.rect.center,
			-analysis_box.rect.angle);
	cv::getRectSubPix(rotated, analysis_box.rect.size, analysis_box.rect.center,
			cropped);
	return cropped;
}

void desenhar_bloco(cv::Mat& imagem, cv::Point ponto, int altura, int largura,
		cv::Vec3b cor) {
	for (int i = ponto.y - altura / 2; i < ponto.y + altura / 2; i++) {
		for (int j = ponto.x - largura / 2; j < ponto.x + largura / 2; j++) {
			imagem.at<cv::Vec3b>(i, j) = cor;
		}
	}
}

void render_temp_box(cv::Mat& image) {
	desenhar_bloco(image, cv::Point(temp_box.x, temp_box.y), 4, 4,
			cv::Vec3b(0, 0, 255));
	desenhar_bloco(image, cv::Point(temp_box.x + temp_box.width, temp_box.y), 4,
			4, cv::Vec3b(0, 0, 255));
	desenhar_bloco(image, cv::Point(temp_box.x, temp_box.y + temp_box.height),
			4, 4, cv::Vec3b(0, 0, 255));
	desenhar_bloco(image,
			cv::Point(temp_box.x + temp_box.width,
					temp_box.y + temp_box.height), 4, 4, cv::Vec3b(0, 0, 255));

	cv::rectangle(image, cv::Point(temp_box.x, temp_box.y),
			cv::Point(temp_box.x + temp_box.width,
					temp_box.y + temp_box.height), cv::Scalar(0, 0, 255));
}

void motion_analysis(const cv::Mat& last_frame, const cv::Mat& current_frame,
		cv::Mat& motion_x, cv::Mat& motion_y, double lambda) {
	cv::Mat last_gray;
	cv::Mat current_gray;
	cv::cvtColor(last_frame, last_gray, CV_BGR2GRAY);
	cv::cvtColor(current_frame, current_gray, CV_BGR2GRAY);

	CvSize frame_size;
	frame_size.width = current_frame.cols;
	frame_size.height = current_frame.rows;

	IplImage last_gray_ipl = last_gray;
	IplImage current_gray_ipl = current_gray;
	IplImage* motion_x_ipl = cvCreateImage(frame_size, IPL_DEPTH_32F, 1);
	IplImage* motion_y_ipl = cvCreateImage(frame_size, IPL_DEPTH_32F, 1);
	cvCalcOpticalFlowHS(&last_gray_ipl, &current_gray_ipl, 0, motion_x_ipl,
			motion_y_ipl, lambda, cvTermCriteria(
			CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, current_frame.cols, 1e-6));

	motion_x = cv::Mat(motion_x_ipl);
	motion_y = cv::Mat(motion_y_ipl);
}

void draw_motion(const cv::Mat& motion, cv::Mat& area_result) {
	for (int i = 0; i < motion.rows; i++) {
		for (int j = 0; j < motion.cols; j++) {
			if (motion.at<uchar>(i, j) > 0 || motion.at<uchar>(i, j) > 0) {
				area_result.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
			}
		}
	}
}

cv::Mat deal_with_it(const cv::Mat& last_frame, const cv::Mat& current_frame) {
	auto area_last = get_selected_area(last_frame);
	auto area_current = get_selected_area(current_frame);
	cv::Mat motion_x, motion_y, area_result;
	cv::Mat filtered_motion_x, filtered_motion_y;
	cv::Mat bin_motion_x, bin_motion_y;
	cv::Mat open_motion_x, open_motion_y;
	auto result = current_frame.clone();
	float threshold = 1;

	area_result = area_current.clone();
	motion_analysis(area_last, area_current, motion_x, motion_y, lambda_slider);

	// Filtra os mapas.
	cv::medianBlur(motion_x, filtered_motion_x, 3);
	cv::medianBlur(motion_y, filtered_motion_y, 3);

	// Binariza os dois mapas.
	cv::threshold(filtered_motion_x, bin_motion_x, threshold, 255,
			cv::THRESH_BINARY);
	cv::threshold(filtered_motion_y, bin_motion_y, threshold, 255,
			cv::THRESH_BINARY);
	bin_motion_x.convertTo(bin_motion_x, CV_8U);
	bin_motion_y.convertTo(bin_motion_y, CV_8U);

	auto structuring_element = cv::getStructuringElement(cv::MORPH_RECT,
			cv::Size(3, 3));
	cv::morphologyEx(bin_motion_x, open_motion_x, cv::MORPH_OPEN,
			structuring_element);
	cv::morphologyEx(bin_motion_y, open_motion_y, cv::MORPH_OPEN,
			structuring_element);

	cv::Mat laplacian_motion_x, laplacian_motion_y;
	cv::Laplacian(open_motion_x, laplacian_motion_x, CV_8U);
	cv::Laplacian(open_motion_y, laplacian_motion_y, CV_8U);

	// Desenha os movimentos.
	draw_motion(laplacian_motion_x, area_result);
	draw_motion(laplacian_motion_y, area_result);

	auto box = cv::Rect(cv::Point2f(0, 0), analysis_box.rect.size);
	auto rotated_result = rotateImage(area_result, nullptr, box,
			-analysis_box.rect.angle);

	// Transfere as mudanças de volta pra imagem original.
	int k = 0;
	for (int i = analysis_box.rect.center.y - rotated_result.rows / 2;
			i < analysis_box.rect.center.y + rotated_result.rows / 2;
			i++, k++) {
		int l = 0;
		for (int j = analysis_box.rect.center.x - rotated_result.cols / 2;
				j < analysis_box.rect.center.x + rotated_result.cols / 2;
				j++, l++) {
			auto pixel = rotated_result.at<cv::Vec3b>(k, l);
			if (pixel.val[0] != 0 || pixel.val[1] != 0 || pixel.val[2] != 0) {
				result.at<cv::Vec3b>(i, j) = pixel;
			}
		}
	}

	return result;
}

void on_trackbar(int, void*) {
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Uso: " << argv[0] << " nome_video" << std::endl;
		exit(0);
	}

	cv::VideoCapture video = cv::VideoCapture(std::string(argv[1]));
	cv::namedWindow("Video");
	cv::setMouseCallback("Video", mouse_callback, nullptr);

	cv::createTrackbar("Lambda", "Video", &lambda_slider, 100, on_trackbar,
			nullptr);

	cv::Mat current_frame;
	cv::Mat last_frame;
	video.read(last_frame);
	current_frame = last_frame.clone();

	while (true) {
		if (playing) {
			last_frame = current_frame.clone();
			if (!video.read(current_frame)) {
				break;
			}
		}
		cv::Mat modified_image;

		switch (state) {
		case DRAWING:
			modified_image = current_frame.clone();
			render_temp_box(modified_image);
			break;
		case DRAWN:
			modified_image = deal_with_it(last_frame, current_frame);
			analysis_box.render(modified_image);
			break;
		default:
			modified_image = current_frame.clone();
			break;
		}

		cv::imshow("Video", modified_image);

		auto pressed_key = handle_keyboard(16);
		if (pressed_key == 'q') {
			break;
		}
	}

	return 0;
}
