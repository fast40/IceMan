#include "Actor.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "StudentWorld.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iterator>
#include <system_error>


// ---------- ACTOR ----------

Actor::Actor(StudentWorld *studentWorld, int imageID, int startX, int startY, Direction dir, double size, unsigned int depth) : GraphObject(imageID, startX, startY, dir, size, depth), m_studentWorld(studentWorld), m_ID(imageID), m_isAlive(true) {
        setVisible(true);
}

void Actor::doSomething() {
        if (!isAlive()) {
                return;
        }
}

Actor::~Actor() {}

StudentWorld* Actor::getStudentWorld() const {
        return m_studentWorld;
}

int Actor::getID() const {
        return m_ID;
}

void Actor::move(Direction dir) {
        if (dir != none) {
                setDirection(dir);
        }

        int x = getX();
        int y = getY();

        switch(dir) {
        case up:
                moveTo(x, y + 1); 
                break;
        case down:
                moveTo(x, y - 1); 
                break;
        case left:
                moveTo(x - 1, y); 
                break;
        case right:
                moveTo(x + 1, y); 
                break;
        default:
                moveTo(x, y);
                break;
        }
}

void Actor::annoy(int amount) {}

void Actor::kill() {
        setVisible(false);
        m_isAlive = false;
}

bool Actor::isAlive() {
        return m_isAlive;
}

void Actor::bribe() {}

// ---------- AGENT ----------

Agent::Agent(StudentWorld *studentWorld, int health, int imageID, int startX, int startY, Direction dir, double size, unsigned int depth) : Actor(studentWorld, imageID, startX, startY, dir, size, depth), m_health(health) {}

void Agent::annoy(int amount) {
        m_health -= amount;
}

int Agent::getHealth() {
        return m_health;
}

// ---------- ICEMAN ----------

Iceman::Iceman(StudentWorld *studentWorld) : Agent(studentWorld, 10, IID_PLAYER, 30, 60, right, 1, 0), m_items({{IID_SONAR, 1}, {IID_GOLD, 0}, {IID_WATER_POOL, 5}, {IID_BARREL, 0}}) {}

void Iceman::doSomething() {
        Actor::doSomething();

        if (getHealth() <= 0) {
                kill();
                return;
        }

        if (getStudentWorld()->removeIceAt(getX(), getY())) {
                getStudentWorld()->playSound(SOUND_DIG);
        }

        int key;

        if(!getStudentWorld()->getKey(key)) {
                return;
        }

        switch (key) {
                case KEY_PRESS_ESCAPE:
                        kill();
                        return;
                case KEY_PRESS_SPACE:
                        fireSquirt();
                        break;
                case KEY_PRESS_TAB:
                        dropBribe();
                        break;
                case KEY_PRESS_UP: case KEY_PRESS_DOWN: case KEY_PRESS_LEFT: case KEY_PRESS_RIGHT:
                        break;
                case 'z': case 'Z':
                        useSonar();
                        return;
                case 'x':
                        m_items[IID_BARREL] = 100;
                        return;
        }

        Direction dir = keyToDir(key);

        if (getDirection() != dir && dir != none) {
                setDirection(dir);
        }
        else if (canMoveInDir(dir)) {
                move(dir);
        }
        else {
                move(none);
        }
}

void Iceman::dropBribe() {
        if (m_items[IID_GOLD] <= 0) {
                return;
        }

        getStudentWorld()->addActor(new Gold(getStudentWorld(), getX(), getY(), true, 100, false, true));
        m_items[IID_GOLD]--;
}

void Iceman::useSonar() {
        if (m_items[IID_SONAR] <= 0) {
                return;
        }

        getStudentWorld()->revealGameElementsWithinRadiusOfIceman(12);
        m_items[IID_SONAR]--;
}

void Iceman::giveItem(int itemID, int points) {
        if (itemID == IID_BARREL) {
                getStudentWorld()->playSound(SOUND_FOUND_OIL);
        }
        else {
                getStudentWorld()->playSound(SOUND_GOT_GOODIE);
        }

        if (itemID == IID_WATER_POOL) {
                m_items[itemID] += 5;
        }
        else {
                m_items[itemID]++;
        }

        getStudentWorld()->increaseScore(points);
};

int Iceman::getItemCount(int itemID) {
        return m_items[itemID];
}


