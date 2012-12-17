#define _CRTDBG_MAP_ALLOC
#define NOMINMAX
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

    GameState(int windowW, int windowH) : mWindowWidth(windowW),
        mWindowHeight(windowH),
        mPlayerScore(0),
        mPlayerLives(MaxLives),
        mFireKeyWasDown(0)
    {
    }

    int mWindowWidth;
    int mWindowHeight;
    int mPlayerScore;
    int mPlayerLives;
    float mLastTime;//Time values are in seconds.
    int mFloorLastTime;
    float mTimeOfLastFire;
    int mFireKeyWasDown;
    SceneObjectVector mObjects;
    ISprite* mSprites[NUM_OBJECT_TYPES];
};

void ProcessKeyboardInput(IDiceInvaders* system,
                          GameState& state,
                          const float deltaTimeInSecs)
{
    IDiceInvaders::KeyStatus keys;
    system->getKeyStatus(keys);

    const float move = deltaTimeInSecs * PLAYER_SPEED;

    assert(PLAYER == 0);//Assume player is first in vector;
    SceneObjectData* player = &state.mObjects[0];

    player->mPosition.moveX((keys.right * move) + (-move * keys.left));
    player->mPosition.clampX(0.0f, state.mWindowWidth-F_SPRITE_SIZE);

    const float currentTime = system->getElapsedTime();

    if(keys.fire)
    {
        if(!state.mFireKeyWasDown || 
            (currentTime-state.mTimeOfLastFire > ROCKET_RATE_OF_FIRE))
        {
            //Fire rocket upwards from just above the player position.
            Vec2 velocity(0.0f, -ROCKET_SPEED);
            CreateObjects(ROCKET, 1, player->mPosition - Vec2(0, SPRITE_SIZE/2), velocity, Vec2(0, 0), state.mObjects);

            state.mTimeOfLastFire = currentTime;
        }
    }
    state.mFireKeyWasDown = keys.fire;
}

void ResultScreen(IDiceInvaders* system,
                GameState& state)
{
    const int resultStringSize = 16+MAX_SCORE_DIGITS;
    char resultString[resultStringSize];
    if(sprintf_s(resultString, resultStringSize, "Final score is %d", state.mPlayerScore))
    {
        system->drawText(state.mWindowWidth/3, state.mWindowHeight/2, resultString);
    }
}

