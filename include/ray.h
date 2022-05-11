#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

class Ray {
public:
	glm::vec3 _org;
	glm::vec3 _dir;
	Ray() {};
	Ray(glm::vec3 eye, glm::vec3 dir) {
		_org = eye;
		_dir = glm::normalize(dir);
	};
	~Ray() {};

	/* Get point on ray _eye + _direction * t */
	glm::vec3 _get_point(float t) {
		return _org + (_dir * t);
	}

	/* Debugging logger */
	void _log() {
		std::cout << "Origin: " << glm::to_string(_org) << "\n";
		std::cout << "Direction: " << glm::to_string(_dir) << "\n";
	}
};