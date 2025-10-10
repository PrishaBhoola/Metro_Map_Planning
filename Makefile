# Makefile for Metro_Map_Planning

CXX := g++
CXXFLAGS := -std=c++11 -O2 -Wall
BIN_DIR := bin
SRC_DIR := src

RUN1 := $(BIN_DIR)/run1
RUN2 := $(BIN_DIR)/run2

.PHONY: all clean dirs run

all: dirs $(RUN1) $(RUN2)

$(RUN1): $(SRC_DIR)/run1.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(RUN2): $(SRC_DIR)/run2.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

dirs:
	@mkdir -p $(BIN_DIR)

clean:
	@echo Cleaning build artifacts and generated files
	@rm -f $(RUN1) $(RUN2)
	@rm -f *.satinput *.satoutput *.metromap *.vars
	@rm -rf $(BIN_DIR)