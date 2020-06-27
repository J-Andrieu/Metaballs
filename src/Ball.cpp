#include "Ball.h"

void updateMetaballs_StraightPath(std::vector<Ball> balls, size_t width, size_t height) {
    for (auto ball : balls) {
        ball.position.x += ball.velocity.x;
        ball.position.y += ball.velocity.y;

        if (ball.position.x >= width || ball.position.x <= 0) {
            ball.velocity.x *= -1;
        }
        if (ball.position.y >= height || ball.position.y <= 0) {
            ball.velocity.y *= -1;
        }
    }
}

void updateMetaballs_RandomPath(std::vector<Ball> balls, float theta, size_t width, size_t height) {
    for (auto ball : balls) {
        //get normalized vector of velocity
        struct {float x; float y;} normalVel = {ball.velocity.x, ball.velocity.y};
        float mag = std::pow(normalVel.x, 2) + std::pow(normalVel.y, 2);
        mag = std::sqrt(mag);
        normalVel = {normalVel.x / mag, normalVel.y / mag};
        float currAngle = std::atan(normalVel.y / normalVel.x);

        //get degree change to new velocity
        float degChange = (float) std::rand() / RAND_MAX;
        degChange *= theta;
        degChange -= theta / 2;

        //set new velocity
        currAngle += degChange;
        normalVel = {std::sin(currAngle), std::cos(currAngle)};

        //proceed as normal
        ball.velocity = {normalVel.x * mag, normalVel.y * mag};
        ball.position.x += ball.velocity.x;
        ball.position.y += ball.velocity.y;

        if (ball.position.x >= width || ball.position.x <= 0) {
            ball.velocity.x *= -1;
        }
        if (ball.position.y >= height || ball.position.y <= 0) {
            ball.velocity.y *= -1;
        }
    }
}