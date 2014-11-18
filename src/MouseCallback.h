/*
 * MouseCallback.h
 *
 *  Created on: 30/09/2014
 *      Author: vitor
 */

#ifndef MOUSECALLBACK_H_
#define MOUSECALLBACK_H_

#include <functional>
#include "opencv2/opencv.hpp"

template<typename T>
class MouseCallback {
private:
	static std::function<void(int, int, int, T&)> callback_func;

	static void cv_callback(int evento, int x, int y, int flags, void* params) {
		T* data = (T*) params;
		callback_func(evento, y, x, *data);
	}

public:
	static void set_mouse_callback(const std::string& nome_janela,
			std::function<void(int, int, int, T&)> callback, T& data) {
		callback_func = callback;
		cv::setMouseCallback(nome_janela, cv_callback, (void*) &data);
	}
};

template<typename T>
std::function<void(int, int, int, T&)> MouseCallback<T>::callback_func;

#endif /* MOUSECALLBACK_H_ */
