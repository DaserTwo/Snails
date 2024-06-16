SRC := $(wildcard src/*)
BIN := $(patsubst src/%, bin/%.o, $(SRC))
OUT := Snails

LIB := -Llib -lraylib
STD := -std=c++2a

.PHONY: all

all: $(OUT)

$(OUT): bin $(BIN)
	$(CXX) $(BIN) -o $@ $(STD) $(LIB)

Snails.exe: 
	x86_64-w64-mingw32-g++ -I include -o $@ $(SRC) raylib.dll -static

bin:
	@mkdir $@

$(BIN):bin/%.o:src/%
	$(CXX) -c -o $@ $^ $(STD) -Iinclude -DPLATFORM_DESKTOP
