#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "ray.h"

class Light {
public:

	Light(glm::vec3 pos, glm::vec3 diff, glm::vec3 spec) {
		_pos = pos;
		_diff = diff;
		_spec = spec;
	}
	~Light() {};

	glm::vec3 _pos;
	glm::vec3 _diff;
	glm::vec3 _spec;
};


/* Object base class */
class Object {
public:
	glm::vec3 _diff;
	glm::vec3 _spec;
	double _s; //specular coefficient
	glm::vec3 _pos; // abstract pos
	std::string _type; // string type
	int _transparent_flag = 0; // transparency flag
	int _reflect_flag = 0; // reflection flag
	double _refr_n = -99.f; // n1 from snells equation

	virtual double _hit(Ray r) {
		return -1.f;
	};
	
	virtual glm::vec3 _get_normal(glm::vec3 p) {
		return glm::vec3(-1.f);
	}

	virtual void _set_reflection_flag() {
		if (_spec.x > 1e-2 || _spec.y > 1e-2 || _spec.z > 1e-2) {
			_reflect_flag = 1;
		}
	}

	// sphere manipulation funcs
	virtual void _setCenter(glm::vec3 c) { return; };
	virtual glm::vec3 _get_center() { return glm::vec3(-99999.f, -99999.f, -99999.f); };
};

/* Sphere object */
class Sphere : public Object {
public:
	
	Sphere(double r, glm::vec3 c, glm::vec3 diff, glm::vec3 spec, double s) {
		_radius = r;
		_center = c;
		_diff = diff;
		_spec = spec;
		_s = s;
		_type = "SPHERE";
		_set_reflection_flag();
	};

	glm::vec3 _get_center() {
		return _center;
	}

	void _setCenter(glm::vec3 c) {
		_center = c;
	}

	double _hit (Ray r) {
		glm::vec3 v = _center - r._org;                       // line segment: camera <-------> sphere center
		//double tc = glm::dot(v, r->_dir);                       // proj v onto ray
		double tc = glm::dot(v,r._dir);// *r->_dir.x + v.y * r->_dir.y + v.z * r->_dir.z;
		if (tc < 1e-6) return -1.f;
		double d = glm::dot(v, v) - (tc * tc);       // distance tc to sphere center
		if (d > _radius*_radius) {
			return -1.f;
		}
		d = glm::sqrt(d);
		double ts = glm::sqrt(_radius * _radius - d * d);       // length of segment: intersect 1 <-------> intersect 2
		double t0 = tc - ts;									   // intersect 1
		double t1 = tc + ts;                                    // intersect 2

		// check discriminant
		if (t0 > t1) {
			std::swap(t0, t1);
		}

		if (t0 < 0) {
			t0 = t1;
			if (t0 < 0) {
				return -1.f;
			}
		}
		return t0;
	}

	glm::vec3 _get_normal(glm::vec3 p) {
		return glm::normalize(p - _center);
	}

	// public attr
	double _radius;
	glm::vec3 _center;
};

class Plane : public Object{
public:
	glm::vec3 _normal;
	glm::vec3 _a;
	Plane(glm::vec3 n, glm::vec3 a, glm::vec3 diff, glm::vec3 spec, double s) {
		_normal = n;
		_a = a;
		_diff = diff;
		_spec = spec;
		_s = s;
		_type = "PLANE";
		_set_reflection_flag();
	};

	double _hit(Ray r) {
		double den = glm::dot(_normal, r._dir);
		if (fabs(den) > 1e-4) {
			glm::vec3 q = _a - r._org;
			double t = glm::dot(q, _normal) / den;
			if (t >= 1e-4) return t;
		}
		
		return -1.f;
	}

	glm::vec3 _get_normal(glm::vec3 p) {
		return glm::normalize(_a+_normal);
	}
};


class Triangle : public Object {
public:
	
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 diff, glm::vec3 spec, double s) {
		_a = a;
		_b = b;
		_c = c;
		_diff = diff;
		_spec = spec;
		_s = s;
		_type = "TRIANGLE";
		_set_reflection_flag();
	}

	glm::vec3 _get_normal(glm::vec3 p) {
		glm::vec3 n = glm::normalize(glm::cross(_b - _a, _c - _a));
		return n;
	}
	
	double _hit(Ray r) {
		/*
		0 – parallel
		1 – intersection
		t,u,v – values
		*/
		glm::vec3 p = _b - _a;
		glm::vec3 q = _c - _a;
		glm::vec3 tmp1 = glm::cross(r._dir, q);
		double dot1 = glm::dot(tmp1,p);
		if (dot1 > -1e-5 && dot1 < 1e-5) return -1.f;
		double f = 1 / dot1;
		glm::vec3 s = r._org - _a;
		double u = f * glm::dot(s, tmp1);
		if (u < 0.f || u > 1.f) return -1.f; // outside
		glm::vec3 tmp2 = glm::cross(s,p);
		double v = f * glm::dot(r._dir, tmp2);
		if (v < 0.f || (u + v) > 1.f) return -1.f;
		double t = f * glm::dot(q, tmp2);
		return t;
	}

	glm::vec3 _a;
	glm::vec3 _b;
	glm::vec3 _c;
};


class Quad : public Object {
public:

	glm::vec3 _a;
	glm::vec3 _b;
	glm::vec3 _c;

