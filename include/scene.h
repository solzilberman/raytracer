#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "time.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <json/json.cpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STBI/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION    
#include "STBI/stbi_image.h"
#include "objects.h"
#include "utils.h"


/* Skybox data store class */
class SkyBox {
public:
	int _nr = 0;
	int _width = 0;
	int _height = 0;
	std::vector<unsigned char*> _skybox_faces;
	SkyBox(std::vector<std::string> faces) {
		unsigned char* data;
		for (int i = 0; i < 6; i++) {
			data = stbi_load(faces[i].c_str(), &_width, &_height, &_nr, 0);
			_skybox_faces.push_back(data);
		}
	}

	glm::vec3 _get_color_at(Ray ray) {
		glm::vec3 sky_point = glm::normalize(ray._get_point(1000));
		int face_ind;
		float x, y;
		double max_val = std::max((double)std::max((double)abs(sky_point.x), (double)abs(sky_point.y)), (double)abs(sky_point.z));
		//sky_point /= max_val;
		if (abs(max_val - abs(sky_point.x)) < 1e-4) { // left or right
			int chk = (int)round(sky_point.x / max_val);
			face_ind = chk > 0 ? 0 : 1;
			x = sky_point.z * chk;
			y = sky_point.y * chk;
		}
		else if (abs(max_val - abs(sky_point.y)) < 1e-4) { // left or right
			int chk = (int)round(sky_point.y / max_val);
			face_ind = chk > 0 ? 2 : 3;
			x = sky_point.x * chk;
			y = sky_point.z * chk;
		}
		else if (abs(max_val - abs(sky_point.z)) < 1e-4) { // left or right
			int chk = (int)round(sky_point.z / max_val);
			face_ind = chk > 0 ? 4 : 5;
			x = sky_point.x * chk;
			y = sky_point.y * chk;
		}
		unsigned char* data = _skybox_faces[face_ind];
		int realx = (x + 1) * 0.5f * _width;
		int realy = (y + 1) * 0.5f *_height;
		unsigned char* pixelOffset = data + (realx + _width * realy) * _nr;
		glm::vec3 skb_rgb = glm::vec3((float)pixelOffset[0] / 255, (float)pixelOffset[1] / 255, (float)pixelOffset[2] / 255);
		return skb_rgb;
	}
};

/* 
Framebuffer class.
- Stores pixel buffer data and writes png to file.
*/
class FrameBuffer {
public:

	FrameBuffer(int width, int height) {
		_width = width;
		_height = height;
		_buffer = std::vector<double>(4 * _width * _height, 0);
		_data = (unsigned char*)malloc(_buffer.size() * sizeof(unsigned char));
	}

	~FrameBuffer() {
		_buffer.clear();
		free(_data);
	}

	void _setPixel(int i, int j, glm::vec3 color) {

		_buffer[j * (4 * _width) + (i*4) + 0] = color.x;
		_buffer[j * (4 * _width) + (i*4) + 1] = color.y;
		_buffer[j * (4 * _width) + (i*4) + 2] = color.z;
		_buffer[j * (4 * _width) + (i*4) + 3] = 1;
	}

	void _writePNG() {
		//ref: https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
		//stbi_flip_vertically_on_write(1);
		for (int i = 0; i < _buffer.size(); i++) {
			_data[i] = (unsigned char)int(_buffer[i]*255);
		}
		
		/*char buff[1000];
		tmpnam_s(buff, 1000);*/
		std::string fc = std::to_string(_frameCount);
		std::string uuid;
		if (!_animation_mode) { // gen uuid for still renders
			srand(time(NULL));
			for (int i = 0; i < 10; i++) {
				uuid += std::to_string(rand() % 10);
			}
		}
		std::string suff = _animation_mode ? "" : uuid;
		suff = suff + ".png";
		std::string path = _animation_mode ? "renders/frames/render" : "renders/stills/render";
		std::string fn = path + fc + suff;
		_frameCount += 1;
		//stbi_flip_vertically_on_write(1);
		int saved = stbi_write_png(fn.data(), _width, _height, 4, _data, 0);
	}

