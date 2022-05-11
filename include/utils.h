#pragma once
#include <stdio.h>
#include <string>
#include <chrono>

// ansi color codes:https://stackoverflow.com/questions/5762491/how-to-print-color-in-console-using-system-out-println
std::string BLACK       = "";// "\033[0;30m";   // BLACK
std::string RED         = "";// "\033[0;31m";     // RED
std::string GREEN       = "";// "\033[0;32m";   // GREEN
std::string YELLOW      = "";// "\033[0;33m";  // YELLOW
std::string BLUE        = "";// "\033[0;34m";    // BLUE
std::string PURPLE      = "";// "\033[0;35m";  // PURPLE
std::string CYAN        = "";// "\033[0;36m";    // CYAN
std::string WHITE       = "";// "\033[0;37m";   // WHITE
std::string BLACK_BOLD  = "";// "\033[1;30m";  // BLACK
std::string RED_BOLD    = "";// "\033[1;31m";    // RED
std::string GREEN_BOLD  = "";// "\033[1;32m";  // GREEN
std::string YELLOW_BOLD = "";// "\033[1;33m"; // YELLOW
std::string BLUE_BOLD   = "";// "\033[1;34m";   // BLUE
std::string PURPLE_BOLD = "";// "\033[1;35m"; // PURPLE
std::string CYAN_BOLD   = "";// "\033[1;36m";   // CYAN
std::string WHITE_BOLD  = "";// "\033[1;37m";  // WHITE

// profiler Timer class
class Timer {
public:
	std::chrono::time_point<std::chrono::system_clock> _s;
	std::chrono::time_point<std::chrono::system_clock> _e;
	std::chrono::duration<double> _dur;
	Timer() {};

	void _start() {
		_s = std::chrono::system_clock::now();
	};

	void _stop() {
		_e = std::chrono::system_clock::now();
		_dur = _e - _s;
	};

	double _count() {
		return _dur.count();
	};
};



