#include <iostream>
#include <vector>
#include "SFML/Graphics.hpp"
#include "vector2.cpp"

const int HEIGHT = 1366;
const int WIDTH = 697;
const int FPS = 60;
const int SUBSTEPS = 8;

class Object {
public:
    Vec2 position{0,0};
    Vec2 oldPosition{ 0, 0 };
    Vec2 acceleration{ 0,0 };
    float radius;
    sf::Color color;

    Object(Vec2 position, float radius, sf::Color color=sf::Color::White) 
        : position(position), oldPosition(position), radius(radius), color(color)
    {}

    void updatePosition(float dt) {
        Vec2 velocity = position - oldPosition;
        oldPosition = position;
        position = position + velocity + acceleration * dt * dt;
        acceleration = Vec2(0, 0);
    }

    void accelerate(Vec2 force) {
        acceleration += force;
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape circle(radius, 25);
        circle.setPosition(position.x-radius, position.y-radius);
        circle.setFillColor(color);
        window.draw(circle);
    }
};

class Solver {
public:
    std::vector<Object> objects;
    Vec2 gravity{ 0, 1000 };
    int inputBuffer = 5;
    int inputBufferCounter = 0;

    void update(float dt, sf::RenderWindow& window) {
        if (objects.size()) {
            float subdt = dt / SUBSTEPS;
            for (int i = 0; i < SUBSTEPS; i++) {
                applyGravity();
                applyConstraint();
                checkCollision();
                updatePositions(subdt);
            }
        }
        draw(window);
    }

    void addObject(Vec2 position, float radius, sf::Color color=sf::Color::White) {
        objects.push_back(Object(position, radius, color));
    }

    void input(sf::RenderWindow& window, sf::Event& event) {
        bool left = false, right = false;
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) left = true;
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) right = true;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        Vec2 mouse(mousePos.x, mousePos.y);
        if ((left || right) && inputBufferCounter >= inputBuffer) {
            if (left) addObject(mouse, 25);
            else if (right) {
                for (int i = 0; i < objects.size(); i++) {
                    Vec2 toMouse = mouse - objects[i].position;
                    float dist = toMouse.magnitude();
                    if (dist <= objects[i].radius) objects.erase(objects.begin() + i);
                }
            }
            inputBufferCounter = 0;
        }
        else inputBufferCounter++;
    }

    void updatePositions(float dt) {
        for (auto& obj : objects) {
            obj.updatePosition(dt);
        }
    }

    void applyGravity() {
        for (auto& obj : objects) {
            obj.accelerate(gravity);
        }
    }

    void applyConstraint() {
        const Vec2 center(683, 349);
        const float radius = 300;
        for (auto& obj : objects) {
            Vec2 to_obj = obj.position - center;
            float dist = to_obj.magnitude();
            if (dist > radius - obj.radius) {
                Vec2 normDist = to_obj.normalized();
                obj.position = center + normDist * (radius - obj.radius);
            }
        }
    }

    void checkCollision() {
        for (uint32_t i = 0; i < objects.size()-1; i++) {
            Object& obj1 = objects[i];
            for (uint32_t j = i + 1; j < objects.size(); j++) {
                Object& obj2 = objects[j];
                Vec2 axis = obj1.position - obj2.position;
                float dist = axis.magnitude();
                if (dist < obj1.radius + obj2.radius) {
                    Vec2 normDist = axis.normalized();
                    float delta = obj1.radius + obj2.radius - dist;
                    obj1.position += normDist * delta * 0.5f;
                    obj2.position -= normDist * delta * 0.5f;
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        sf::CircleShape arena(300, 100);
        arena.setPosition(383, 49);
        arena.setFillColor(sf::Color::Black);
        window.draw(arena);

        for (auto& obj : objects) {
            obj.draw(window);
        }
        
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(HEIGHT, WIDTH), "Verlet Solver");
    window.setFramerateLimit(FPS);

    Solver solver;
    solver.addObject(Vec2(883, 200), 25);
    solver.addObject(Vec2(883, 449), 50);
    solver.addObject(Vec2(500, 275), 35);
    int spawnTime = 0;
    bool paused = false;

    sf::Clock clock;
    while (window.isOpen()) {
        window.clear(sf::Color(50, 50, 50));

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                paused = !paused;
            }
            solver.input(window, event);
        }

        spawnTime++;
        if (spawnTime >= 20 && !paused) {
            spawnTime = 0;
            solver.addObject(Vec2(883, 200), 25);
        }

        solver.update(clock.restart().asSeconds(), window);
        window.display();
    }

    return 0;
}