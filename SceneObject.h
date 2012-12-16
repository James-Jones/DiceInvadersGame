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

struct Box
{
    float mLeft;
    float mRight;
    float mTop;
    float mBottom;
};

//Player is object 0.
const uint32_t FIRST_GENERIC_OBJECT = 1;

const uint32_t SPRITE_SIZE = 32; //In pixels. Width=Height.
const float F_SPRITE_SIZE = 32.0f;//Float version to help avoid casts and compiler warnings.

void CreateObjects(const ObjectType type,
                   const uint32_t count,
                   const Vec2& pos,
                   const Vec2& vel,
                   const Vec2& deltaPos,
                   std::vector<CommonSceneObjectData>& objects);

void DrawObjects(std::vector<CommonSceneObjectData>& objects,
                 ISprite* __restrict sprites[NUM_OBJECT_TYPES]);

void MoveObjects(std::vector<CommonSceneObjectData>& objects,
                 const float deltaTimeInSecs);

void Animate(std::vector<CommonSceneObjectData>& objects,
                 const int timeInSecs);

void CullObjects(std::vector<CommonSceneObjectData>& objects,
                 const int width, const int height);

void CollideObjects(std::vector<CommonSceneObjectData>& objects,
                    int hitCounts[NUM_OBJECT_TYPES]);

void AliensRandomFire(std::vector<CommonSceneObjectData>& objects,
                 int floorLastTime, int floorNewTime);

void AliensChangeDirection(std::vector<CommonSceneObjectData>& objects,
                           Box& box,
                           const float clampMinX,
                           const float clampMaxX,
                           const float deltaTimeInSecs);

void CalcAlienBBox(std::vector<CommonSceneObjectData>& objects,
                 Box& box);

#endif
