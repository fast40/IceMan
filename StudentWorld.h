#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include <string>

class StudentWorld : public GameWorld
{
public:
        StudentWorld(std::string assetDir);
        virtual int init();
        virtual int move();
        virtual void cleanUp();
        virtual ~StudentWorld();
        bool removeIceAt(int x, int y);
        bool isDirObstructed(int x, int y, Actor::Direction dir) const;
        void addActor(Actor* gameObject);
        std::vector<Actor*> getActors() const;
        Iceman* getIceman() const;
        static double distance(int x1, int y1, int x2, int y2);
        double distanceFromIceman(int x, int y) const;
        int getNumItemsInLevel(int itemID);
private:
        Ice* m_ice[VIEW_HEIGHT][VIEW_WIDTH];
        Iceman* m_iceman;
        std::vector<Actor*> m_gameObjects;

        bool isDistributedObjectPositionOk(int x, int y) const;
};

#endif // STUDENTWORLD_H_
