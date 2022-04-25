#pragma once
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>
#include <iterator>
#include <utility>
#include "defs.h"

using std::vector;

class manual_timer
{
	std::chrono::high_resolution_clock::time_point t0;
	double timestamp{ 0.f };
public:
	void start();
	void stop();
	const double & get();
};

bool read_ply_file(const std::string & filepath, vector<float3> &_vertices,
	vector<std::pair<uint, uint>> &_edges, vector<vector<uint>>& _polygons);