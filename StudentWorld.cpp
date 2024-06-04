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

int StudentWorld::getNumItemsInLevel(int itemID) {
        switch (itemID) {
        case IID_BOULDER:
                return std::min(2 + static_cast<int>(getLevel() / 2), 9);
        case IID_GOLD:
                return std::max(5 - static_cast<int>(getLevel() / 2), 2);
        case IID_BARREL:
                return std::min(2 + static_cast<int>(getLevel()), 9);
        default:
                return 0;
        }
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

        for (int i = 0; i < getNumItemsInLevel(IID_BOULDER); i++) {
                int x;
                int y;

                bool foundCoords = false;

                while (!foundCoords) {
                        x = (std::rand() % (VIEW_WIDTH - SPRITE_WIDTH)) + 1;
                        y = (std::rand() % (VIEW_HEIGHT - (SPRITE_WIDTH * 2))) + 1;

                        foundCoords = true;

                        for (Actor* object : getActors()) {
                                if (distance(x, y, object->getX(), object->getY()) <= 6) {
                                        foundCoords = false;
                                        break;
                                }
                        }
                }

                m_gameObjects.push_back(new Boulder(this, x, y));
                removeIceAt(x, y);
        }

        for (int i = 0; i < getNumItemsInLevel(IID_BARREL); i++) {
                int x;
                int y;

                bool foundCoords = false;

                while (!foundCoords) {
                        x = (std::rand() % (VIEW_WIDTH - SPRITE_WIDTH)) + 1;
                        y = (std::rand() % (VIEW_HEIGHT - (SPRITE_WIDTH * 2))) + 1;

                        foundCoords = true;

                        for (Actor* object : getActors()) {
                                if (distance(x, y, object->getX(), object->getY()) <= 6) {
                                        foundCoords = false;
                                        break;
                                }
                        }
                }

                m_gameObjects.push_back(new Barrel(this, x, y));
        }

        addActor(new Protester(this, 60, 60, IID_PROTESTER));
        addActor(new Gold(this, 20, 0, false, -1, true, false));

        return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::isDistributedObjectPositionOk(int x, int y) const {
        for (Actor* object : getActors()) {
                if (std::sqrt(std::pow(x - object->getX(), 2) - std::pow(y - object->getY(), 2)) <= 6) {
                        return false;
                }
        }

        return true;
}

int StudentWorld::move() {
        // format this nicely later
        std::string statusBar = "Lvl:  " + std::to_string(getLevel()) + "  " +
                                "Lives: " + std::to_string(m_iceman->getLives()) + "  " + 
                                "Hlth: " + std::to_string((m_iceman->getHealth() / 10) * 100) + "%  " +
                                "Wtr:  " + std::to_string(m_iceman->getItemCount(IID_WATER_POOL)) + "  " +
                                "Gld:  " + std::to_string(m_iceman->getItemCount(IID_GOLD)) + "  " +
                                "Oil Left:  " + std::to_string(getNumItemsInLevel(IID_BARREL) - m_iceman->getItemCount(IID_BARREL)) + "  " + 
                                "Sonar:  " + std::to_string(m_iceman->getItemCount(IID_SONAR)) + "  " +
                                "Scr: " + std::to_string(getScore());

        setGameStatText(statusBar);

        m_iceman->doSomething();

        for (std::vector<Actor*>::iterator it = m_gameObjects.begin(); it != m_gameObjects.end();) {
                (*it)->doSomething();

                if (!(*it)->isAlive()) {
                        delete *it;
                        it = m_gameObjects.erase(it);
                }
                else {
                        it++;
                }
        }

        if (!m_iceman->isAlive()) {
                decLives();
                return GWSTATUS_PLAYER_DIED;
        }
        else if (getNumItemsInLevel(IID_BARREL) - m_iceman->getItemCount(IID_BARREL) == 0) {
                return GWSTATUS_FINISHED_LEVEL;
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

void StudentWorld::addActor(Actor* gameObject) {
        m_gameObjects.push_back(gameObject);
}

bool StudentWorld::isDirObstructed(int x, int y, Actor::Direction dir) const {
        switch (dir) {
        case Actor::up:
                if (y >= VIEW_HEIGHT - SPRITE_WIDTH) {
                        return true;
                }

                for (int ox = x; ox < x + SPRITE_WIDTH; ox++) {
                        if (m_ice[y + SPRITE_WIDTH][ox] != nullptr) {
                                return true;
                        }
                }

                return false; // I need to return false under each case because I at least need break and this is clearer to readers.
        case Actor::down:
                if (y <= 0) {
                        return true;
                }

                for (int ox = x; ox < x + SPRITE_WIDTH; ox++) {
                        if (m_ice[y - 1][ox] != nullptr) {
                                return true;
                        }
                }

                return false;
        case Actor::left:
                if (x <= 0) {
                        return true;
                }

                for (int oy = y; oy < y + SPRITE_WIDTH; oy++) {
                        if (m_ice[oy][x - 1] != nullptr) {
                                return true;
                        }
                }

                return false;
        case Actor::right:
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

std::vector<Actor*> StudentWorld::getActors() const {
        return static_cast<const std::vector<Actor*>>(m_gameObjects);
};

Iceman* StudentWorld::getIceman() const {  // this is bad because someone else can delete the iceman
        return m_iceman;
}

// x1, y1, x2, y2 (original issue w/ diagonal lines etc) originally for isCollision
double StudentWorld::distance(int x1, int y1, int x2, int y2) {
        return std::sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void StudentWorld::cleanUp() {
        for (Actor* gameObject : m_gameObjects) {
                delete gameObject;
        }

        m_gameObjects.clear();

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

double StudentWorld::distanceFromIceman(int x, int y) const {
        return StudentWorld::distance(x, y, m_iceman->getX(), m_iceman->getY());
}
