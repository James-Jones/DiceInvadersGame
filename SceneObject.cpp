#include "SceneObject.h"
#include <cmath>
#include <assert.h>

//Currently a simple discrete method. Will fail to detect
//collision if not called frequently enough.
void CollideObjects(std::vector<CommonSceneObjectData>& objects,
                    int hitCounts[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        if(objects[index].mType == ROCKET)
        {
            //Rocket bitmap
            //12,7
            //17,26
            const float rx = objects[index].mPosition.x() + 12;
            const float ry = objects[index].mPosition.y() + 7;
            const float rx2 = rx + 5;
            const float ry2 = ry + 19;
            for(uint32_t innerIndex = FIRST_GENERIC_OBJECT; innerIndex < count; ++innerIndex)
            {
                if(objects[innerIndex].mType == ENEMY1 || objects[innerIndex].mType == ENEMY2 )
                {
                    const float left = objects[innerIndex].mPosition.x();
                    const float top = objects[innerIndex].mPosition.y();

                    const float right = left + SPRITE_SIZE;
                    const float bottom = top + SPRITE_SIZE;

                    if((rx > left) && (rx < right))
                    {
                        if((ry < bottom) && (ry > top))
                        {
                            hitCounts[objects[innerIndex].mType]++;
                            objects[innerIndex].mType = NULL_OBJECT;
                            objects[index].mType = NULL_OBJECT;
                        }
                    }
                }
            }
        }
    }
}

void CullObjects(std::vector<CommonSceneObjectData>& objects,
                 const int width, const int height)
{
    uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count;)
    {
        if(objects[index].mType == NULL_OBJECT ||
            objects[index].mPosition.x() < 0 || 
            objects[index].mPosition.x() > width ||

            objects[index].mPosition.y() < 0 || 
            objects[index].mPosition.y() > height)
        {
            //Best to delete from the end of vector. Swap with the end object
            //then delete.
            objects[index] = objects.back();
            objects.pop_back();
            count--;
            //Don't update index
            --index;
        }
        ++index;
    }
}

void Animate(std::vector<CommonSceneObjectData>& objects,
                 const int timeInSecs)
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        const ObjectType type = objects[index].mType;
        if(type == ENEMY1 || type == ENEMY2)
        {
            if(timeInSecs & 1)
                objects[index].mType = ENEMY2;
            else
                objects[index].mType = ENEMY1;

            //Move when sprite changes.
            if(type != objects[index].mType)
            {
                objects[index].mPosition += Vec2(8, 0);
            }
        }
    }
}

void MoveObjects(std::vector<CommonSceneObjectData>& objects,
                 const float deltaTimeInSecs)
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        objects[index].mPosition += objects[index].mVelocity * deltaTimeInSecs;
    }
}

void DrawObjects(std::vector<CommonSceneObjectData>& objects,
                 ISprite* __restrict sprites[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    for(uint32_t index = 0; index < count; ++index)
    {
        assert(objects[index].mType < NUM_OBJECT_TYPES);
        sprites[objects[index].mType]->draw(static_cast<int>(objects[index].mPosition.x()),
                                            static_cast<int>(objects[index].mPosition.y())-SPRITE_SIZE);
    }
}

//Add <count> objects of <type>. Intitialse with given position and veclocity.
void CreateObjects(const ObjectType type,
                   const uint32_t count,
                   const Vec2& pos,
                   const Vec2& vel,
                   const Vec2& deltaPos,
                   std::vector<CommonSceneObjectData>& objects)
{
    assert(type < NUM_OBJECT_TYPES);
    Vec2 accumPos = pos;
    for(uint32_t index = 0; index < count; ++index)
    {
        CommonSceneObjectData newObject;
        newObject.mType = type;
        newObject.mPosition = accumPos;
        newObject.mVelocity = vel;
        objects.push_back(newObject);

        accumPos += deltaPos;
    }
}
