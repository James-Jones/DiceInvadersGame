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
    NULL_OBJECT,//Marked for deletion. Deleted by CullObjects
    NUM_OBJECT_TYPES,
};

struct CommonSceneObjectData
{
    CommonSceneObjectData(){}
    ObjectType mType;
    Vec2 mPosition;
    Vec2 mVelocity;
};

//First 4 are special. Player and health sprites.
const uint32_t FIRST_GENERIC_OBJECT = 4;

const uint32_t SPRITE_SIZE = 32; //In pixels. Width=Height.
const float F_SPRITE_SIZE = 32.0f;//Float version to help avoid casts and compiler warnings.

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

void CullObjects(std::vector<CommonSceneObjectData>& objects,
                 int width, int height);

void CollideObjects(std::vector<CommonSceneObjectData>& objects);

#endif
