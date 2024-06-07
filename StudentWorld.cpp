#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "GameController.h"
#include "freeglut_std.h"
#include <array>
#include <cstdio>
#include <future>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <queue>
#include <unordered_map>

GameWorld* createStudentWorld(std::string assetDir) {
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir) : GameWorld(assetDir) {
        srand(0);
}

int StudentWorld::getNumItemsInLevel(int itemID) {
        switch (itemID) {
        case IID_BOULDER:
                // return 20;
                return std::min(2 + static_cast<int>(getLevel() / 2), 9);
        case IID_GOLD:
                return std::max(5 - static_cast<int>(getLevel() / 2), 2);
        case IID_BARREL:
                // return 1;
                return std::min(2 + static_cast<int>(getLevel()), 9);
        default:
                return 0;
        }
}

// random algorithm failed when there were too many elements already in place
// need to make sure this does the right thing for boulders water pools (spawn in places with no ice) etc.


StudentWorld::coordinate StudentWorld::getCoordinatesFor(int itemID) {
        if (itemID == IID_SONAR) {
                return coordinate{0, 60};
        }

        std::vector<coordinate> candidateCoordinates;

        for (int y = 0; y < VIEW_HEIGHT - (SPRITE_WIDTH * 2); y++) {
                for (int x = 0; x < VIEW_WIDTH - SPRITE_WIDTH; x++) {
                        if (itemID == IID_BOULDER && (x <= 30 - SPRITE_WIDTH || x > 33) && y >= 4) {
                                candidateCoordinates.push_back(coordinate{x, y});
                        }
                        else if ((itemID == IID_GOLD || itemID == IID_BARREL) && (x <= 30 - SPRITE_WIDTH || x > 33 || y == 0)) {
                                candidateCoordinates.push_back(coordinate{x, y});
                        }
                        else if (itemID == IID_WATER_POOL) {
                                bool shouldAdd = true;

                                for (int iy = y; iy < y + 4; iy++) {
                                        for (int ix = x; ix < x + 4; ix++) {
                                                if (m_ice[iy][ix] != nullptr) {
                                                        shouldAdd = false;
                                                }
                                        }
                                }

                                if (shouldAdd) {
                                        candidateCoordinates.push_back(coordinate{x, y});
                                }
                        }
                }
        }

        candidateCoordinates.erase(std::remove_if(candidateCoordinates.begin(), candidateCoordinates.end(), [this](const coordinate& c) {
                for (Actor* actor : getActors()) {
                        if (distance(c.x, c.y, actor->getX(), actor->getY()) <= 6) {
                                return true;
                        }
                }

                return false;

        }), candidateCoordinates.end());

        if (candidateCoordinates.size() == 0) {
                return coordinate{-1, -1};
        }

        int index = rand() % candidateCoordinates.size();

        return candidateCoordinates[index];
}

bool StudentWorld::isFreePosition(int x, int y) {
        if (x > VIEW_WIDTH - SPRITE_WIDTH || y > VIEW_WIDTH - SPRITE_WIDTH) {
                return false;
        }

        for (int iy = y; iy < y + SPRITE_WIDTH; iy++) {
                for (int ix = x; ix < x + SPRITE_WIDTH; ix++) {
                        if (m_ice[iy][ix] != nullptr) {
                                return false;
                        }
                }
        }

        return true;
}

std::vector<StudentWorld::coordinate> StudentWorld::getFreeNeighbors(int x, int y) {
        std::vector<coordinate> freeNeighbors;

        if (isFreePosition(x - 1, y)) {
                freeNeighbors.push_back(coordinate{x - 1, y});
        }

        if (isFreePosition(x + 1, y)) {
                freeNeighbors.push_back(coordinate{x + 1, y});
        }

        if (isFreePosition(x, y - 1)) {
                freeNeighbors.push_back(coordinate{x, y - 1});
        }

        if (isFreePosition(x, y + 1)) {
                freeNeighbors.push_back(coordinate{x, y + 1});
        }

        return freeNeighbors;
}


void StudentWorld::calculateShortestPathsToExit() {
        std::queue<coordinate> queue;

        coordinate start = coordinate{VIEW_WIDTH - SPRITE_WIDTH, VIEW_HEIGHT - SPRITE_WIDTH};

        for (int iy = 0; iy < VIEW_HEIGHT; iy++) {
                for (int ix = 0; ix < VIEW_HEIGHT; ix++) {
                        m_shortestPathsToExit[iy][ix] = std::vector<coordinate>();
                        m_directionsToExit[iy][ix] = Actor::Direction::none;
                }
        }

        m_shortestPathsToExit[start.y][start.x].push_back(start);
        queue.push(start);

        while (!queue.empty()) {
                coordinate current = queue.front();
                queue.pop();

                for (coordinate neighbor : getFreeNeighbors(current.x, current.y)) {
                        if (m_shortestPathsToExit[neighbor.y][neighbor.x].empty()) {
                                if (current.y > neighbor.y) { // current is above neighbor so neighbor needs to go up etc
                                        m_directionsToExit[neighbor.y][neighbor.x] = Actor::Direction::up;
                                }
                                else if (current.y < neighbor.y) {
                                        m_directionsToExit[neighbor.y][neighbor.x] = Actor::Direction::down;
                                }
                                else if (current.x < neighbor.x) {
                                        m_directionsToExit[neighbor.y][neighbor.x] = Actor::Direction::left;
                                }
                                else if (current.x > neighbor.x) {
                                        m_directionsToExit[neighbor.y][neighbor.x] = Actor::Direction::right;
                                }
                                else {
                                        m_directionsToExit[neighbor.y][neighbor.x] = Actor::Direction::none;
                                }

                                m_shortestPathsToExit[neighbor.y][neighbor.x] = m_shortestPathsToExit[current.y][current.x];
                                m_shortestPathsToExit[neighbor.y][neighbor.x].push_back(neighbor);
                                queue.push(neighbor);
                        }
                }
        }

}

