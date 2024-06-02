#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "GameController.h"
#include "freeglut_std.h"
#include <array>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

GameWorld* createStudentWorld(std::string assetDir) {
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir) : GameWorld(assetDir) {
        srand(0);
}

int StudentWorld::init() {

        for (int y = 0; y < VIEW_HEIGHT - 4; y++) {
                for (int x = 0; x < VIEW_WIDTH; x++) {
                        if (x < 30 || x > 33 || y < 4) {
                                m_ice[y][x] = new Ice(this, x, y);
                        }
                        else {
                                m_ice[y][x] = nullptr;
                        }
                }
        }

        m_iceman = new Iceman(this);

        // set up everything that is not the ice and iceman

        int numBoulders = std::min(2 + static_cast<int>(getLevel() / 2), 9);
        int numGold = std::max(5 - static_cast<int>(getLevel() / 2), 2);
        int numBarrels = std::min(2 + static_cast<int>(getLevel()), 9);

        std::cout << "Level: " << getLevel() << std::endl;
        std::cout << "numBoulders: " << numBoulders << std::endl;
        std::cout << "numGold: " << numGold << std::endl;
        std::cout << "numBarrels: " << numBarrels << std::endl;

        for (int i = 0; i < numBoulders; i++) {
                int x;
                int y;

                bool foundCoords = false;

                while (!foundCoords) {
                        x = (std::rand() % VIEW_WIDTH) + 1;
                        y = (std::rand() % (VIEW_HEIGHT - 4)) + 1;

                        foundCoords = true;

                        for (GameObject* object : getGameObjects()) {
                                if (std::sqrt(std::pow(x - object->getX(), 2) - std::pow(y - object->getY(), 2)) <= 6) {
                                        foundCoords = false;
                                        break;
                                }
                        }
                }

                m_gameObjects.push_back(new Boulder(this, x, y));
                removeIceAt(x, y);
        }

        m_gameObjects.push_back(new Boulder(this, 30, 30));
        m_gameObjects.push_back(new Protester(this, 60, 60, IID_PROTESTER));

        return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::isDistributedObjectPositionOk(int x, int y) {
        for (GameObject* object : getGameObjects()) {
                if (std::sqrt(std::pow(x - object->getX(), 2) - std::pow(y - object->getY(), 2)) <= 6) {
                        return false;
                }
        }

        return true;
}

int StudentWorld::move() {
        std::string statusBar = "Lvl:\t0\tLives:\t2\tHlth:\t100%\tWtr:\t" + std::to_string(m_iceman->getWater()) + "\tGld:\t1\tOil Left:\t2\tSonar:\t1\tScr:\t000000";

        setGameStatText(statusBar);

        m_iceman->doSomething();

        for (GameObject* object : m_gameObjects) {
                object->doSomething();
        }

        return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::removeIceAt(int x, int y) {
        bool removedAnything = false;

        for (int iy = y; iy < y + 4; iy++) {
                for (int ix = x; ix < x + 4; ix++) {
                        if (m_ice[iy][ix] != nullptr && iy >= 0 && iy < VIEW_HEIGHT && ix >= 0 && ix < VIEW_WIDTH) {
                                delete m_ice[iy][ix];
                                m_ice[iy][ix] = nullptr;
                                removedAnything = true;
                        }
                }
        }

        return removedAnything;
}

void StudentWorld::addGameObject(GameObject* gameObject) {
        m_gameObjects.push_back(gameObject);
}

bool StudentWorld::isDirObstructed(int x, int y, GameObject::Direction dir) {
        switch (dir) {
        case GameObject::up:
                if (y >= VIEW_HEIGHT - SPRITE_WIDTH) {
                        return true;
                }

                for (int ox = x; ox < x + SPRITE_WIDTH; ox++) {
                        if (m_ice[y + SPRITE_WIDTH][ox] != nullptr) {
                                return true;
                        }
                }

                return false; // I need to return false under each case because I at least need break and this is clearer to readers.
        case GameObject::down:
                if (y <= 0) {
                        return true;
                }

                for (int ox = x; ox < x + SPRITE_WIDTH; ox++) {
                        if (m_ice[y - 1][ox] != nullptr) {
                                return true;
                        }
                }

                return false;
        case GameObject::left:
                if (x <= 0) {
                        return true;
                }

                for (int oy = y; oy < y + SPRITE_WIDTH; oy++) {
                        if (m_ice[oy][x - 1] != nullptr) {
                                return true;
                        }
                }

                return false;
        case GameObject::right:
                if (x >= VIEW_WIDTH - SPRITE_WIDTH) {
                        return true;
                }

                for (int oy = y; oy < y + SPRITE_WIDTH; oy++) {
                        if (m_ice[oy][x + SPRITE_WIDTH] != nullptr) {
                                return true;
                        }
                }

                return false;
        default:
                return true;
        }
}

void StudentWorld::cleanUp() {
        for (GameObject* gameObject : m_gameObjects) {
                delete gameObject;
        }

        for (int y = 0; y < VIEW_HEIGHT; y++) {
                for (int x = 0; x < VIEW_WIDTH; x++) {
                        delete m_ice[y][x];
                }
        }


        delete m_iceman;
}

StudentWorld::~StudentWorld() {
        cleanUp();
}

