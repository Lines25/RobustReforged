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

CXX = clang++
CXXFLAGS = -O3 -march=x86-64-v2 -fPIC -shared -std=c++23
SRC = $(wildcard reforged/src/*.cpp)
OUT = reforged/build/libreforged.so
INCLUDE = -Ireforged/include

.PHONY: all clean

all:
	mkdir -p reforged/build
	@echo " CXX $(SRC) -> $(OUT)"
	@$(CXX) $(CXXFLAGS) $(INCLUDE) $(SRC) -o $(OUT)

clean:
	rm -rf reforged/build
