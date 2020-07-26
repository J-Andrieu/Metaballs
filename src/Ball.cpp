#include "Ball.h"

void updateMetaballs_StraightPath(std::vector<Ball>& balls, size_t width,
                                  size_t height) {
    updateMetaballs_StraightPath(balls.size(), balls.data(), width, height);
}

void updateMetaballs_StraightPath(size_t numBalls, Ball* balls, size_t width,
                                  size_t height) {
    for (int i = 0; i < numBalls; i++) {
        balls[i].position.x += balls[i].velocity.x;
        balls[i].position.y += balls[i].velocity.y;

        if (balls[i].position.x + balls[i].size >= width) {
            balls[i].position.x = width - (balls[i].size + 1);
            if (balls[i].velocity.x > 0.0f) {
                balls[i].velocity.x *= -1;
            }
        } else if (balls[i].position.x - balls[i].size <= 0) {
            balls[i].position.x = balls[i].size + 1;
            if (balls[i].velocity.x < 0.0f) {
                balls[i].velocity.x *= -1;
            }
        }
        if (balls[i].position.y + balls[i].size >= height) {
            balls[i].position.y = height - (balls[i].size + 1);
            if (balls[i].velocity.y > 0.0f) {
                balls[i].velocity.y *= -1;
            }
        } else if (balls[i].position.y - balls[i].size <= 0) {
            balls[i].position.y = balls[i].size + 1;
            if (balls[i].velocity.y < 0.0f) {
                balls[i].velocity.y *= -1;
            }
        }
    }
}

void updateMetaballs_RandomPath(std::vector<Ball>& balls, float theta, size_t width,
                                  size_t height) {
    updateMetaballs_RandomPath(balls.size(), balls.data(), theta, width, height);
}

void updateMetaballs_RandomPath(size_t numBalls, Ball* balls, float theta,
                                size_t width, size_t height) {
    for (int i = 0; i < numBalls; i++) {
        // get normalized vector of velocity
        struct {
            float x;
            float y;
        } normalVel = {balls[i].velocity.x, balls[i].velocity.y};
        float mag = std::pow(normalVel.x, 2) + std::pow(normalVel.y, 2);
        mag = std::sqrt(mag);
        normalVel = {normalVel.x / mag, normalVel.y / mag};
        float currAngle = std::atan(normalVel.y / normalVel.x);

        // get degree change to new velocity
        float degChange = (float)std::rand() / RAND_MAX;
        degChange *= theta;
        degChange -= theta / 2;

        // set new velocity
        currAngle += degChange;
        normalVel = {std::sin(currAngle), std::cos(currAngle)};

        // proceed as normal
        balls[i].velocity = {normalVel.x * mag, normalVel.y * mag};
        balls[i].position.x += balls[i].velocity.x;
        balls[i].position.y += balls[i].velocity.y;

        if (balls[i].position.x + balls[i].size >= width) {
            balls[i].position.x = width - (balls[i].size + 1);
            if (balls[i].velocity.x > 0.0f) {
                balls[i].velocity.x *= -1;
            }
        } else if (balls[i].position.x - balls[i].size <= 0) {
            balls[i].position.x = balls[i].size + 1;
            if (balls[i].velocity.x < 0.0f) {
                balls[i].velocity.x *= -1;
            }
        }
        if (balls[i].position.y + balls[i].size >= height) {
            balls[i].position.y = height - (balls[i].size + 1);
            if (balls[i].velocity.y > 0.0f) {
                balls[i].velocity.y *= -1;
            }
        } else if (balls[i].position.y - balls[i].size <= 0) {
            balls[i].position.y = balls[i].size + 1;
            if (balls[i].velocity.y < 0.0f) {
                balls[i].velocity.y *= -1;
            }
        }
    }
}