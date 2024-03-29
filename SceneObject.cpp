#include "SceneObject.h"
#include <cmath>
#include <assert.h>

void SpawnAliens(SceneObjectVector& objects, const int windowWidth)
{
    const int NumAlienRows = 8;
    for(int i =0; i < NumAlienRows; ++i)
    {
        //One row of aliens.
        CreateObjects(ENEMY1,
            static_cast<uint32_t>(std::floor(windowWidth/F_SPRITE_SIZE*0.66f)),
            Vec2(1.0f, F_SPRITE_SIZE + F_SPRITE_SIZE * i),
            Vec2(1.0f, 0.0f),
            Vec2(F_SPRITE_SIZE + 4.0f, 0.0f),
            objects);
    }
}

static void SortObjectsByType(SceneObjectVector& objects)
{
    //Buble sort objects so that entities of the same type are next to each other.
    bool swapped = true;
    while(swapped)
    {
        swapped = false;
        for(size_t i = 1; i < objects.size(); ++i)
        {
            if(objects[i-1].mType > objects[i].mType)
            {
                std::swap(objects[i-1], objects[i]);
                swapped = true;
            }
        }
    }
}

void CalcAlienBBox(SceneObjectVector& objects,
                   Box& box)
{
    box.mBottom = 0.0f;
    box.mTop = 100000.0f; //Assumes window smaller than this.
    box.mLeft = 100000.0f;
    box.mRight = 0.0f;

    for(SceneObjectVector::iterator itor = objects.begin(); itor != objects.end(); ++itor)
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

void AliensChangeDirection(SceneObjectVector& objects,
                           Box& box,
                           const float clampMinX,
                           const float clampMaxX,
                           const float deltaTimeInSecs)
{
    const uint32_t count = objects.size();

    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        SceneObjectData& obj = objects[index];

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
void AliensRandomFire(SceneObjectVector& objects,
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
                Vec2(0, BOMB_SPEED), Vec2(0, 0), objects);
        }
    }
}

//Currently a simple discrete method. Will fail to detect
//collision if not called frequently enough.
void CollideObjects(SceneObjectVector& objects,
                    int hitCounts[NUM_OBJECT_TYPES])
{
    const uint32_t count = objects.size();
    bool bResort = false;

    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        if(objects[index].mType == ROCKET)
        {
            //Rocket bitmap dimensions (outside of this is black)
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
            //Bomb bitmap dimensions (outside of this is black)
            //9,8
            //20,25
            const float rx = objects[index].mPosition.x() + 9;
            const float ry = objects[index].mPosition.y() + 8;
            const float rx2 = rx + 11;
            const float ry2 = ry + 17;
            
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
        SortObjectsByType(objects);
    }
}

void CullObjects(SceneObjectVector& objects,
                 const int width, const int height,
                 int cullCounts[NUM_OBJECT_TYPES])
{
    bool bResort = false;

    //Delete null objects. They are sorted to be at the end of the
    //vector.
    assert(NULL_OBJECT == NUM_OBJECT_TYPES -1);
    while(objects[objects.size()-1].mType == NULL_OBJECT)
    {
         objects.pop_back();
         cullCounts[NULL_OBJECT]++;
    }

    uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count;)
    {
        if(objects[index].mPosition.x() < -1 || 
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
        SortObjectsByType(objects);
    }
}

void Animate(SceneObjectVector& objects,
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
                objects[index].mPosition += Vec2(ALIEN_SPEED * objects[index].mVelocity.x(), 0);
            }
        }
    }
}

void MoveObjects(SceneObjectVector& objects,
                 const float deltaTimeInSecs)
{
    const uint32_t count = objects.size();
    for(uint32_t index = FIRST_GENERIC_OBJECT; index < count; ++index)
    {
        objects[index].mPosition += objects[index].mVelocity * deltaTimeInSecs;
    }
}

void DrawObjects(SceneObjectVector& objects,
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
                   SceneObjectVector& objects)
{
    assert(type < NUM_OBJECT_TYPES);
    Vec2 accumPos = pos;
    for(uint32_t index = 0; index < count; ++index)
    {
        SceneObjectData newObject;
        newObject.mType = type;
        newObject.mPosition = accumPos;
        newObject.mVelocity = vel;
        objects.push_back(newObject);

        accumPos += deltaPos;
    }

    SortObjectsByType(objects);
}
