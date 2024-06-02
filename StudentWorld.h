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
        bool isDirObstructed(int x, int y, GameObject::Direction dir);

        void addGameObject(GameObject* gameObject);

        const std::vector<GameObject*> getGameObjects() {
                return static_cast<const std::vector<GameObject*>>(m_gameObjects);
        };

        Ice* (*getIce())[VIEW_WIDTH] {
                return m_ice;
        }
private:
        Ice* m_ice[VIEW_HEIGHT][VIEW_WIDTH];
        Iceman* m_iceman;
        std::vector<GameObject*> m_gameObjects;

        bool isDistributedObjectPositionOk(int x, int y);
};

#endif // STUDENTWORLD_H_
