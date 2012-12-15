#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <cassert>
#include <cstdio>
#include <cmath>

#include "SceneObject.h"

class DiceInvadersLib
{
public:
	explicit DiceInvadersLib(const char* libraryPath)
	{
		m_lib = LoadLibrary(libraryPath);
		assert(m_lib);

		DiceInvadersFactoryType* factory = reinterpret_cast<DiceInvadersFactoryType*>(GetProcAddress(
				m_lib, "DiceInvadersFactory"));
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

    const float move = deltaTimeInSecs * 160.0f;

    if (keys.right)
    {
        player->mPosition.moveX(move);
        player->mPosition.clampX(0.0f, 640.0f-F_SPRITE_SIZE);
    }
    else if (keys.left)
    {
        player->mPosition.moveX(-move);
        player->mPosition.clampX(0.0f, 640.0f-F_SPRITE_SIZE);
    }

    const float fMaxRateOfFire = 0.2f;
    const float now = system->getElapsedTime();

    if(keys.fire && !wasDown && (now-timeOfLastFire >fMaxRateOfFire))
    {
        //Fire rocket upwards from just above the player position.
        Vec2 velocity(0.0f, -640.0f);
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
	IDiceInvaders * const system = lib.get();
    const int screenWidth = 640;
    const int screenHeight = 480;
    const int hudWidth = 32;
    const float fScreenWidth = 640.0f;
    const float fScreenHeight = 480.0f;
    const float fHudWidth = 32.0f;

    if(system->init(screenWidth, screenHeight) == false)
    {
        return 0;
    }

    std::vector <CommonSceneObjectData> objects;
    objects.reserve(512);

    //Create the player first. Guaranteed to be at the first
    //index so no need to search for it.
    CreateObjects(PLAYER, 1, Vec2(fScreenWidth/2.0f, fScreenHeight-fHudWidth), Vec2(0, 0), Vec2(0, 0), objects);

	ISprite* mSprites[NUM_OBJECT_TYPES];
    mSprites[ROCKET] = system->createSprite("data/rocket.bmp");
    mSprites[BOMB] = system->createSprite("data/bomb.bmp");
    mSprites[PLAYER] = system->createSprite("data/player.bmp");
    mSprites[ENEMY1] = system->createSprite("data/enemy1.bmp");
    mSprites[ENEMY2] = system->createSprite("data/enemy2.bmp");
    mSprites[NULL_OBJECT] = system->createSprite("data/null.bmp");

    const int MaxLives = 3;

    //One row of aliens.
    CreateObjects(ENEMY1, static_cast<uint32_t>(std::floor(fScreenWidth/F_SPRITE_SIZE)),
        Vec2(0.0f, F_SPRITE_SIZE),
        Vec2(0.0f, 0.0f), Vec2(F_SPRITE_SIZE, 0.0f), objects);

    int score = 0;
    float lastTime = system->getElapsedTime();
    float timeOfLastFire = lastTime;
    int wasDown = 0;
    int lives = MaxLives;

	while (system->update())
	{
        char scoreString[8]; //Max score is 99999999
		const float newTime = system->getElapsedTime();
        const float deltaTimeInSecs = newTime - lastTime;

		lastTime = newTime;

        sprintf_s(scoreString, 8, "%d", score);

        system->drawText(0, screenHeight-SPRITE_SIZE, scoreString);

#if defined(SHOW_STATS)
        char debugInfo[512];
        sprintf_s(debugInfo, 512, "%d objects; %.4f ms", objects.size(),
            deltaTimeInSecs * 1000.0f);
        system->drawText(0, screenHeight-64, debugInfo);
#endif

        DrawObjects(objects,
            mSprites);

        //Health. 1 player sprite for each life.
        for(int i=0; i<lives; ++i)
        {
            const int x = screenWidth-(SPRITE_SIZE*MaxLives) + SPRITE_SIZE*i;
            const int y = screenHeight-SPRITE_SIZE;
            mSprites[PLAYER]->draw(x, y);
        }

        MoveObjects(objects, deltaTimeInSecs);

        CullObjects(objects, screenWidth, screenHeight-hudWidth);

        Animate(objects, static_cast<int>(std::floor(newTime)));

        int hitCounts[NUM_OBJECT_TYPES];
        for(int i=0; i<NUM_OBJECT_TYPES;++i)
        {
            hitCounts[i] = 0;
        }
        CollideObjects(objects, hitCounts);

        score += hitCounts[ENEMY1];
        score += hitCounts[ENEMY2];
        lives -= hitCounts[PLAYER];

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



