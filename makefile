OUT_DIR = out
SRC_DIR = src

CC = gcc -lm -O3 -fomit-frame-pointer -pipe
CPP = g++ -O3 -fomit-frame-pointer -pipe

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OUT_FILES = $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%,$(SRC_FILES))
CPP_SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OUT_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OUT_DIR)/%,$(CPP_SRC_FILES))

$(OUT_DIR)/%: $(SRC_DIR)/%.c
	$(CC) -o $@ $<

$(OUT_DIR)/%: $(SRC_DIR)/%.cpp
	$(CPP) -o $@ $<

all: $(OUT_FILES) $(CPP_OUT_FILES)

clean:
	rm -rf $(OUT_DIR)
	mkdir $(OUT_DIR)
