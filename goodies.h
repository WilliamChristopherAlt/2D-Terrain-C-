#ifndef GOODIES_H
#define GOODIES_H

#include <cmath>
#include <chrono>
using namespace std::chrono;

#include <ctime>

#include "vec3.h"

uint32_t timeSinceEpochMillisec() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

double random_double()
{
    static uint32_t state = timeSinceEpochMillisec();
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state / 4294967296.0;
}

double random_double1()
{
    static uint32_t state = 2;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state / 4294967296.0;
}

double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

int random_int(int min, int max)
{
    return floor(random_double(min, max));
}

double ease_out(double x)
{
    return std::max(0.0, std::min(1.0, 1.0 - pow(1.0 - x, 6.0)));
}

double lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

double fade(double t)
{
    return t * t * t * (t * (t*6.0 - 15.0) + 10.0);
}

struct Environment {
    double** height_map;
    int width;
    int height;
    vec3 sun_pos;
    double water_level;
    double shadow_brightness;
    double min_step;
};

#endif