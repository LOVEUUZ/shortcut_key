#include "config.h"

json Coordinate::toJson() const {
	return json{ {"x", x}, {"y", y} };
}

Coordinate Coordinate::fromJson(const json& j) {
	return Coordinate(j.at("x").get<int>(), j.at("y").get<int>());
}

// 将 Config 转为 JSON
json Config::toJson() const {
	return json{
		{"id", id},
		{"fileName", fileName},
		{"absolutePath", absolutePath},
		{"creationTime", creationTime},
		{"lastMoveTime", lastMoveTime},
		{"coordinate", coordinate.toJson()},
		{"count", count}
	};
}

// 从 JSON 转为 Config
Config Config::fromJson(const json& j) {
	Config config;
	config.id = j.at("id").get<int>();
	config.fileName = j.at("fileName").get<string>();
	config.absolutePath = j.at("absolutePath").get<string>();
	config.creationTime = j.at("creationTime").get<string>();
	config.lastMoveTime = j.at("lastMoveTime").get<string>();
	config.coordinate = Coordinate::fromJson(j.at("coordinate"));
	config.count = j.at("count").get<int>();
	return config;
}