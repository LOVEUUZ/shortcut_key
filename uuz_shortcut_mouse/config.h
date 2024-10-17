#pragma once

#include <string>
#include "json.hpp"

// using namespace std;
using json = nlohmann::json;

class Coordinate {
  public:
    int x;
    int y;

    Coordinate() = default;
    Coordinate(int x, int y) : x(x), y(y) {}

    json              toJson() const;
    static Coordinate fromJson(const json & j);
};

class Config {
  public:
    int         id;           //索引
    std::string fileName;     //文件绝对路径
    std::string showName;     //显示名称
    std::string absolutePath; //重复的绝对路径
    std::string creationTime; //创建时间
    std::string lastMoveTime; //修改时间
    Coordinate  coordinate;   //坐标
    // int count;

    json          toJson() const;
    static Config fromJson(const json & j);
};
