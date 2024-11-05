#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include <mcpp/mcpp.h>

// Constants for modes
const bool TESTING_MODE = true;
const bool NORMAL_MODE = false;

// Variables to store current maze and its base coordinates
extern std::vector<std::string> currentMaze;
extern int mazeX, mazeY, mazeZ;
extern std::vector<mcpp::Coordinate> changes;

// Function declarations
void readMazeFromTerminal(mcpp::MinecraftConnection& mc, bool mode);
void generateRandomMaze(mcpp::MinecraftConnection& mc, bool mode);
void buildMazeInMinecraft(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool avoidObstacles, std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks);
void flattenTerrain(mcpp::MinecraftConnection& mc, int x, int y, int z, int length, int width,std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks);
bool isOdd(int number);
void recursiveDivision(std::vector<std::string>& maze, int x1, int y1, int x2, int y2, bool horizontal, bool mode);
bool isValidMazeStructure(const std::vector<std::string>& maze, int length, int width);
void solveMazeManually(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool mode);
void showEscapeRoute(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode);
void cleanWorld(mcpp::MinecraftConnection& mc, const std::vector<mcpp::Coordinate>& changes);
void storeBlocks(mcpp::MinecraftConnection& mc, const std::vector<mcpp::Coordinate>& changes, std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks);
void revertBlocks(mcpp::MinecraftConnection& mc, const std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks);
void showEscapeRouteBFS(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode);
//void showEscapeRouteFinal(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode);
//std::vector<std::pair<int, int>> find_path_in_maze(const std::vector<std::string>& maze, const std::pair<int, int>& start, const std::pair<int, int>& exit);
void showEscapeRouteFinal(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode, bool showEscapeWithBFS);
// Add the avoidObstaclesAndBuildMaze function declaration
void avoidObstaclesAndBuildMaze(mcpp::MinecraftConnection& mc, std::vector<std::string>& maze, int x, int y, int z);

#endif // MAZE_H
