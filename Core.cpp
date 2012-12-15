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

struct GameState
{
    static const int HudWidth = 32;
    static const int MaxLives = 3;

    GameState(int screenW, int screenH) : mScreenWidth(screenW),
        mScreenHeight(screenH),
        mPlayerScore(0),
        mPlayerLives(MaxLives),
        mFireKeyWasDown(0)
    {
    }

    int mScreenWidth;
    int mScreenHeight;
    int mPlayerScore;
    int mPlayerLives;
    float mLastTime;//Seconds
    int mFloorLastTime;
    float mTimeOfLastFire;
    int mFireKeyWasDown;
    std::vector <CommonSceneObjectData> mObjects;
    ISprite* mSprites[NUM_OBJECT_TYPES];
};

void ProcessKeyboardInput(IDiceInvaders* system,
                          CommonSceneObjectData* player,
                          std::vector <CommonSceneObjectData>& objects,
                          float deltaTimeInSecs,
                          float& timeOfLastFire,
                          int& fireKeyWasDown)
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

    if(keys.fire && !fireKeyWasDown && (now-timeOfLastFire >fMaxRateOfFire))
    {
        //Fire rocket upwards from just above the player position.
        Vec2 velocity(0.0f, -640.0f);
        CreateObjects(ROCKET, 1, player->mPosition - Vec2(0, SPRITE_SIZE/2), velocity, Vec2(0, 0), objects);

        timeOfLastFire = now;
    }
    fireKeyWasDown = keys.fire;
}

void ResultScreen(IDiceInvaders* system,
                GameState& state)
{
    char resultString[256];
    sprintf_s(resultString, 256, "Final score is %d", state.mPlayerScore);
    system->drawText(state.mScreenWidth/3, state.mScreenHeight/2, resultString);
}

void GameScreen(IDiceInvaders* system,
                GameState& state)
{
    const float newTime = system->getElapsedTime();
    const float deltaTimeInSecs = newTime - state.mLastTime;
    const int iFloorNewTime = static_cast<int>(std::floor(newTime));

    {
        char scoreString[8]; //Max score is 99999999
        sprintf_s(scoreString, 8, "%d", state.mPlayerScore);
        system->drawText(0, state.mScreenHeight-SPRITE_SIZE, scoreString);
    }
#if defined(SHOW_STATS)
    {
        char debugInfo[512];
        sprintf_s(debugInfo, 512, "%d objects; %.4f ms", state.mObjects.size(),
            deltaTimeInSecs * 1000.0f);
        system->drawText(0, state.mScreenHeight-64, debugInfo);
    }
#endif

    state.mLastTime = newTime;

    DrawObjects(state.mObjects,
        state.mSprites);

    //Health. 1 player sprite for each life.
    for(int i=0; i<state.mPlayerLives; ++i)
    {
        const int x = state.mScreenWidth-(SPRITE_SIZE*GameState::MaxLives) + SPRITE_SIZE*i;
        const int y = state.mScreenHeight-SPRITE_SIZE;
        state.mSprites[PLAYER]->draw(x, y);
    }

    MoveObjects(state.mObjects, deltaTimeInSecs);

    CullObjects(state.mObjects, state.mScreenWidth, state.mScreenHeight-state.HudWidth);

    Animate(state.mObjects, static_cast<int>(std::floor(newTime)));

    int hitCounts[NUM_OBJECT_TYPES];
    for(int i=0; i<NUM_OBJECT_TYPES;++i)
    {
        hitCounts[i] = 0;
    }
    CollideObjects(state.mObjects, hitCounts);

    state.mPlayerScore += hitCounts[ENEMY1];
    state.mPlayerScore += hitCounts[ENEMY2];
    state.mPlayerLives -= hitCounts[PLAYER];

    AliensRandomFire(state.mObjects, state.mFloorLastTime, iFloorNewTime);

    ProcessKeyboardInput(system,
        &state.mObjects[0],
        state.mObjects,
        deltaTimeInSecs,
        state.mTimeOfLastFire,
        state.mFireKeyWasDown);

    state.mFloorLastTime = iFloorNewTime;

    //Wait until the end to free all objects so preceding
    //code and safely assume there is at least 1 object in vector.
    if(!state.mPlayerLives)
    {
        state.mObjects.clear();
    }
}

void InitGameState(IDiceInvaders* system, GameState& gameState)
{
    gameState.mObjects.reserve(512);
    const float fScreenWidth = static_cast<float>(gameState.mScreenWidth);
    const float fScreenHeight = static_cast<float>(gameState.mScreenHeight);
    const float fHudWidth = static_cast<float>(gameState.HudWidth);

    //Create the player first. Guaranteed to be at the first
    //index so no need to search for it.
    CreateObjects(PLAYER, 1, Vec2(fScreenWidth/2.0f, fScreenHeight-fHudWidth), Vec2(0, 0), Vec2(0, 0), gameState.mObjects);

    gameState.mSprites[ROCKET] = system->createSprite("data/rocket.bmp");
    gameState.mSprites[BOMB] = system->createSprite("data/bomb.bmp");
    gameState.mSprites[PLAYER] = system->createSprite("data/player.bmp");
    gameState.mSprites[ENEMY1] = system->createSprite("data/enemy1.bmp");
    gameState.mSprites[ENEMY2] = system->createSprite("data/enemy2.bmp");
    gameState.mSprites[NULL_OBJECT] = system->createSprite("data/null.bmp");

    for(int i =0; i < 8; ++i)
    {
        //One row of aliens.
        CreateObjects(ENEMY1, static_cast<uint32_t>(std::floor(fScreenWidth/F_SPRITE_SIZE*.75f)),
            Vec2(0.0f, F_SPRITE_SIZE * i),
            Vec2(0.0f, 0.0f), Vec2(F_SPRITE_SIZE, 0.0f), gameState.mObjects);
    }

    gameState.mLastTime = system->getElapsedTime();
    gameState.mTimeOfLastFire = gameState.mLastTime;
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

    if(system->init(screenWidth, screenHeight) == false)
    {
        return 0;
    }

    GameState gameState(screenWidth, screenHeight);

    InitGameState(system, gameState);

    bool bSystemOK = system->update();

    while(bSystemOK && gameState.mPlayerLives)
    {
        GameScreen(system, gameState);
        bSystemOK = system->update();
    }

	while (bSystemOK)
	{
        //Game has ended. Window has not been closed. Show final score
        ResultScreen(system, gameState);
        bSystemOK = system->update();
	}

    for(uint32_t index = 0; index < NUM_OBJECT_TYPES; ++index)
    {
        gameState.mSprites[index]->destroy();
    }

	system->destroy();

	return 0;
}



