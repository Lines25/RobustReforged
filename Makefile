## Ancient commented code - may be useful idk
# - Lines, Apr 18 03:21 PM EEST 2026
# CXX = clang++
# CXXFLAGS = -O3 -march=x86-64-v2 -fPIC -shared -std=c++23
# SRC = $(wildcard reforged/src/*.cpp)
# OUT = reforged/build/libreforged.so
# INCLUDE = -Ireforged/include
# 
# .PHONY: all native managed clean
# 
# all: native managed publish
# 
# native:
# 	mkdir -p reforged/build
# 	@echo " CXX" $(SRC) "->" $(OUT)
# 	@$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) -o $(OUT)
# 
# managed:
# 	dotnet build Robust.Server/Robust.Server.csproj
# 
# publish: managed native
# 	dotnet publish
# 	# dotnet run --project Content.Packaging server --hybrid-acz --platform linux-x64 # For compilling ss14 cont
# 
# clean:
# 	rm -rf reforged/build
# 	dotnet clean Robust.Reforged/
## Ancient commented code - may be useful idk

SHELL := /bin/sh
.SUFFIXES:

ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
else
    UNAME_S := $(shell uname -s)
    PLATFORM := Linux
endif

ifeq ($(PLATFORM),Windows)
    MKDIR = if not exist $(subst /,\\,$(1)) mkdir $(subst /,\\,$(1))
    RMDIR = if exist $(subst /,\\,$(1)) rmdir /s /q $(subst /,\\,$(1))
    EXT_SO := .dll
    OMP_FLAG := -fopenmp
else
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)

	# I've tried adding MacOS as different target.. but..
	# Apple Clang is TOO strange tbh, fuck Apple, use Linux !
	# I use Arch btw
    EXT_SO := .so
    OMP_FLAG := -fopenmp
endif

CXX := clang++
CXXFLAGS := -O3 -march=x86-64-v2 $(OMP_FLAG) -fPIC -std=c++23
CPPFLAGS := -Ireforged/include

SRC_DIR := reforged/src
BUILD_DIR := reforged/build
OBJ_DIR := $(BUILD_DIR)/objs
TARGET := $(BUILD_DIR)/libreforged$(EXT_SO)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "  LD       $@ -> $@"
	@$(call MKDIR,$(@D))
	@$(CXX) $(CXXFLAGS) -shared $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "  CXX      $< -> $@"
	@$(call MKDIR,$(@D))
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@echo "  CLEAN    $(BUILD_DIR)"
	@$(call RMDIR,$(BUILD_DIR))
