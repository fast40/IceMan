#include "Actor.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include "StudentWorld.h"
#include <algorithm>
#include <filesystem>
#include <system_error>

void GameObject::move(Direction dir) {
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

void Iceman::doSomething() {
        if (m_state == dead) {
                return;
        }

        if (m_studentWorld->removeIceAt(getX(), getY())) {
                m_studentWorld->playSound(SOUND_DIG);
        }

        int key;

        if(!Game().getLastKey(key)) {
                return;
        }

        switch (key) {
                case KEY_PRESS_ESCAPE:
                        m_state = dead;
                        return;
                case KEY_PRESS_SPACE:
                        fireSquirt();
                        break;
                case KEY_PRESS_TAB:
                        break;
                case KEY_PRESS_UP: case KEY_PRESS_DOWN: case KEY_PRESS_LEFT: case KEY_PRESS_RIGHT:
                        break;
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

void Iceman::fireSquirt() {
        if (m_water <= 0) {
                return;
        }

        m_studentWorld->playSound(SOUND_PLAYER_SQUIRT);
        m_water--;


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

        m_studentWorld->addGameObject(new Squirt(m_studentWorld, squirtX, squirtY, getDirection()));
}

GameObject::Direction Iceman::keyToDir(int key) {
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

// x1, y1, x2, y2 (original issue w/ diagonal lines etc)
bool Iceman::isCollision(int x1, int y1, int x2, int y2) {
        return ((x1 < x2 + 4) && (x1 > x2 - 4) && (y1 < y2 + 4) && (y1 > y2 - 4));
}

bool Iceman::canMoveInDir(Direction dir) {
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

        for (GameObject* object : m_studentWorld->getGameObjects()) {
                if (object->getID() == IID_BOULDER && object->isVisible() && isCollision(target_x, target_y, object->getX(), object->getY())) {
                        return false;
                }
        }

        if (target_y > VIEW_HEIGHT - 4 || target_y < 0 || target_x > VIEW_WIDTH - 4 || target_x < 0) {
                return false;
        }

        // m_studentWorld->getGameObjects();

        return true;
}





//------------------boulder ------------------


void Boulder::doSomething() {
        switch (m_state) {
        case stable:
                if(getY() <= 0 || !m_studentWorld->isDirObstructed(getX(), getY(), down)) {
                        m_state = waiting;
                }

                break;
        case waiting:
                m_ticksWaited++;

                if (m_ticksWaited >= 30) {
                        m_state = falling;
                        m_studentWorld->playSound(SOUND_FALLING_ROCK);
                }

                break;
        case falling:
                {
                        int x = getX();
                        int y = getY();

                        if(y > 0 && !m_studentWorld->isDirObstructed(x, y, down)) {
                                moveTo(getX(), getY() - 1);
                        }
                        else {
                                setVisible(false);
                                m_state = fallen;
                        }
                }

                break;
        case fallen:
                return;
        }
}






// ------------------------ protestor -----------------------

void Protester::doSomething() {
        if (m_health <= 0) {
                setVisible(false);
                m_state = dead;
        }

        switch (m_state) {
        case moving:
                move(getDirection());

                if (m_numSquaresMovedInCurrentDirection >= m_numSquaresToMoveInCurrentDirection) {
                        m_numTicksToWait = std::max(0, static_cast<int>(3 - m_studentWorld->getLevel() / 4));
                        m_state = waiting;
                }

                break;
        case waiting:
                m_numTicksWaited++;

                if (m_numTicksWaited >= m_numTicksToWait) {
                        m_state = moving;
                }

                break;
        case leaving:
                break;
        case dead:
                return;
        }
}


// ---------------------- squirt ---------------


void Squirt::doSomething() {
        if (!m_studentWorld->isDirObstructed(getX(), getY(), getDirection())) {
                std::cout << "move" << std::endl;
                move(getDirection());
        }
        else {
                setVisible(false);
        }
}