	int _width;
	int _height;
	std::vector<double> _buffer;
	int _frameCount = 0;
	int _animation_mode = 0;
	unsigned char* _data;
};

/* Abstract camera class */
class Camera {
public:

	Camera(double fovy, glm::vec3 eye, glm::vec3 target, glm::vec3 up) {
		_fovy = fovy;
		_eye = eye;
		_target = target;
		_up = up;
	}

	// translate camera->_eye by tr
	void _translate(glm::vec3 tr) {
		_eye += tr;
		_target += tr;
	}

	double _fovy;
	glm::vec3 _eye;
	glm::vec3 _target;
	glm::vec3 _up;
};

/* Parses config for scene and mjcf files */
class ConfigParser {
public:
	json::jobject _result;
	
	ConfigParser(std::string f, std::string model)
	{
		try {
			// Parse the input
			// ref: https://www.oldgreg.net/simpleson/1.0.0/index.html
			std::ifstream js(f);
			std::string js_object((std::istreambuf_iterator<char>(js)), std::istreambuf_iterator<char>());
			_result = json::jobject::parse(js_object.c_str());
		}
		catch (const std::exception& e) { 
			std::cout << RED_BOLD + "[error] " + WHITE +  e.what() << std::endl; 
			exit(1);
		};
		if (model.length() > 0) _load_MJCF(model);
	}

	~ConfigParser() {};

	/* load mjcf model and return object array */
	std::vector<Object*> _load_MJCF(std::string fn) {
		std::ifstream file(fn);
		if (!file.is_open()) {
			std::cout << "[error] Could not open file: " << fn << std::endl;
			exit(1);
		}
		// load lines into buffer lines
		std::string line;
		std::getline(file, line);
		std::vector<float> data;
		std::string tok;
		std::stringstream str(line);
		while (str >> tok) data.push_back(std::stof(tok));
		str.clear();
		std::vector<Object*> ret;
		for (int i = 0; i < data.size() / 19; i++) {
			float r = data[(i * 19) + 0] * 10.f;
			float size = data[(i * 19) + 1] * 10.f;
			glm::vec3 from = glm::vec3(0.f, 0.f, 0.f);
			glm::vec3 to = glm::vec3(0.f, 0.f, size);
			glm::mat3 rot = glm::mat3(data[(i * 19) + 6], data[(i * 19) + 7], data[(i * 19) + 8],
				data[(i * 19) + 9], data[(i * 19) + 10], data[(i * 19) + 11],
				data[(i * 19) + 12], data[(i * 19) + 13], data[(i * 19) + 14]);


			from = rot * from;
			to = rot * to;

			glm::vec3 tr = glm::vec3(data[(i * 19) + 3], data[(i * 19) + 4], -data[(i * 19) + 5]) * 10.f;
			//glm::vec3 rgb = glm::vec3(data[(i * 19) + 15], data[(i * 19) + 16], data[(i * 19) + 17]);
			float red = 92.f / 255.f;
			float g = 62.f / 255.f;
			float b = 62.f / 255.f;
			from += tr;
			to += tr;
			ret.push_back(new Cap(
				r,//_radius = r;
				from,//from
				to,//to 
				glm::vec3(red, g, b),//_diff = diff;
				glm::vec3(1, 1, 1),//_spec = spec;
				500.f//_s = s;
			));
		}
		return ret;
	}