void GameScreen(IDiceInvaders* system,
                GameState& state)
{
    const float newTime = system->getElapsedTime();
    const float deltaTimeInSecs = newTime - state.mLastTime;
    const int iFloorNewTime = static_cast<int>(std::floor(newTime));

    {
        const int scoreStringSize = MAX_SCORE_DIGITS+8;
        char scoreString[scoreStringSize];
        if(sprintf_s(scoreString, scoreStringSize, "Score: %d", state.mPlayerScore))
        {
            system->drawText(0, state.mWindowHeight-SPRITE_SIZE, scoreString);
        }
    }
#if defined(SHOW_STATS)
    {
        const int debugInfoSize = 128;
        char debugInfo[debugInfoSize];

        //Average milliseconds per frame over a 1 second period.
        static float startTime = newTime;
        static float accumTime = 0;
        static int frame = 0;
        static float avgFrameTime = 0;

        if(accumTime > 1)//Reset approx each second
        {
            avgFrameTime = accumTime/frame;
            startTime = newTime;
            accumTime = 0;
            frame = 0;
        }

        frame++;
        accumTime += deltaTimeInSecs;

        if(sprintf_s(debugInfo, debugInfoSize, "%d objects; %.4f ms", state.mObjects.size(),
            avgFrameTime * 1000.0f))
        {
            system->drawText(0, state.mWindowHeight-64, debugInfo);
        }
    }
#endif

    state.mLastTime = newTime;

    DrawObjects(state.mObjects,
        state.mSprites);

    //Health. 1 player sprite for each life.
    for(int i=0; i<state.mPlayerLives; ++i)
    {
        const int x = state.mWindowWidth-(SPRITE_SIZE*GameState::MaxLives) + SPRITE_SIZE*i;
        const int y = state.mWindowHeight-SPRITE_SIZE;
        state.mSprites[PLAYER]->draw(x, y);
    }

    {
        Box mAlienBBox;//Bounding box of ALL aliens
        CalcAlienBBox(state.mObjects, mAlienBBox);

        bool hitLeft =  mAlienBBox.mLeft <= 0;
        bool hitRight = mAlienBBox.mRight >= (state.mWindowWidth);
        if(hitLeft || hitRight)
            AliensChangeDirection(state.mObjects, mAlienBBox, 0, state.mWindowWidth-F_SPRITE_SIZE-1.0f, deltaTimeInSecs);
    }

    MoveObjects(state.mObjects, deltaTimeInSecs);

    int cullCounts[NUM_OBJECT_TYPES];
    for(int i=0; i<NUM_OBJECT_TYPES;++i)
    {
        cullCounts[i] = 0;
    }
    CullObjects(state.mObjects, state.mWindowWidth, state.mWindowHeight-state.HudWidth, cullCounts);

    if(cullCounts[ENEMY1] || cullCounts[ENEMY2])
    {
        //Alien reached the bottom of the window
        state.mPlayerLives = 0;
    }

    Animate(state.mObjects, iFloorNewTime);

    int hitCounts[NUM_OBJECT_TYPES];
    for(int i=0; i<NUM_OBJECT_TYPES;++i)
    {
        hitCounts[i] = 0;
    }
    CollideObjects(state.mObjects, hitCounts);

    state.mPlayerScore += hitCounts[ENEMY1];
    state.mPlayerScore += hitCounts[ENEMY2];
    state.mPlayerLives -= hitCounts[PLAYER];

    state.mPlayerScore = std::min(state.mPlayerScore, MAX_SCORE);

    AliensRandomFire(state.mObjects, state.mFloorLastTime, iFloorNewTime);

    ProcessKeyboardInput(system,
        state,
        deltaTimeInSecs);

    //Check for no more aliens. The objects are sorted
    //so if the second object is not alien then there are none
    if(state.mObjects.size() > 1 && state.mObjects[1].mType > ENEMY2)
        SpawnAliens(state.mObjects, state.mWindowWidth);

    state.mFloorLastTime = iFloorNewTime;

    //Wait until the end to free all objects so preceding
    //code and safely assume there is at least 1 object in vector.
    if(!state.mPlayerLives)
    {
        state.mObjects.clear();
    }
}

void InitLevel(IDiceInvaders* system, GameState& gameState)
{
    gameState.mObjects.reserve(512);
    const float fWindowWidth = static_cast<float>(gameState.mWindowWidth);
    const float fWindowHeight = static_cast<float>(gameState.mWindowHeight);
    const float fHudWidth = static_cast<float>(gameState.HudWidth);

    //Create the player first. Guaranteed to be at the first
    //index so no need to search for it.
    CreateObjects(PLAYER, 1, Vec2(fWindowWidth/2.0f, fWindowHeight-fHudWidth), Vec2(0, 0), Vec2(0, 0), gameState.mObjects);

    gameState.mSprites[ROCKET] = system->createSprite("data/rocket.bmp");
    gameState.mSprites[BOMB] = system->createSprite("data/bomb.bmp");
    gameState.mSprites[PLAYER] = system->createSprite("data/player.bmp");
    gameState.mSprites[ENEMY1] = system->createSprite("data/enemy1.bmp");
    gameState.mSprites[ENEMY2] = system->createSprite("data/enemy2.bmp");
    gameState.mSprites[NULL_OBJECT] = system->createSprite("data/null.bmp");

    SpawnAliens(gameState.mObjects, gameState.mWindowWidth);

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

    const int windowWidth = GetSystemMetrics(SM_CXFULLSCREEN)/3*2;
    const int windowHeight = GetSystemMetrics(SM_CYFULLSCREEN)/3*2;

    if(system->init(windowWidth, windowHeight) == false)
    {
        return 0;
    }

    GameState gameState(windowWidth, windowHeight);

    InitLevel(system, gameState);

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



