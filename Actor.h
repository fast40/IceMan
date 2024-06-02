#ifndef ACTOR_H_
#define ACTOR_H_

#include "GameConstants.h"
#include "GameController.h"
#include "GraphObject.h"

#include <vector>

class StudentWorld;


class GameObject : public GraphObject {
public:
	GameObject(StudentWorld *studentWorld, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0) : GraphObject(imageID, startX, startY, dir, size, depth), m_studentWorld(studentWorld), m_ID(imageID) {
                setVisible(true);
        }
        virtual void doSomething() {}
        virtual ~GameObject() {}
        int getID() {
                return m_ID;
        }
        void move(Direction dir);
protected:
        StudentWorld *m_studentWorld;
private:
        int m_ID;
};

class Ice : public GameObject {
public:
        Ice(StudentWorld *studentWorld, int startX, int startY) : GameObject(studentWorld, IID_ICE, startX, startY, none, 0.25, 3) {}
};


class Iceman : public GameObject {
public:
        Iceman(StudentWorld *studentWorld) : GameObject(studentWorld, IID_PLAYER, 0, 0, right, 1, 0), m_state(alive), m_water(5) {}
        void doSomething();
        int getWater() { return m_water; }
private:
        bool isCollision(int x1, int x2, int y1, int y2);
        bool canMoveInDir(Direction dir);
        Direction keyToDir(int key);
        void fireSquirt();

        enum State { alive, dead };
        State m_state;

        int m_water;
};


class Boulder : public GameObject {
public:
        Boulder(StudentWorld *studentWorld, int x, int y) : GameObject(studentWorld, IID_BOULDER, x, y, none, 1, 1), m_ticksWaited(0) {}
        void doSomething();
private:
        enum State { stable, waiting, falling, fallen };
        State m_state;
        int m_ticksWaited;
};


class Squirt : public GameObject {
public:
        Squirt(StudentWorld *studentWorld, int x, int y, Direction dir) : GameObject(studentWorld, IID_WATER_SPURT, x, y, dir, 1, 2) {}
        void doSomething();
};


class Barrel : public GameObject {
public:
        Barrel(StudentWorld *studentWorld, int x, int y) : GameObject(studentWorld, IID_BARREL, x, y, none, 1, 2) {}
};


class Gold : public GameObject {
public:
        Gold(StudentWorld *studentWorld, int x, int y) : GameObject(studentWorld, IID_GOLD, x, y, none, 1, 2) {}
};


class Sonar : public GameObject {
public:
        Sonar(StudentWorld *studentWorld, int x, int y) : GameObject(studentWorld, IID_SONAR, x, y, none, 1, 2) {}
};


class Water : public GameObject {
public:
        Water(StudentWorld *studentWorld, int x, int y) : GameObject(studentWorld, IID_WATER_POOL, x, y, none, 1, 2) {}
};

// class Protester : public GameObject {
// public:
//         Protester(StudentWorld *studentWorld, int x, int y, int imageID) : GameObject(studentWorld, imageID, x, y, left, 1, 0) {}
// };

class Protester : public GameObject {
public:
        Protester(StudentWorld *studentWorld, int x, int y, int imageID) : GameObject(studentWorld, imageID, x, y, left, 1, 0), m_numSquaresToMoveInCurrentDirection(0), m_health(5) {}
        void doSomething();
private:
        enum State { moving, waiting, leaving, dead };
        int m_numSquaresToMoveInCurrentDirection;
        int m_numSquaresMovedInCurrentDirection;
        int m_numTicksToWait;
        int m_numTicksWaited;
        int m_health;
        State m_state;
};


// // maybe should remove this and hardcore can inherit from regular
// class RegularProtester : public Protester {
// public:
//         RegularProtester(StudentWorld *studentWorld, int x, int y) : Protester(studentWorld, x, y, IID_PROTESTER) {}
// };
//
// class HardcoreProtester : public Protester {
// public:
//         HardcoreProtester(StudentWorld *studentWorld, int x, int y) : Protester(studentWorld, x, y, IID_HARD_CORE_PROTESTER) {}
// };
//
// opening theme sound is in gamecontroller's doSomething


#endif // ACTOR_H_
