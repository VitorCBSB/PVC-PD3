/*
 * Rectangle.h
 *
 *  Created on: 28/10/2014
 *      Author: vitor
 */

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include "opencv2/opencv.hpp"
#include <math.h>

class Rectangle {
public:
	cv::RotatedRect rect;

	Rectangle(
			cv::RotatedRect rect = cv::RotatedRect(cv::Point2f(0, 0),
					cv::Size2f(0, 0), 0)) :
			rect(rect) {
	}

	void render(cv::Mat& image);

private:
	void desenhar_bloco(cv::Mat& imagem, cv::Point ponto, int altura,
			int largura, cv::Vec3b cor);
};

#endif /* RECTANGLE_H_ */
