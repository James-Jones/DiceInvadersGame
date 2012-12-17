#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <vector>
#include "DiceInvaders.h"
#include "pstdint.h"
#include "Vec2.h"

//Objects will be ordered using the sequence declared here.
//i.e. player before aliens before projectiles.
enum ObjectType {
    PLAYER,
    ENEMY1,
    ENEMY2,
    BOMB,
    ROCKET,
    NULL_OBJECT,//Marked for deletion. Deleted by CullObjects
    NUM_OBJECT_TYPES,
};

struct SceneObjectData
{
    SceneObjectData(){}
    ObjectType mType;
    Vec2 mPosition;
    Vec2 mVelocity;
};

typedef std::vector<SceneObjectData> SceneObjectVector;

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

//Pixels per second.
const float PLAYER_SPEED = 160.0f;
const float BOMB_SPEED = 128.0f;
const float ROCKET_SPEED = BOMB_SPEED*3;
const float ALIEN_SPEED = 8.0f;

//Seconds between rocket launch when
//fire key held down.
const float ROCKET_RATE_OF_FIRE = 0.3f;

const int MAX_SCORE = 99999999;
const int MAX_SCORE_DIGITS = 8;

void CreateObjects(const ObjectType type,
                   const uint32_t count,
                   const Vec2& pos,
                   const Vec2& vel,
                   const Vec2& deltaPos,
                   SceneObjectVector& objects);

void DrawObjects(SceneObjectVector& objects,
                 ISprite* __restrict sprites[NUM_OBJECT_TYPES]);

void MoveObjects(SceneObjectVector& objects,
                 const float deltaTimeInSecs);

void Animate(SceneObjectVector& objects,
                 const int timeInSecs);

void CullObjects(SceneObjectVector& objects,
                 const int width, const int height,
                 int cullCounts[NUM_OBJECT_TYPES]);

void CollideObjects(SceneObjectVector& objects,
                    int hitCounts[NUM_OBJECT_TYPES]);

void AliensRandomFire(SceneObjectVector& objects,
                 int floorLastTime, int floorNewTime);

void AliensChangeDirection(SceneObjectVector& objects,
                           Box& box,
                           const float clampMinX,
                           const float clampMaxX,
                           const float deltaTimeInSecs);

void CalcAlienBBox(SceneObjectVector& objects,
                   Box& box);

void SortObjectsByType(SceneObjectVector& objects);

void CountAliens(SceneObjectVector& objects,
               int& count);

void SpawnAliens(SceneObjectVector& objects, const int windowWidth);

#endif