void Iceman::fireSquirt() {
        if (getItemCount(IID_WATER_POOL) <= 0) {
                return;
        }

        getStudentWorld()->playSound(SOUND_PLAYER_SQUIRT);
        m_items[IID_WATER_POOL]--;

        int squirtX;
        int squirtY;

        switch (getDirection()) {
        case up:
                squirtX = getX();
                squirtY = getY() + SPRITE_WIDTH;
                break;
        case down:
                squirtX = getX();
                squirtY = getY() - SPRITE_WIDTH;
                break;
        case left:
                squirtX = getX() - SPRITE_WIDTH;
                squirtY = getY();
                break;
        case right:
                squirtX = getX() + SPRITE_WIDTH;
                squirtY = getY();
                break;
        case none:
                squirtX = getX();
                squirtY = getY();
                break;
        }

        getStudentWorld()->addActor(new Squirt(getStudentWorld(), squirtX, squirtY, getDirection()));
}

Actor::Direction Iceman::keyToDir(int key) {
        switch(key) {
        case KEY_PRESS_UP:
                return up;
        case KEY_PRESS_DOWN:
                return down;
        case KEY_PRESS_RIGHT:
                return right;
        case KEY_PRESS_LEFT:
                return left;
        default:
                return none;
        }
}

bool Iceman::canMoveInDir(Direction dir) const {
        int target_x = getX();
        int target_y = getY();

        switch(dir) {
        case up:
                target_y++;
                break;
        case down:
                target_y--;
                break;
        case left:
                target_x--;
                break;
        case right:
                target_x++;
                break;
        default:
                return false;
        }

        for (Actor* object : getStudentWorld()->getActors()) {
                if (object->getID() == IID_BOULDER && object->isVisible() && StudentWorld::distance(target_x, target_y, object->getX(), object->getY()) <= 3) {
                        return false;
                }
        }

        if (target_y > VIEW_HEIGHT - 4 || target_y < 0 || target_x > VIEW_WIDTH - 4 || target_x < 0) {
                return false;
        }

        // getStudentWorld().getActors();

        return true;
}

// ---------- BOULDER ----------

void Boulder::doSomething() {
        if (!isAlive()) {
                return;
        }

        switch (m_state) {
        case stable:
                if(getY() <= 0 || !getStudentWorld()->isDirObstructed(getX(), getY(), down)) {
                        m_state = waiting;
                }

                break;
        case waiting:
                m_ticksWaited++;

                if (m_ticksWaited >= 30) {
                        m_state = falling;
                        getStudentWorld()->playSound(SOUND_FALLING_ROCK);
                }

                break;
        case falling:
                {
                        int x = getX();
                        int y = getY();

                        for (Actor* actor : getStudentWorld()->getActors()) {
                                if (StudentWorld::distance(x, y, actor->getX(), actor->getY()) <= 3) {
                                        actor->annoy(100);
                                }
                        }

                        if (StudentWorld::distance(x, y, getStudentWorld()->getIceman()->getX(), getStudentWorld()->getIceman()->getY()) <= 3) {
                                getStudentWorld()->getIceman()->annoy(100);
                        }

                        if(y > 0 && !getStudentWorld()->isDirObstructed(x, y, down)) {
                                moveTo(getX(), getY() - 1);
                        }
                        else {
                                kill();
                        }
                }

                break;
        }
}

// ---------- PROTESTER ----------

Protester::Protester(StudentWorld *studentWorld, int x, int y, int health, int imageID) : Agent(studentWorld, health, imageID, x, y, left, 1, 0), m_numSquaresToMoveInCurrentDirection(0), m_state(moving), m_numTicksToWait(std::max(0, static_cast<int>(3 - getStudentWorld()->getLevel() / 4))) {}

void Protester::doSomething() {
        Actor::doSomething();

        if (getHealth() <= 0) {
                m_state = leaving;
        }

        switch (m_state) {
        case moving:
                // move(getDirection());

                if (m_numSquaresMovedInCurrentDirection >= m_numSquaresToMoveInCurrentDirection) {
                        
                }

                m_state = waiting;

                break;
        case waiting:
                m_numTicksWaited++;

                if (m_numTicksWaited >= m_numTicksToWait) {
                        m_state = moving;
                }

                break;
        case leaving:
                move(getStudentWorld()->getDirectionToExit(getX(), getY()));

                if(getX() == VIEW_WIDTH - SPRITE_WIDTH && getY() == VIEW_HEIGHT - SPRITE_WIDTH) {
                        kill();
                }

                break;
        }
}

