/*
 * rotate2D.cpp
 *
 *  Created on: 29/10/2014
 *      Author: vitor
 */

#include "opencv2/opencv.hpp"
#include <math.h>

// ROTATE p by R
/**
 * Rotate p according to rotation matrix (from getRotationMatrix2D()) R
 * @param R     Rotation matrix from getRotationMatrix2D()
 * @param p     Point2f to rotate
 * @return      Returns rotated coordinates in a Point2f
 */
cv::Point2f rotPoint(const cv::Mat &R, const cv::Point2f &p) {
	cv::Point2f rp;
	rp.x = (float) (R.at<double>(0, 0) * p.x + R.at<double>(0, 1) * p.y
			+ R.at<double>(0, 2));
	rp.y = (float) (R.at<double>(1, 0) * p.x + R.at<double>(1, 1) * p.y
			+ R.at<double>(1, 2));
	return rp;
}

//COMPUTE THE SIZE NEEDED TO LOSSLESSLY STORE A ROTATED IMAGE
/**
 * Return the size needed to contain bounding box bb when rotated by R
 * @param R     Rotation matrix from getRotationMatrix2D()
 * @param bb    bounding box rectangle to be rotated by R
 * @return      Size of image(width,height) that will compleley contain bb when rotated by R
 */
cv::Size rotatedImageBB(const cv::Mat &R, const cv::Rect &bb) {
	//Rotate the rectangle coordinates
	std::vector<cv::Point2f> rp;
	rp.push_back(rotPoint(R, cv::Point2f(bb.x, bb.y)));
	rp.push_back(rotPoint(R, cv::Point2f(bb.x + bb.width, bb.y)));
	rp.push_back(rotPoint(R, cv::Point2f(bb.x + bb.width, bb.y + bb.height)));
	rp.push_back(rotPoint(R, cv::Point2f(bb.x, bb.y + bb.height)));
	//Find float bounding box r
	float x = rp[0].x;
	float y = rp[0].y;
	float left = x, right = x, up = y, down = y;
	for (int i = 1; i < 4; ++i) {
		x = rp[i].x;
		y = rp[i].y;
		if (left > x)
			left = x;
		if (right < x)
			right = x;
		if (up > y)
			up = y;
		if (down < y)
			down = y;
	}
	int w = (int) (right - left + 0.5);
	int h = (int) (down - up + 0.5);
	return cv::Size(w, h);
}

/**
 * Rotate region "fromroi" in image "fromI" a total of "angle" degrees and put it in "toI" if toI exists.
 * If toI doesn't exist, create it such that it will hold the entire rotated region. Return toI, rotated imge
 *   This will put the rotated fromroi piece of fromI into the toI image
 *
 * @param fromI     Input image to be rotated
 * @param toI       Output image if provided, (else if &toI = 0, it will create a Mat fill it with the rotated image roi, and return it).
 * @param fromroi   roi region in fromI to be rotated.
 * @param angle     Angle in degrees to rotate
 * @return          Rotated image (you can ignore if you passed in toI
 */
cv::Mat rotateImage(const cv::Mat &fromI, cv::Mat *toI, const cv::Rect &fromroi,
		double angle) {
	//CHECK STUFF
	// you should protect against bad parameters here ... omitted ...

	//MAKE OR GET THE "toI" MATRIX
	cv::Point2f cx((float) fromroi.x + (float) fromroi.width / 2.0,
			fromroi.y + (float) fromroi.height / 2.0);
	cv::Mat R = getRotationMatrix2D(cx, angle, 1);
	cv::Mat rotI;
	if (toI)
		rotI = *toI;
	else {
		cv::Size rs = rotatedImageBB(R, fromroi);
		rotI.create(rs, fromI.type());
	}

	//ADJUST FOR SHIFTS
	double wdiff = (double) ((cx.x - rotI.cols / 2.0));
	double hdiff = (double) ((cx.y - rotI.rows / 2.0));
	R.at<double>(0, 2) -= wdiff; //Adjust the rotation point to the middle of the dst image
	R.at<double>(1, 2) -= hdiff;

	//ROTATE
	cv::warpAffine(fromI, rotI, R, rotI.size(), cv::INTER_CUBIC,
			cv::BORDER_CONSTANT);

	//& OUT
	return (rotI);
}
