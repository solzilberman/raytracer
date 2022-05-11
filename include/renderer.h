#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include "tracer.h"

#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

class LiveRenderManager {
public:
	GLFWwindow* _window;
	RayTracer* _rt;
	LiveRenderManager(RayTracer* rt) {
		_rt = rt;
		_init_GL();
	}

	~LiveRenderManager() {
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	void _init_GL() {
		if (!glfwInit()) std::cout << "glfw init failed!\n";
		_window = glfwCreateWindow(_rt->_scene->_screen->_width, _rt->_scene->_screen->_height, "My Title", NULL, NULL);
		if (!_window) std::cout << "window init failed!\n";
		glfwSetKeyCallback(_window, _key_callback);
		glfwMakeContextCurrent(_window); // make curr window gl context
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		glfwSwapInterval(1);
	}

	static void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	void _display() {

		glViewport(0, 0, _rt->_scene->_screen->_width, _rt->_scene->_screen->_height);
		glClear(GL_COLOR_BUFFER_BIT);
		glRasterPos2f(-1, 1);
		glPixelZoom(1, -1);
		glDrawPixels(_rt->_scene->_screen->_width, _rt->_scene->_screen->_height, GL_RGBA, GL_UNSIGNED_BYTE, &_rt->_scene->_screen->_data[0]);
	}

	int _get_window_state() {
		return glfwWindowShouldClose(_window);
	}

	void _refresh_frame() {
		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
};