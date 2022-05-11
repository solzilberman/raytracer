#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw3.h>
#include "tracer.h"
#include "renderer.h"
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")

// TESTING VARS
#define TESTING_FLAG 0 // testing flag - if enabled only shoot one ray and check color
#define GL_FLAG 1 // render live display via opengl
#define MJCF_LOAD_FLAG 0 // flag to load mjcf into scene
#define ANIMATION_FLAG 0 // animation flag
#define ANIMATION_LENGTH 100 // num frame to trace if animation enabled

// global objects
Scene* scene;
RayTracer* rt;
LiveRenderManager* renderer;

void printConfigInfo(RayTracer* rt) {
	std::cout << CYAN_BOLD + "*******************************************************************" + WHITE << "\n";
	std::cout << BLUE_BOLD + "[info]" + WHITE + " Rendering scene '" << rt->_scene->_input_file << "'\n";
	std::cout << BLUE_BOLD + "[info] " + WHITE + "Size: " << rt->_scene->_screen->_width << "x" << rt->_scene->_screen->_height << "\n";
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	double eta = (double)rt->_scene->_screen->_width * (double)rt->_scene->_screen->_height * ((double)rt->_scene->_rss+1) * (double)1.6e-6 * (double)rt->_scene->_objects.size() / 10.f + r;
	std::cout << BLUE_BOLD + "[info] " + WHITE + "Estimated time to render: " << eta << " seconds.\n";
	std::cout << CYAN_BOLD + "*******************************************************************" + WHITE << "\n";
	std::cout << GREEN_BOLD + "[status]" + WHITE + " shooting rays into image...\n";
	std::cout << CYAN_BOLD + "*******************************************************************\n";
}

void printStats(RayTracer* rt, double elapsed_time) {
	double et = elapsed_time;
	std::cout << GREEN_BOLD + "[status]" + WHITE + " raytracing complete - saving image...\n";
	std::cout << CYAN_BOLD + "*******************************************************************\n";
	std::cout << BLUE_BOLD + "[info] " + WHITE + "Time elapsed: " << et << " seconds.\n";
	std::cout << BLUE_BOLD + "[info] RT/S: " + WHITE << ((double)rt->_scene->_screen->_width * (double)rt->_scene->_screen->_height * ((double)rt->_scene->_rss + .5f)) / et << " rays / second.\n";
}

void initGeom() {
	//std::vector<Sphere*> ss;// = _genAutomata(3, -40, 3);
	/*for (auto s : ss) {
		rt->_scene->_addObject(s);
	}*/
	//int n = 10;
	//float inc = 2 * M_PI / (float)n;
	//for (int i = 0; i < 2*n; i++) {
	//	float a = 0.f;
	//	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	for (int j = 0; j < n; j++) {
	//		int t = r > .5000f;
	//		rt->_scene->_addObject(
	//			new Sphere(
	//				2.f,//_radius = r;
	//				glm::vec3(cos(a+i) * 25, 15+sin(a+i) * 25, -55 + (i * 5)),//_center = c;
	//				glm::vec3(r, g, b),//_diff = diff;
	//				glm::vec3(1, 1, 1),//_spec = spec;
	//				50.f//_s = s;
	//			));
	//		a += inc;

	//	}
	//}


	//rt->_scene->_addObject(
	//	new Triangle(
	//		glm::vec3(-50,-50,-30),
	//		glm::vec3(50, -50, -30),
	//		glm::vec3(0,0,-55),
	//		glm::vec3(.5,.5,5),//_diff = diff;
	//		glm::vec3(1, 1, 1),//_spec = spec;
	//		50.f
	//	));

	//rt->_scene->_addObject(new Sphere(
	//	150.f,//_radius = r;
	//	glm::vec3(0, 100, 50),//_center = c;
	//	glm::vec3(.5, .5, .5),//_diff = diff;
	//	glm::vec3(1, 1, 1),//_spec = spec;
	//	50.f//_s = s;
	//));

	//rt->_scene->_addObject(new Sphere(
	//	10.f,//_radius = r;
	//	glm::vec3(0, 15, 0),//_center = c;
	//	glm::vec3(.5, .5, .5),//_diff = diff;
	//	glm::vec3(1, 1, 1),//_spec = spec;
	//	50.f//_s = s;
	//));


	//rt->_scene->_addObject(new Cap(
	//	30.f,//_radius = r;
	//	glm::vec3(0, 30, 0),//from
	//	glm::vec3(50, 130, 0),//to 
	//	glm::vec3(.33,.3,.27),//_diff = diff;
	//	glm::vec3(1,1,1),//_spec = spec;
	//	500.f//_s = s;
	//));
	//float arr[] = { 0.043821, 0.190000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, -1.000000, 0.000000, 1.000000, 0.000000 };
	//float r = arr[0] * 100.f;
	//float size = arr[1]*100.f;
	//glm::vec3 from = glm::vec3(0.f, 0.f, 0.f);
	//glm::vec3 to = glm::vec3(0.f, size * 2.f, 0.f);
	//from = glm::mat3(arr[6], arr[7], arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14]) * from;
	//to = glm::mat3(arr[6], arr[7], arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14]) * to;
	//glm::vec3 tr = glm::vec3(arr[3], arr[4], arr[5]);
	//from += tr;
	//to += tr;
	//rt->_scene->_addObject(new Cap(
	//	r,//_radius = r;
	//	from,//from
	//	to,//to 
	//	glm::vec3(1,1,1),//_diff = diff;
	//	glm::vec3(1, 1, 1),//_spec = spec;
	//	500.f//_s = s;
	//));

}

void initTracer() {
	// combine following into one class
	if (MJCF_LOAD_FLAG) 
		scene = new Scene("configs/test", "configs/model.mjcf");
	else 
		scene = new Scene("configs/cornell.json");

	rt = new RayTracer(scene);
}

void runAnimation() {
	rt->_scene->_screen->_animation_mode = ANIMATION_FLAG;
	for (int i = 0; i < ANIMATION_LENGTH; i++) {
		//rt->_scene->_camera->_eye = glm::vec3(60.f*cos((float)i/10.f), 10, 60.f*sin((float)i / 10.f)); // move camera forward
		rt->_scene->_camera->_eye += glm::vec3(0,0,-1.f); // move camera forward
		std::cout << "\r" + YELLOW_BOLD + "[status] " + WHITE + "rendering frame: " << i << " / " << ANIMATION_LENGTH << std::flush;
		rt->_run();
	}
	std::cout << "\n";
}

void liveDisplay() {
	renderer = new LiveRenderManager(rt);
	while (!renderer->_get_window_state())
	{
		renderer->_display();
		renderer->_refresh_frame();
	}
	delete renderer;
}

int main() {
	Timer* timer = new Timer();
	initTracer();
	initGeom();

	if (!TESTING_FLAG) // run full raytrace on scene
	{
		printConfigInfo(rt);
		timer->_start(); // profiler start

		// main raytrace entry point
		if (ANIMATION_FLAG)
			runAnimation();
		else 
			rt->_run();

		timer->_stop(); // profiler end
		printStats(rt, timer->_count());

		if (GL_FLAG) liveDisplay();
	}
	else // TESTING MODE - trace one ray
	{
		Ray* r = new Ray();
		std::cout << "size: " << sizeof(r) << std::endl;
		rt->_set_ray(500, 500, r);
		r->_log();
		glm::vec3 col = rt->_trace_ray(*r, 2);
	}
	delete rt;
	delete timer;
	return 0;
}