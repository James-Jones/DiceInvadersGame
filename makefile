DiceInvaders.exe: Core.cpp SceneObject.cpp
	cl /nologo /EHsc /W3 $** 

clean: dummy
	-@del DiceInvaders.exe
	-@del Core.obj
	-@del SceneObject.obj

dummy:
	