void Protester::bribe() {
        m_state = leaving;
}

// ---------- REGULAR PROTESTER ----------

RegularProtester::RegularProtester(StudentWorld *studentWorld, int x, int y) : Protester(studentWorld, x, y, 5, IID_PROTESTER) {}

// ---------- HARDCORE PROTESTER ----------

HardcoreProtester::HardcoreProtester(StudentWorld *studentWorld, int x, int y) : Protester(studentWorld, x, y, 20, IID_HARD_CORE_PROTESTER) {}

// ---------- SQUIRT ----------

Squirt::Squirt(StudentWorld *studentWorld, int x, int y, Direction dir) : Actor(studentWorld, IID_WATER_SPURT, x, y, dir, 1, 1), m_distanceTraveled(0) {}

void Squirt::doSomething() {
        if (!isAlive()) {
                return;
        }
        else if (!getStudentWorld()->isDirObstructed(getX(), getY(), getDirection()) && m_distanceTraveled < 4) {
                move(getDirection());

                for (Actor* actor : getStudentWorld()->getActors()) {
                        if ((actor->getID() == IID_PROTESTER || actor->getID() == IID_HARD_CORE_PROTESTER) && StudentWorld::distance(getX(), getY(), actor->getX(), actor->getY()) <= 3) {
                                actor->annoy(2);
                                kill();
                        }
                }

                m_distanceTraveled++;
        }
        else {
                kill();
        }
}

// ---------- ITEM ----------

Item::Item(StudentWorld *studentWorld, int imageID, int points, int x, int y, bool visible, int timeLeft, bool canBePickedUpByIceman, bool canBePickedUpByProtesters) : Actor(studentWorld, imageID, x, y, right, 1, 2), m_points(points), m_timeLeft(timeLeft), m_canBePickedUpByIceman(canBePickedUpByIceman), m_canBePickedUpByProtesters(canBePickedUpByProtesters) {
        if (!visible) {
                setVisible(false);
        }
}

void Item::doSomething() {
        if (!isAlive()) {
                return;
        }

        if (m_timeLeft > 0) {
                m_timeLeft--;

                if (m_timeLeft == 0) {
                        kill();

                        return;
                }
        }

        if (m_canBePickedUpByIceman) {
                double distanceFromIceman = getStudentWorld()->distanceFromIceman(getX(), getY());

                if (!isVisible() && distanceFromIceman <= 4) {
                        setVisible(true);
                }

                if (distanceFromIceman <= 3) {
                        getStudentWorld()->getIceman()->giveItem(getID(), getPoints());
                        kill();
                }
        }

        if (m_canBePickedUpByProtesters) {
                for (Actor* actor : getStudentWorld()->getActors()) {
                        if (actor->getID() != IID_PROTESTER && actor->getID() != IID_HARD_CORE_PROTESTER) {
                                continue;
                        }

                        double distanceFromProtester = getStudentWorld()->distance(actor->getX(), actor->getY(), getX(), getY());

                        if (distanceFromProtester <= 3) {
                                actor->bribe();
                                kill();

                                return;
                        }
                }
        }
}

int Item::getPoints() {
        return m_points;
}

// ---------- GOLD ----------

Gold::Gold(StudentWorld *studentWorld, int x, int y, bool visible, int timeLeft, bool canBePickedUpByIceman, bool canBePickedUpByProtesters) : Item(studentWorld, IID_GOLD, 10, x, y, visible, timeLeft, canBePickedUpByIceman, canBePickedUpByProtesters) {}


// ---------- BARREL ----------

Barrel::Barrel(StudentWorld *studentWorld, int x, int y) : Item(studentWorld, IID_BARREL, 1000, x, y, false, -1, true, false) {}

// ---------- WATER ----------

Water::Water(StudentWorld *studentWorld, int x, int y, int timeLeft) : Item(studentWorld, IID_WATER_POOL, 100, x, y, true, timeLeft, true, false) {}

// ---------- SONAR ----------

Sonar::Sonar(StudentWorld *studentWorld, int x, int y, int timeLeft) : Item(studentWorld, IID_SONAR, 75, x, y, true, timeLeft, true, false) {}
