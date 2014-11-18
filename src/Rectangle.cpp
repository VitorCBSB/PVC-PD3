/*
 * Rectangle.cpp
 *
 *  Created on: 28/10/2014
 *      Author: vitor
 */

#include "Rectangle.h"

void Rectangle::desenhar_bloco(cv::Mat& imagem, cv::Point ponto, int altura,
		int largura, cv::Vec3b cor) {
	for (int i = ponto.y - altura / 2; i < ponto.y + altura / 2; i++) {
		for (int j = ponto.x - largura / 2; j < ponto.x + largura / 2; j++) {
			imagem.at<cv::Vec3b>(i, j) = cor;
		}
	}
}

void Rectangle::render(cv::Mat& image) {
	cv::Point2f vertices[4];
	rect.points(vertices);
	for (int i = 0; i < 4; i++) {
		desenhar_bloco(image, vertices[i], 4, 4, cv::Vec3b(0, 0, 255));
	}

	for (int i = 0; i < 4; i++) {
		cv::line(image, vertices[i], vertices[(i + 1) % 4],
				cv::Scalar(0, 0, 255));
	}
}