	/* get background color param from json file */
	glm::vec3 _parse_bg_config_JSON() {
		json::jobject render = _result["renderer"];
		if (!render.has_key("anti_aliasing"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Renderer missing 'anti_aliasing' key-value!\n";
			exit(1);
		}
		return glm::vec3(render["background_color"].array(0), render["background_color"].array(1), render["background_color"].array(2));
	}

	/* get bounce depth param from json file */
	int _parse_md_config_JSON() {
		json::jobject render = _result["renderer"];
		if (!render.has_key("anti_aliasing"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Renderer missing 'anti_aliasing' key-value!\n";
			exit(1);
		}
		return render["max_depth"];
	}

	/* get antialiasing param from json file */
	int _parse_rss_config_JSON() {
		json::jobject render = _result["renderer"];
		if (!render.has_key("anti_aliasing"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Renderer missing 'anti_aliasing' key-value!\n";
			exit(1);
		}
		return render["anti_aliasing"];
	}

	/* get resuloution, bounce depth, anti alias params*/
	FrameBuffer* _parse_screen_config_JSON() {
		if (!_result.has_key("renderer"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Missing 'renderer' object!\n";
			exit(1);
		}
		json::jobject render = _result["renderer"];
		std::vector<int> res = render["resolution"];
		return new FrameBuffer(res[0], res[1]);
	}

	/* parse and init camera class from json file*/
	Camera* _parse_camera_config_JSON() {
		if (!_result.has_key("camera"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Missing 'camera' object!\n";
			exit(1);
		}
		json::jobject cam = _result["camera"];
		float fovy = cam["fovy"];
		glm::vec3 eye = glm::vec3(cam["eye"].array(0), cam["eye"].array(1), cam["eye"].array(2));
		glm::vec3 target = glm::vec3(cam["target"].array(0), cam["target"].array(1), cam["target"].array(2));
		glm::vec3 up = glm::vec3(cam["up"].array(0), cam["up"].array(1), cam["up"].array(2));
		return new Camera(fovy, eye, target, up);
	}

	/* parse and init skybox class from json file*/
	SkyBox* _parse_skybox_config_JSON() {
		if (!_result.has_key("skybox")) return NULL;
		json::jobject skybox = _result["skybox"];
		std::string right = skybox["right"];
		std::string left = skybox["left"];
		std::string top = skybox["top"];
		std::string bottom = skybox["bottom"];
		std::string front = skybox["front"];
		std::string back = skybox["back"];
		std::vector<std::string> faces = { right,left,top,bottom,front,back };
		return new SkyBox(faces);
	}

	/* parse and init lights from array in json file*/
	std::vector<Light*> _parse_lights_config_JSON() {
		if (!_result.has_key("lights"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Missing 'lights' object!\n";
			exit(1);
		}
		std::vector<json::jobject> lights = _result["lights"];
		std::vector<Light*> ret;
		for (int i = 0; i < lights.size(); i++) {
			json::jobject curr_js_light = lights[i];
			glm::vec3 pos = glm::vec3((double)curr_js_light["pos"].array(0), (double)curr_js_light["pos"].array(1), (double)curr_js_light["pos"].array(2));
			glm::vec3 diff = glm::vec3((double)curr_js_light["diff"].array(0), (double)curr_js_light["diff"].array(1), (double)curr_js_light["diff"].array(2));
			glm::vec3 spec = glm::vec3((double)curr_js_light["spec"].array(0), (double)curr_js_light["spec"].array(1), (double)curr_js_light["spec"].array(2));
			ret.push_back(new Light(pos, diff, spec));
		}
		return ret;
	}

	/* parse and init objects from array in json file*/
	std::vector<Object*> _parse_objects_config_JSON() {
		if (!_result.has_key("objects"))
		{
			std::cout << RED + "[error] " + WHITE + "JSON syntax error in config file. Missing 'objects' object!\n";
			exit(1);
		}
		std::vector<json::jobject> objects = _result["objects"];
		std::vector<Object*> ret;
		for (int i = 0; i < objects.size(); i++) {
			json::jobject curr_js_obj = objects[i];
			std::string type = curr_js_obj["type"];
			if (type == "plane") {
				glm::vec3 pos = glm::vec3((double)curr_js_obj["pos"].array(0), (double)curr_js_obj["pos"].array(1), (double)curr_js_obj["pos"].array(2));
				glm::vec3 norm = glm::vec3((double)curr_js_obj["normal"].array(0), (double)curr_js_obj["normal"].array(1), (double)curr_js_obj["normal"].array(2));
				glm::vec3 diff = glm::vec3((double)curr_js_obj["diff"].array(0), (double)curr_js_obj["diff"].array(1), (double)curr_js_obj["diff"].array(2));
				glm::vec3 spec = glm::vec3((double)curr_js_obj["spec"].array(0), (double)curr_js_obj["spec"].array(1), (double)curr_js_obj["spec"].array(2));
				float s = (double)curr_js_obj["shineness"];
				ret.push_back(new Plane(pos, norm, diff, spec, s));
			}
			else if (type == "sphere") {
				double radius = curr_js_obj["radius"];
				glm::vec3 pos = glm::vec3((double)curr_js_obj["pos"].array(0), (double)curr_js_obj["pos"].array(1), (double)curr_js_obj["pos"].array(2));
				glm::vec3 diff = glm::vec3((double)curr_js_obj["diff"].array(0), (double)curr_js_obj["diff"].array(1), (double)curr_js_obj["diff"].array(2));
				glm::vec3 spec = glm::vec3((double)curr_js_obj["spec"].array(0), (double)curr_js_obj["spec"].array(1), (double)curr_js_obj["spec"].array(2));
				double s = curr_js_obj["shineness"];
				ret.push_back(new Sphere(radius, pos, diff, spec, s));
			}
			else if (type == "quad") {
				glm::vec3 p0 = glm::vec3((double)curr_js_obj["p0"].array(0), (double)curr_js_obj["p0"].array(1), (double)curr_js_obj["p0"].array(2));
				glm::vec3 p1 = glm::vec3((double)curr_js_obj["p1"].array(0), (double)curr_js_obj["p1"].array(1), (double)curr_js_obj["p1"].array(2));
				glm::vec3 p2 = glm::vec3((double)curr_js_obj["p2"].array(0), (double)curr_js_obj["p2"].array(1), (double)curr_js_obj["p2"].array(2));
				glm::vec3 diff = glm::vec3((double)curr_js_obj["diff"].array(0), (double)curr_js_obj["diff"].array(1), (double)curr_js_obj["diff"].array(2));
				glm::vec3 spec = glm::vec3((double)curr_js_obj["spec"].array(0), (double)curr_js_obj["spec"].array(1), (double)curr_js_obj["spec"].array(2));
				float s = (double)curr_js_obj["shineness"];
				ret.push_back(new Quad(p0, p1, p2, diff, spec, s));
			}
		}
		return ret;
	}
};

/* 
Abstract scene class.
- Stores object and lighting data
- Stores configuration data
*/
class Scene {
public:
	Scene() {}; // default constructor
	Scene(std::string fn, std::string model = "") {
		_input_file = fn;
		 _config_parser = new ConfigParser(fn, model);
		_screen = _config_parser->_parse_screen_config_JSON();
		_camera = _config_parser->_parse_camera_config_JSON();
		_bg_color = _config_parser->_parse_bg_config_JSON();
		_objects = _config_parser->_parse_objects_config_JSON();
		_lights = _config_parser->_parse_lights_config_JSON();
		_max_depth = _config_parser->_parse_md_config_JSON();
		_rss = _config_parser->_parse_rss_config_JSON();
		_skybox = _config_parser->_parse_skybox_config_JSON();
	}
	~Scene() {
		for (auto l : _lights) {
			delete l; // free allocated light pointers
		}
		for (auto o : _objects) {
			delete o; // free allocated object pointers
		}
		_lights.clear();
		_objects.clear();
		delete _config_parser; // call parser destructor
	}

	void _setBackground(glm::vec3 bg) {
		_bg_color = bg;
	}

	void _addLight(Light* l) { 
		_lights.push_back(l); 
	}
	void _addObject(Object* o) { 
		_objects.push_back(o); 
	}

	// scene objects
	FrameBuffer* _screen = NULL;
	Camera* _camera = NULL;
	ConfigParser* _config_parser = NULL;
	SkyBox* _skybox = NULL;
	std::vector<Light*> _lights = {};
	std::vector<Object*> _objects = {};
	// scene params
	glm::vec3 _bg_color = glm::vec3(0.f);
	std::string _input_file = "";
	int _max_depth = 1;
	int _rss = 1;

};