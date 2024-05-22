SRC := $(wildcard src/*)
BIN := $(patsubst src/%, bin/%.o, $(SRC))
OUT := Snails

LIB := -Llib -lraylib
STD := -std=c++2a

.PHONY: all

all: $(OUT)

$(OUT): bin $(BIN)
	$(CXX) $(BIN) -o $@ $(STD) $(LIB)

bin:
	@mkdir $@

$(BIN):bin/%.o:src/%
	$(CXX) -c -o $@ $^ $(STD) -Iinclude -DPLATFORM_DESKTOP