	Quad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 diff, glm::vec3 spec, double s) {
		_a = a;
		_b = b;
		_c = c;
		_diff = diff;
		_spec = spec;
		_s = s;
		_precomp();
		_set_reflection_flag();
	}

	glm::vec3 _get_normal(glm::vec3 p) {
		return glm::normalize(glm::cross(_b - _a, _c - _a));
	}

	double _hit(Ray r) {
		/*
		0 – parallel
		1 – intersection
		t,u,v – values
		*/
		glm::vec3 tmp1 = glm::cross(r._dir, _q);
		double dot1 = glm::dot(tmp1, _p);
		if (dot1 > -(1e-5) && dot1 < (1e-5)) return -1.f;
		double f = 1 / dot1;
		glm::vec3 s = r._org - _a;
		double u = f * glm::dot(s, tmp1);
		if (u < 0.f || u > 1.f) return -1.f; // outside
		glm::vec3 tmp2 = glm::cross(s, _p);
		double v = f * glm::dot(r._dir, tmp2);
		if (v < 0.f || v > 1.f) return -1.f;
		double t = f * glm::dot(_q, tmp2);
		return t;
	}

private:
	glm::vec3 _p;
	glm::vec3 _q;
	void _precomp() {
		_p = _b - _a;
		_q = _c - _a;
	}
};

class Box : public Object {
public:
	Box(Quad* q1, Quad* q2, Quad* q3, Quad* q4) {
		_quads.push_back(q1);
		_quads.push_back(q2);
		_quads.push_back(q3);
		_quads.push_back(q4);
	}
	std::vector<Quad*> _quads;
};



class Cap : public Object {
public:
	double _radius;
	glm::vec3 _from;
	glm::vec3 _to;

	Cap(double radius, glm::vec3 from, glm::vec3 to, glm::vec3 diff, glm::vec3 spec, double s) {
		_radius = radius;
		_from = from;
		_to = to;
		_diff = diff;
		_spec = spec;
		_s = s;
		_type = "CAPSULE";
		_set_reflection_flag();
		_precomp(); // precomp normal and hit vals
	}

	glm::vec3 _get_normal(glm::vec3 p) {
		// ref: https://www.shadertoy.com/view/Xt3SzX
		//glm::vec3 c = (_ax) * (float)(glm::dot(_ax, p) / _len_ax_squared);

		//// hits cylinder section
		//float a = glm::length(c - _norm_from);
		//float b = glm::length(_norm_to - c);
		//float k = glm::length(_norm_to - _norm_from);
		//float ww = a + b - k;
		//if ( -1e-2 < ww && ww < 1e-2 ) {
		//	return glm::normalize(p - c);
		//}
		//
		//// hit from hemi
		//a = glm::length(c - _from_hemi_tip);
		//b = glm::length(_norm_from - c);
		//k = glm::length(_norm_from - _from_hemi_tip);
		//ww = a + b - k;
		//if (-1e-2 < ww && ww < 1e-2) {
		//	return glm::normalize(p - _norm_from);;
		//}
		//// hit to hemi
		//a = glm::length(_to_hemi_tip - c);
		//b = glm::length(c - _norm_to);
		//k = glm::length(_to_hemi_tip - _norm_to);
		//ww = a + b - k;
		//glm::vec3 ret = glm::normalize(p - _norm_to);
		//return ret;

		glm::vec3  ba = _to - _from;
		glm::vec3  pa = p - _from;
		float h = glm::clamp((float)glm::dot(pa, ba) / (float)glm::dot(ba, ba), (float)0.0, (float)1.0);
		return glm::normalize((pa - h * ba) / (float)_radius);
	}

	double _hit(Ray r) {
		/*
		0 – parallel
		1 – intersection
		t,u,v – values
		ref: https://www.iquilezles.org/www/articles/intersectors/intersectors.htm
		*/
		glm::vec3  oa = r._org - _from;
		float baba = glm::dot(_ax, _ax);
		float bard = glm::dot(_ax, r._dir);
		float baoa = glm::dot(_ax, oa);
		float rdoa = glm::dot(r._dir, oa);
		float oaoa = glm::dot(oa, oa);

		float a = baba - bard * bard;
		float b = baba * rdoa - baoa * bard;
		float c = baba * oaoa - baoa * baoa - _radius * _radius * baba;
		float h = b * b - a * c;
		if (h >= 0.0)
		{
			float t = (-b - sqrt(h)) / a;
			float y = baoa + t * bard;
			// body
			if (y > 0.0 && y < baba) return t;
			// caps
			glm::vec3 oc = (y <= 0.0) ? oa : r._org - _to;
			b = glm::dot(r._dir, oc);
			c = glm::dot(oc, oc) - _radius * _radius;
			h = b * b - c;
			if (h > 0.0) return -b - sqrt(h);
		}
		return -1.0;
	}
private:
	glm::vec3 _ax;
	float _len_ax;
	float _len_ax_squared;
	glm::vec3 _norm_from;
	glm::vec3 _norm_to;
	glm::vec3 _from_hemi_tip;
	glm::vec3 _to_hemi_tip;
	void _precomp() {
		_ax = _to - _from;
		_len_ax = glm::length(_ax);
		_len_ax_squared = glm::pow(glm::length(_ax), 2);
		_norm_from = (_ax) * (float)(glm::dot(_ax, _from) / _len_ax_squared);
		_norm_to = (_ax) * (float)(glm::dot(_ax, _to) / _len_ax_squared);
		_from_hemi_tip = _norm_from - (float)_radius * glm::normalize(_ax);;
		_to_hemi_tip = _norm_to + (float)_radius * glm::normalize(_ax);;
	}

};

std::vector<Sphere*> _genAutomata(int xr, int y, int zr)
{
	std::vector<Sphere*> s;

	for (int i = -xr; i < xr+1; i++) {
		for (int j = -zr; j < zr+1; j++) {
			float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		
			s.push_back(
				new Sphere(
					25.f,//_radius = r;
					glm::vec3(i * 60, y,60*j),//_center = c;
					glm::vec3(r,g,b),//_diff = diff;
					glm::vec3(1,1,1),//_spec = spec;
					50.f//_s = s;
				));
		}
	}
	return s;
}

