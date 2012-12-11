#include "SceneObject.h"
#include <assert.h>

void CullObjects(std::vector<CommonSceneObjectData>& objects,
                 int width, int height)
{
    uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        if(objects[index].mPosition.x() < 0 || 
            objects[index].mPosition.x() > width ||

            objects[index].mPosition.y() < 0 || 
            objects[index].mPosition.y() > height)
        {
            //Move last to here.
            objects[index] = objects.back();
            //Delete last
            objects.pop_back();
            //Update count
            count--;
        }
    }
}

void Animate(std::vector<CommonSceneObjectData>& objects,
                 int timeInSecs)
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        if(objects[index].mType == ENEMY1 || objects[index].mType == ENEMY2)
        {
            if(timeInSecs & 1)
                objects[index].mType = ENEMY2;
            else
                objects[index].mType = ENEMY1;

        }
    }
}

void MoveObjects(std::vector<CommonSceneObjectData>& objects,
                 float deltaTimeInSecs)
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        objects[index].mPosition += objects[index].mVelocity * deltaTimeInSecs;
    }
}

void DrawObjects(std::vector<CommonSceneObjectData>& objects,
                 ISprite* sprites[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    for(uint32_t index = 0; index < count; ++index)
    {
        assert(objects[index].mType < NUM_OBJECT_TYPES);
        sprites[objects[index].mType]->draw(objects[index].mPosition.x(),
                                            objects[index].mPosition.y()-SPRITE_SIZE);
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
