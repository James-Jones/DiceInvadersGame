TARGET = DiceInvaders
LL=link.exe -nologo
CC=cl.exe -nologo
CFLAGS = /EHsc /W3
LIBS = /DEFAULTLIB:User32.lib

!IF "$(DEBUG)" == "1"
!message Building DEBUG version
CFLAGS = $(CFLAGS) /Zi /MTd
!ELSE
!message Building RELEASE version
CFLAGS = $(CFLAGS) /O2 /MT
!ENDIF

!IF "$(SHOW_STATS)" == "1"
CDEFINES = $(CDEFINES) -DSHOW_STATS
!ENDIF

SRC = Core.obj SceneObject.obj
all: clean $(TARGET).exe

# cpp -> obj
.cpp{$(OBJ)}.obj:
	$(CC) -c $(CDEFINES) $(CFLAGS) -Fo$@ $<

# Link the source components into the executable
$(TARGET).exe: $(SRC)
	$(LL) $(LFLAGS) $(LIBS) $(SRC) /OUT:$(TARGET).exe

clean: dummy
	-@del $(TARGET).exe
	-@del Core.obj
	-@del SceneObject.obj

dummy:
