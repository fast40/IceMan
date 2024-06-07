#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include <unordered_map>
#include <vector>
#include <string>

class StudentWorld : public GameWorld
{
public:
        struct coordinate {
                int x;
                int y;
        };

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
        coordinate getCoordinatesFor(int itemID);
        void revealGameElementsWithinRadiusOfIceman(int radius);
        void calculateShortestPathsToExit();
        coordinate getNextLocationOnEscapeRoute(int x, int y);
        Actor::Direction getDirectionToExit(int x, int y);
private:
        Ice* m_ice[VIEW_HEIGHT][VIEW_WIDTH];
        Iceman* m_iceman;
        std::vector<Actor*> m_actors;
        std::vector<coordinate> m_shortestPathsToExit[VIEW_HEIGHT][VIEW_WIDTH];
        Actor::Direction m_directionsToExit[VIEW_HEIGHT][VIEW_WIDTH];
        // std::unordered_map<std::pair<int, int>, std::vector<std::pair<int, int>>> m_shortestPathsToExit;

        bool isFreePosition(int x, int y);
        std::vector<coordinate> getFreeNeighbors(int x, int y);
};

#endif // STUDENTWORLD_H_
