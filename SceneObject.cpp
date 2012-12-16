#include "SceneObject.h"
#include <cmath>
#include <assert.h>

void SpawnAliens(ObjectVector& objects, const float fScreenWidth)
{
    const int NumAlienRows = 8;
    for(int i =0; i < NumAlienRows; ++i)
    {
        //One row of aliens.
        CreateObjects(ENEMY1,
            static_cast<uint32_t>(std::floor(fScreenWidth/F_SPRITE_SIZE*.75f)),
            Vec2(1.0f, F_SPRITE_SIZE + F_SPRITE_SIZE * i),
            Vec2(1.0f, 0.0f), Vec2(F_SPRITE_SIZE, 0.0f), objects);
    }
}

static void SortObjectsByType(std::vector<CommonSceneObjectData>& objects)
{
    //Buble sort objects so that entities of the same type are next to each other.
    bool swapped = true;
    while(swapped)
    {
        swapped = false;
        for(int i = 1; i < objects.size(); ++i)
        {
            if(objects[i-1].mType > objects[i].mType)
            {
                std::swap(objects[i-1], objects[i]);
                swapped = true;
            }
        }
    }
}


void CalcAlienBBox(ObjectVector& objects,
                   Box& box)
{
    box.mBottom = 0.0f;
    box.mTop = 100000.0f; //Assumes window smaller than this.
    box.mLeft = 100000.0f;
    box.mRight = 0.0f;

    for(ObjectVector::iterator itor = objects.begin(); itor != objects.end(); ++itor)
    {
        if(itor->mType == ENEMY1 || itor->mType == ENEMY2)
        {
            box.mBottom = std::max(box.mBottom, itor->mPosition.y());
            box.mTop = std::min(box.mTop, itor->mPosition.y()-SPRITE_SIZE);
            box.mLeft = std::min(box.mLeft, itor->mPosition.x());
            box.mRight = std::max(box.mRight, itor->mPosition.x()+SPRITE_SIZE);
        }
    }
}

void AliensChangeDirection(std::vector<CommonSceneObjectData>& objects,
                           Box& box,
                           const float clampMinX,
                           const float clampMaxX,
                           const float deltaTimeInSecs)
{
    const uint32_t count = objects.size();

    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        CommonSceneObjectData& obj = objects[index];

        if((objects[index].mType == ENEMY1 ||
            objects[index].mType == ENEMY2))
        {
            objects[index].mPosition += Vec2(0, F_SPRITE_SIZE);//Dropd down
            objects[index].mVelocity = Vec2(-1*objects[index].mVelocity.x(), objects[index].mVelocity.y());//Reverse x-direction

            //Snap position away from the edge so it does not get culled during
            //CullObjects pass
            const float clampedX = std::min(std::max(clampMinX, objects[index].mPosition.x()), clampMaxX);
            objects[index].mPosition = Vec2(clampedX, objects[index].mPosition.y());
        }
    }
}

//Pick a random object each second. If the object is an alien
//then it fires a bomb.
void AliensRandomFire(std::vector<CommonSceneObjectData>& objects,
                 int floorLastTime, int floorNewTime)
{
    if(floorLastTime != floorNewTime) //At least one second has passed.
    {
        const uint32_t count = objects.size();
        const uint32_t index = std::rand() % count;

        if(objects[index].mType == ENEMY1 ||
            objects[index].mType == ENEMY2 )
        {
            CreateObjects(BOMB, 1,
                objects[index].mPosition + Vec2(0.0f, F_SPRITE_SIZE),
                Vec2(0, 32), Vec2(0, 0), objects);
        }
    }
}

//Currently a simple discrete method. Will fail to detect
//collision if not called frequently enough.
void CollideObjects(std::vector<CommonSceneObjectData>& objects,
                    int hitCounts[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    bool bResort = false;
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
                            bResort = true;
                        }
                    }
                }
            }
        }

        if(objects[index].mType == BOMB)
        {
            //Rocket bitmap
            //12,7
            //17,26
            const float rx = objects[index].mPosition.x() + 12;
            const float ry = objects[index].mPosition.y() + 7;
            const float rx2 = rx + 5;
            const float ry2 = ry + 19;
            
            const uint32_t innerIndex = 0;

            const float left = objects[innerIndex].mPosition.x();
            const float top = objects[innerIndex].mPosition.y();

            const float right = left + SPRITE_SIZE;
            const float bottom = top + SPRITE_SIZE;

            if((rx > left) && (rx < right))
            {
                if((ry < bottom) && (ry > top))
                {
                    hitCounts[objects[innerIndex].mType]++;
                    objects[index].mType = NULL_OBJECT;
                    bResort = true;
                }
            }
        }
    }

    //Some objects may have changed to null type.
    if(bResort)
    {
        //SortObjectsByType(objects);
    }
}

void CullObjects(std::vector<CommonSceneObjectData>& objects,
                 const int width, const int height,
                 int cullCounts[NUM_OBJECT_TYPES])
{
    uint32_t count = objects.size();
    bool bResort = false;
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count;)
    {
        if(objects[index].mType == NULL_OBJECT ||
            objects[index].mPosition.x() < -1 || 
            objects[index].mPosition.x() > width+1 ||

            objects[index].mPosition.y() < -1 || 
            objects[index].mPosition.y() > height+1)
        {
            cullCounts[objects[index].mType]++;

            //Best to delete from the end of vector. Swap with the end object
            //then delete.
            objects[index] = objects.back();
            objects.pop_back();
            count--;
            //Don't update index
            --index;
            bResort = true;
        }
        ++index;
    }

    if(bResort)
    {
        //SortObjectsByType(objects);
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
                objects[index].mPosition += Vec2(8 * objects[index].mVelocity.x(), 0);
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


//Animate, CalcAlienBBox, AliensChangeDirection and AliensRandomFire only
//operate on alien objects. 
void CountAliens(ObjectVector& objects,
               int& count)
{
    for(ObjectVector::iterator itor = objects.begin(); itor != objects.end(); ++itor)
    {
        if(itor->mType == ENEMY1 || itor->mType == ENEMY2)
            count++;
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

    //SortObjectsByType(objects);
}
