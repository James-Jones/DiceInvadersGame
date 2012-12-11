#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <cassert>
#include <cstdio>
#include "SceneObject.h"

class DiceInvadersLib
{
public:
	explicit DiceInvadersLib(const char* libraryPath)
	{
		m_lib = LoadLibrary(libraryPath);
		assert(m_lib);

		DiceInvadersFactoryType* factory = (DiceInvadersFactoryType*)GetProcAddress(
				m_lib, "DiceInvadersFactory");
		m_interface = factory();
		assert(m_interface);
	}

	~DiceInvadersLib()
	{
		FreeLibrary(m_lib);
	}

	IDiceInvaders* get() const 
	{
		return m_interface;
	}

private: 
	DiceInvadersLib(const DiceInvadersLib&);
	DiceInvadersLib& operator=(const DiceInvadersLib&);

private: 
	IDiceInvaders* m_interface;
	HMODULE m_lib;
};

void ProcessKeyboardInput(IDiceInvaders* system,
                          CommonSceneObjectData* player,
                          std::vector <CommonSceneObjectData>& objects,
                          float deltaTimeInSecs,
                          float& timeOfLastFire,
                          int& wasDown)
{
    IDiceInvaders::KeyStatus keys;
    system->getKeyStatus(keys);

    float move = deltaTimeInSecs * 160.0f;

    if (keys.right)
    {
        player->mPosition.moveX(move);
        player->mPosition.clampX(0, 640-32);
    }
    else if (keys.left)
    {
        player->mPosition.moveX(-move);
        player->mPosition.clampX(0, 640-32);
    }

    const float fMaxRateOfFire = 0.2f;
    const float now = system->getElapsedTime();

    if(keys.fire && !wasDown && (now-timeOfLastFire >fMaxRateOfFire))
    {
        //Fire rocket upwards from just above the player position.
        Vec2 velocity(0, -64);
        CreateObjects(ROCKET, 1, player->mPosition - Vec2(0, SPRITE_SIZE/2), velocity, Vec2(0, 0), objects);

        timeOfLastFire = now;
    }
    wasDown = keys.fire;
}

int APIENTRY WinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	LPSTR commandLine,
	int commandShow)
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	DiceInvadersLib lib("DiceInvaders.dll");
	IDiceInvaders* system = lib.get();
    const uint32_t screenWidth = 640;
    const uint32_t screenHeight = 480;
    const uint32_t hudWidth = 32;

    system->init(screenWidth, screenHeight);

    std::vector <CommonSceneObjectData> objects;
    objects.reserve(5120);

    //Create the player first. Guaranteed to be at the first
    //index so no need to search for it.
    CreateObjects(PLAYER, 1, Vec2(screenWidth/2.0f, screenHeight-hudWidth), Vec2(0, 0), Vec2(0, 0), objects);

	ISprite* mSprites[NUM_OBJECT_TYPES];
    mSprites[ROCKET] = system->createSprite("data/rocket.bmp");
    mSprites[BOMB] = system->createSprite("data/bomb.bmp");
    mSprites[PLAYER] = system->createSprite("data/player.bmp");
    mSprites[ENEMY1] = system->createSprite("data/enemy1.bmp");
    mSprites[ENEMY2] = system->createSprite("data/enemy2.bmp");

	float lastTime = system->getElapsedTime();

    int frame = 0;

    float timeOfLastFire = lastTime;

    int wasDown = 0;

    //Health. 1 player for each life.
    CreateObjects(PLAYER, 3,
        Vec2(screenWidth-SPRITE_SIZE*3, screenHeight),
        Vec2(0, 0), Vec2(SPRITE_SIZE, 0), objects);


    CreateObjects(ENEMY1, screenWidth/SPRITE_SIZE,
        Vec2(0, 32),
        Vec2(0, 0), Vec2(SPRITE_SIZE, 0), objects);

    int score = 0;

	while (system->update())
	{
        char scoreString[8]; //Max score is 99999999
		float newTime = system->getElapsedTime();
        float deltaTimeInSecs = newTime - lastTime;
		float move = deltaTimeInSecs * 160.0f;
		lastTime = newTime;

        sprintf_s(scoreString, 8, "%d", score);

        system->drawText(0, screenHeight-SPRITE_SIZE, scoreString);

        DrawObjects(objects,
            mSprites);

        MoveObjects(objects, deltaTimeInSecs);

        Animate(objects, newTime);

        ProcessKeyboardInput(system,
            &objects[0],
            objects,
            deltaTimeInSecs,
            timeOfLastFire,
            wasDown);
	}

    for(uint32_t index = 0; index < NUM_OBJECT_TYPES; ++index)
    {
        mSprites[index]->destroy();
    }

	system->destroy();

	return 0;
}



