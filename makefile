OUTPUT_NAME=ini.dll
C_FLAGS=-std=gnu11 -DDEBUG -g -shared
C_SOURCES=ini
RAW_LIBS=lua/lua53.dll

all:clear build

clear:
	if exist obj\*.o ( del obj\*.o )
	if not exist obj ( mkdir obj )
	if exist $(OUTPUT_NAME) ( del $(OUTPUT_NAME) )

build:$(C_SOURCES)
	gcc $(C_FLAGS) -o $(OUTPUT_NAME) obj/*.o $(RAW_LIBS)

$(C_SOURCES):
	gcc -c $(C_FLAGS) -o obj\$@.o $@.c

.PHONY:all clear build $(C_SOURCES)
