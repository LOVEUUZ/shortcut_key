#pragma once

#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class Coordinate {
public:
	int x;
	int y;

	Coordinate() = default;
	Coordinate(int x, int y) :x(x), y(y) {}
	
	json toJson() const;
	static Coordinate fromJson(const json& j);
};

class Config {
public:
	int id;
	string fileName;
	string absolutePath;
	string creationTime;
	string lastMoveTime;
	Coordinate coordinate;
	int count;

	json toJson() const;
	static Config fromJson(const json& j);
};
