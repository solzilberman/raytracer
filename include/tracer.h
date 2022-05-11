#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/random.hpp>
#include <omp.h>
#include <chrono>
#include <ctime>
#include "ray.h"
#include "objects.h"
#include "scene.h"
#include "utils.h"

/* Intersection record structure */
class intersection {
public:
	intersection(int o, double t) {
		_o = o;
		_t = t;
	}
	intersection() {
		_o = -1;
		_t = -1;
	};
	~intersection() {};

	void _log() {
		std::cout << "Intersection := ";
		std::cout << "Object: " << _o << " ";
		std::cout << "t: " << _t << "\n";
	}

	bool operator >(const double t) {
		return _t > t;
	}

	int _o;
	double _t;
};

/* Raytrace class. Runs main tracing algorithm and popoulates png data from calculated colors */
class RayTracer {
public:
	Scene* _scene;
	double bias = 1e-1;
	int _THREAD_COUNT = 10;
	RayTracer(Scene* scene) {
		_scene = scene;
	}
	~RayTracer() {
		delete _scene;
	}

	/* Main raytracer method*/
	void _run() {
		_scene->_rss = _scene->_rss == 0 ? 1.f : _scene->_rss+1; //sampling
		double w = 1.f / (double)_scene->_rss;
		omp_set_dynamic(0); // disable dynamic teams
		#pragma omp parallel for collapse(2) // parralelize nested loop with 50 threads
		for (int i = 0; i < _scene->_screen->_width; i++) {
			for (int j = 0; j < _scene->_screen->_height; j++) {

				// calculate ray 
				Ray ray = Ray(); // init ray object
				_set_ray(i, j, &ray); // now we have ray := o + td
				// calculate color of point
				glm::vec3 color = glm::vec3(0,0,0);// _traceRay(ray, _scene->_max_depth);
				//supersample
				for (int ri = 0; ri < _scene->_rss; ri++) {
					// random gen: https://stackoverflow.com/questions/686353/random-float-number-generation
					float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); //fix this for speed
					float q = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					double ru = (i + r);
					double rv = (j + q);
					_set_ray(ru, rv, &ray); // now we have ray := o + td
					// calculate color of point
					color += (_trace_ray(ray, _scene->_max_depth));
				}
				color *= w;
				if (color.x > 1) color.x = .999;
				if (color.y > 1) color.y = .999;
				if (color.z > 1) color.z = .999;

				// set pixel in framebuffer
				_scene->_screen->_setPixel(i, j, color);
			}
		}
		_scene->_screen->_writePNG();
	}

	/* Calculate the ray and set relative params */
	void _set_ray(double i, double j, Ray* r) {
		glm::vec3 l = glm::normalize(_scene->_camera->_target - _scene->_camera->_eye);
		glm::vec3 v = glm::normalize(glm::cross(l, _scene->_camera->_up));
		glm::vec3 u = glm::cross(v, l);
		double d = 1.f / (glm::tan(glm::radians(_scene->_camera->_fovy / 2)));
		double a = (double)_scene->_screen->_width / (double)_scene->_screen->_height;
		glm::vec3 ll = _scene->_camera->_eye + ((float)d * l) - ((float)a * v) - u;
		glm::vec3 p = ll + (2 * (float)a * v * (((float)i) / (float)_scene->_screen->_width)) + (2.f * u * ((float)_scene->_screen->_height-(float)j)/(float)_scene->_screen->_height);
		r->_org = _scene->_camera->_eye;
		r->_dir = glm::normalize(p - r->_org);
	}

	/* calculate color for Ray r with depth bounces */
	glm::vec3 _trace_ray(Ray ray, int depth) {
		glm::vec3 color(0.f);
		intersection inter = _closest_intersection(ray); // get first intersection
		if (inter._o < 0) {
			if (_scene->_skybox) return _scene->_skybox->_get_color_at(ray); //if skybox exists return pixel at ray@1000
			else return _scene->_bg_color; // else return background color
		}// if none then background
		glm::vec3 p = ray._get_point(inter._t); // get point from intersection struct
		glm::vec3 norm = _scene->_objects[inter._o]->_get_normal(p);
		glm::vec3 shadow_norm = glm::dot(ray._dir, norm) > 1e-5 ? -norm : norm;
		p = p + (shadow_norm * (float)bias);
		std::vector<int> contributedLights = _trace_shadow_rays(p);
		for (int i = 0; i < contributedLights.size(); i++) {
			color += _calc_lighting(p, _scene->_lights[contributedLights[i]], _scene->_objects[inter._o], ray, depth);
		}
		contributedLights.clear();// mem mang
		if (depth <= 0) {
			return color;// mem mang
		}
		if (_scene->_objects[inter._o]->_reflect_flag) {
			glm::vec3 reflected = glm::reflect(ray._dir, norm); //get reflected ray
			Ray ref(p, reflected); //get reflected ray
			color += _trace_ray(ref, depth-1);
		}

		/*
		refracted=Refracted(ray,p); //get refracted
		color+=Trace(reflected,depth-1);//recursive down
		color+=Trace(refracted,depth-1);
		return color;
		*/
		return color;
	}

	/* computes closes object/ray intersection to origin of Ray ray*/
	intersection _closest_intersection(Ray ray) {
		intersection hits = intersection(-1,-1);
		double closestDist = INFINITY;
		for (int i = 0; i < _scene->_objects.size(); i++) {
			double t = _scene->_objects[i]->_hit(ray);
			if (t > 1e-5 && t < closestDist) {
				hits = intersection(i, t);
				closestDist = t;
			}
		}
		return hits;
	}
	
	/* computes all shadow rays and returns indices of contributing lights for 3D point p */
	std::vector<int> _trace_shadow_rays(glm::vec3 p) {
		std::vector<int> contibutingLights;
		contibutingLights.reserve(_scene->_lights.size());
		for (int i = 0; i < _scene->_lights.size(); i++) {
			glm::vec3 tmpDir = glm::normalize(_scene->_lights[i]->_pos - p);
			Ray tmpRay(p, tmpDir);
			intersection inter = _closest_intersection(tmpRay); // get first intersection
			if (inter._o < 0 || inter._t > glm::length(_scene->_lights[i]->_pos - p)) {
				contibutingLights.emplace_back(i);
			}
		}
		return contibutingLights;
	}

	/* compute color RGBA of pixel i,j at 3d point p*/
	glm::vec3 _calc_lighting(glm::vec3 p, Light* light, Object * object, Ray ray, int depth) {
		glm::vec3 n;
		glm::vec3 c(0.f);
		n = object->_get_normal(p);
		//n = glm::normalize(n);
		//inside or outside normal
		n = glm::dot(ray._dir, n) > 1e-5 ? -n : n;
		glm::vec3 l = glm::normalize(light->_pos - p);
		glm::vec3 v = glm::normalize(-p);
		glm::vec3 r = glm::reflect(-l,v);
		double sdot = glm::max(glm::dot(l, n), 0.f);
		glm::vec3 diff = light->_diff * object->_diff * (float)sdot;
		glm::vec3 spec(0.f);
		if (sdot < 1e-5) return diff; //if no spec then just return diffuse
		spec = light->_spec * object->_spec * (float)glm::pow(sdot, (double)object->_s);
		return (spec+diff);
		//return glm::vec3(1.f,1.f,1.f);
	}
	
	

};

