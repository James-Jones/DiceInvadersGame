#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include "DiceInvaders.h"
#include "pstdint.h"
#include "Vec2.h"

enum ObjectType {
    ROCKET,
    BOMB,
    PLAYER,
    ENEMY1,
    ENEMY2,
    NUM_OBJECT_TYPES,
};

struct CommonSceneObjectData
{
    CommonSceneObjectData(){}
    ObjectType mType;
    Vec2 mPosition;
    Vec2 mVelocity;
};

const uint32_t SPRITE_SIZE = 32; //In pixels. Width=Height.

void CreateObjects(const ObjectType type,
                   const uint32_t count,
                   const Vec2& pos,
                   const Vec2& vel,
                   const Vec2& deltaPos,
                   std::vector<CommonSceneObjectData>& objects);

void DrawObjects(std::vector<CommonSceneObjectData>& objects,
                 ISprite* sprites[NUM_OBJECT_TYPES]);

void MoveObjects(std::vector<CommonSceneObjectData>& objects,
                 float deltaTimeInSecs);

void Animate(std::vector<CommonSceneObjectData>& objects,
                 int timeInSecs);

#endif
