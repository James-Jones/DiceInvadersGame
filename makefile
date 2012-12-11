DiceInvaders.exe: Core.cpp SceneObject.cpp
	cl /nologo $** 

clean: dummy
	-@del DiceInvaders.exe
	-@del Core.obj
	-@del SceneObject.obj

dummy:
	
