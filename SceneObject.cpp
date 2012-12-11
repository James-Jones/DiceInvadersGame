#include "SceneObject.h"

void DrawObjects(std::vector<CommonSceneObjectData>& objects,
                 ISprite* sprites[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    for(uint32_t index = 0; index < count; ++index)
    {
        sprites[objects[index].mType]->draw(objects[index].mPosition.x(),
                                            objects[index].mPosition.y()-32);
    }
}

//Add <count> objects of <type>. Intitialse with given position and veclocity.
void CreateObjects(const ObjectType type,
                   const uint32_t count,
                   const Vec2& pos,
                   const Vec2& vel,
                   std::vector<CommonSceneObjectData>& objects)
{
    for(uint32_t index = 0; index < count; ++index)
    {
        CommonSceneObjectData newObject;
        newObject.mType = type;
        newObject.mPosition = pos;
        newObject.mVelocity = vel;
        objects.push_back(newObject);
    }
}
