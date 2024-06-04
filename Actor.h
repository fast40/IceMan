#ifndef ACTOR_H_
#define ACTOR_H_

#include "GameConstants.h"
#include "GameController.h"
#include "GraphObject.h"

#include <vector>
#include <unordered_map>

class StudentWorld;


class Actor : public GraphObject {
public:
        Actor(StudentWorld *studentWorld, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0);
        virtual void doSomething();
        virtual ~Actor();

        StudentWorld* getStudentWorld() const;
        int getID() const;
        void move(Direction dir);
        void annoy(int amount = 1); 
        void kill();
        bool isAlive();
        void bribe();
private:
        StudentWorld *m_studentWorld;
        bool m_isAlive;
        int m_ID;
};


class Agent : public Actor {
public:
        Agent(StudentWorld *studentWorld, int health, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0);
        void annoy(int amount = 1); 
        int getHealth() {
                return m_health;
        }
private:
        int m_health = 5;
};


class Iceman : public Agent {
public:
        Iceman(StudentWorld *studentWorld);
        void doSomething();
        void giveItem(int itemID, int points);
        int getLives() const { return m_lives; }
        int getItemCount(int itemID);  // this should be const (ugly)
        void dropBribe();
private:
        bool canMoveInDir(Direction dir) const;
        Direction keyToDir(int key);
        void fireSquirt();
        int m_lives = 3;
        std::unordered_map<int, int> m_items;
};


class Ice : public Actor {
public:
        Ice(StudentWorld *studentWorld, int startX, int startY) : Actor(studentWorld, IID_ICE, startX, startY, none, 0.25, 3) {}
};


class Boulder : public Actor {
public:
        Boulder(StudentWorld *studentWorld, int x, int y) : Actor(studentWorld, IID_BOULDER, x, y, none, 1, 1), m_ticksWaited(0) {}
        void doSomething();
private:
        enum State { stable, waiting, falling };
        State m_state;
        int m_ticksWaited;
};


class Squirt : public Actor {
public:
        Squirt(StudentWorld *studentWorld, int x, int y, Direction dir) : Actor(studentWorld, IID_WATER_SPURT, x, y, dir, 1, 2), m_distanceTraveled(0) {}
        void doSomething();
private:
        int m_distanceTraveled;
};


class Item : public Actor {
public:
        Item(StudentWorld *studentWorld, int imageID, int points, int x, int y, bool visible, int timeLeft, bool canBePickedUpByIceman, bool canBePickedUpByProtesters);
        void doSomething();
        int getPoints();
private:
        const int m_points;
        bool m_canBePickedUpByIceman;
        bool m_canBePickedUpByProtesters;
        int m_timeLeft;
};


class Barrel : public Item {
public:
        Barrel(StudentWorld *studentWorld, int x, int y);
};


class Gold : public Item {
public:
        Gold(StudentWorld *studentWorld, int x, int y, bool visible, int timeLeft, bool canBePickedUpByIceman, bool canBePickedUpByProtesters);
};


class Sonar : public Item {
public:
        Sonar(StudentWorld *studentWorld, int x, int y, int timeLeft);
};


class Water : public Item {
public:
        Water(StudentWorld *studentWorld, int x, int y);
};

// class Protester : public Actor {
// public:
//         Protester(StudentWorld *studentWorld, int x, int y, int imageID) : Actor(studentWorld, imageID, x, y, left, 1, 0) {}
// };

class Protester : public Actor {
public:
        Protester(StudentWorld *studentWorld, int x, int y, int imageID) : Actor(studentWorld, imageID, x, y, left, 1, 0), m_numSquaresToMoveInCurrentDirection(0), m_health(5) {}
        void doSomething();
        void bribe();
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
