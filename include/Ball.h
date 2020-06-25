#ifndef BALL_H
#define BALL_H

#include <cstdlib>
#include <vector>
#include <cmath>

typedef struct {
    float size;
    struct {
        float x;
        float y;
    } position;
    struct {
        float x;
        float y;
    } velocity;
    struct {
        size_t r;
        size_t g;
        size_t b;
    } color;
} Ball;

void updateMetaballs_StraightPath(std::vector<Ball> balls, size_t width, size_t height);
void updateMetaballs_RandomPath(std::vector<Ball> balls, float theta, size_t width, size_t height);

#endif /* BALL_H */