StudentWorld::coordinate StudentWorld::getNextLocationOnEscapeRoute(int x, int y) {
        if (m_shortestPathsToExit[y][x].empty()) {
                return coordinate{x, y};
        }
        return m_shortestPathsToExit[y][x][m_shortestPathsToExit[y][x].size() - 2];
};

Actor::Direction StudentWorld::getDirectionToExit(int x, int y) {
        return m_directionsToExit[y][x];
}

void StudentWorld::revealGameElementsWithinRadiusOfIceman(int radius) {
        int x = m_iceman->getX();
        int y = m_iceman->getY();

        for (Actor* object : getActors()) {
                if (distance(x, y, object->getX(), object->getY()) <= radius && object->isAlive()) {
                        object->setVisible(true);
                }
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
                coordinate c = getCoordinatesFor(IID_BOULDER);
                addActor(new Boulder(this, c.x, c.y));
                removeIceAt(c.x, c.y);
        }

        for (int i = 0; i < getNumItemsInLevel(IID_BARREL); i++) {
                coordinate c = getCoordinatesFor(IID_BARREL);
                addActor(new Barrel(this, c.x, c.y));
        }

        for (int i = 0; i < getNumItemsInLevel(IID_GOLD); i++) {
                coordinate c = getCoordinatesFor(IID_GOLD);
                addActor(new Gold(this, c.x, c.y, false, -1, true, false));
        }

        addActor(new RegularProtester(this, 50, 60));
        addActor(new HardcoreProtester(this, 60, 60));

        return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
        calculateShortestPathsToExit();
        // format this nicely later
        std::string statusBar = "Lvl:  " + std::to_string(getLevel()) + "  " +
                                "Lives: " + std::to_string(getLives()) + "  " + 
                                "Hlth: " + std::to_string((m_iceman->getHealth() / 10) * 100) + "%  " +
                                "Wtr:  " + std::to_string(m_iceman->getItemCount(IID_WATER_POOL)) + "  " +
                                "Gld:  " + std::to_string(m_iceman->getItemCount(IID_GOLD)) + "  " +
                                "Oil Left:  " + std::to_string(getNumItemsInLevel(IID_BARREL) - m_iceman->getItemCount(IID_BARREL)) + "  " + 
                                "Sonar:  " + std::to_string(m_iceman->getItemCount(IID_SONAR)) + "  " +
                                "Scr: " + std::to_string(getScore());

        setGameStatText(statusBar);

        m_iceman->doSomething();

        for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end();) {
                (*it)->doSomething();

                if (!(*it)->isAlive()) {
                        delete *it;
                        it = m_actors.erase(it);
                }
                else {
                        it++;
                }
        }

        if ((static_cast<double>(rand()) / RAND_MAX) < (1.0 / (getLevel() * 25 + 300))) {
                if ((static_cast<double>(rand()) / RAND_MAX) < (1.0 / 5.0)) {
                        coordinate c = getCoordinatesFor(IID_SONAR);
                        addActor(new Sonar(this, c.x, c.y, -1));
                }
                else {
                        coordinate c = getCoordinatesFor(IID_WATER_POOL);
                        int timeLeft = std::max(100, static_cast<int>(300 - (10 * getLevel())));
                        addActor(new Water(this, c.x, c.y, timeLeft));
                }
        }

        if (!m_iceman->isAlive()) {
                decLives();
                return GWSTATUS_PLAYER_DIED;
        }
        else if (getNumItemsInLevel(IID_BARREL) - m_iceman->getItemCount(IID_BARREL) <= 0) {
                playSound(SOUND_FINISHED_LEVEL);
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
        m_actors.push_back(gameObject);
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
        return m_actors;
};

Iceman* StudentWorld::getIceman() const {  // this is bad because someone else can delete the iceman
        return m_iceman;
}

// x1, y1, x2, y2 (original issue w/ diagonal lines etc) originally for isCollision
double StudentWorld::distance(int x1, int y1, int x2, int y2) {
        return std::sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void StudentWorld::cleanUp() {
        for (Actor* actor : getActors()) {
                delete actor;
        }

        m_actors.clear();

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
