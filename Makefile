SRC	:= $(wildcard src/*.c)
MSRC	:= $(wildcard modules/msgpuck/*.c)

OBJ	:= $(SRC:src/%.c=build/%.o) 
MOBJ	:= $(MSRC:modules/msgpuck/%.c=build/%.o)

INCDIR	:= relative .
INCOPT	:= $(INCDIR:%=-I%)
TARGET	:= msgpuck
COPTS	:= -fPIC -MMD

all: build/dr/$(TARGET).so

clean:
	echo $(OBJ)
	rm -f build/*.o
	rm -f build/*.d
	rm -f build/dr/$(TARGET).so

test: build/dr/$(TARGET).so
	prove -r t

build/dr/$(TARGET).so: $(OBJ) $(MOBJ)
	gcc $(INCOPT) $(COPTS) -shared -o $@ $(OBJ) $(MOBJ)

$(OBJ): build/%.o: src/%.c
	gcc $(INCOPT) $(COPTS) -c -o $@ $<

$(MOBJ): build/%.o: modules/msgpuck/%.c
	gcc $(INCOPT) $(COPTS) -c -o $@ $<


$(OBJ): build/%.o: Makefile
build/msgpuck.o: Makefile
build/dr/$(TARGET).so: Makefile

include $(wildcard build/*.d)
