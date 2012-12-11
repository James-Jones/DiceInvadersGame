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
                          float deltaTimeInSecs)
{
    IDiceInvaders::KeyStatus keys;
    system->getKeyStatus(keys);

    float move = deltaTimeInSecs * 160.0f;

    if (keys.right)
    {
        player->mPosition.moveX(move);
    }
    else if (keys.left)
    {
        player->mPosition.moveX(-move);
    }

    if(keys.fire)
    {
        //Fire rocket upwards from just above the player position.
        Vec2 velocity(0, -16);
        CreateObjects(ROCKET, 1, player->mPosition - Vec2(0, SPRITE_SIZE), velocity, objects);
    }
}

int APIENTRY WinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	LPSTR commandLine,
	int commandShow)
{
	DiceInvadersLib lib("DiceInvaders.dll");
	IDiceInvaders* system = lib.get();
    const uint32_t screenWidth = 640;
    const uint32_t screenHeight = 480;

    system->init(screenWidth, screenHeight);

    std::vector <CommonSceneObjectData> objects;
    objects.reserve(512);

    //Create the player first. Guaranteed to be at the first
    //index so no need to search for it.
    CreateObjects(PLAYER, 1, Vec2(screenWidth/2.0f, screenHeight), Vec2(0, 0), objects);

	ISprite* mSprites[NUM_OBJECT_TYPES];
    mSprites[ROCKET] = system->createSprite("data/rocket.bmp");
    mSprites[BOMB] = system->createSprite("data/bomb.bmp");
    mSprites[PLAYER] = system->createSprite("data/player.bmp");
    mSprites[ENEMY1] = system->createSprite("data/enemy1.bmp");
    mSprites[ENEMY2] = system->createSprite("data/enemy2.bmp");

	float lastTime = system->getElapsedTime();
	while (system->update())
	{
		float newTime = system->getElapsedTime();
        float deltaTimeInSecs = newTime - lastTime;
		float move = deltaTimeInSecs * 160.0f;
		lastTime = newTime;

        DrawObjects(objects,
            mSprites);

        MoveObjects(objects, deltaTimeInSecs);

        ProcessKeyboardInput(system,
            &objects[0],
            objects,
            deltaTimeInSecs);
	}

    for(uint32_t index = 0; index < NUM_OBJECT_TYPES; ++index)
    {
        mSprites[index]->destroy();
    }

	system->destroy();

	return 0;
}



