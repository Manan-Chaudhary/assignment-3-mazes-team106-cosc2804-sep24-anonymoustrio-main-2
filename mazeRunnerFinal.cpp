#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <mcpp/mcpp.h>
#include "menuUtils.h"
#include "Maze.h"
#include "Agent.h"
#include <thread>
#include <chrono>

#define NORMAL_MODE 0
#define TESTING_MODE 1

enum States {
    ST_Main,
    ST_GetMaze,
    ST_BuildMaze,
    ST_SolveMaze,
    ST_Creators,
    ST_Exit
};

void readMazeFromTerminal(mcpp::MinecraftConnection& mc, bool mode);
void generateRandomMaze(mcpp::MinecraftConnection& mc, bool mode);
void buildMazeInMinecraft(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z);
void flattenTerrain(mcpp::MinecraftConnection& mc, int x, int y, int z, int length, int width);
bool isOdd(int number);
void recursiveDivision(std::vector<std::string>& maze, int x1, int y1, int x2, int y2, bool horizontal, bool mode);
bool isValidMazeStructure(const std::vector<std::string>& maze, int length, int width);
void solveMazeManually(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool mode);
void showEscapeRoute(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode);
void cleanWorld(mcpp::MinecraftConnection& mc);

std::vector<mcpp::Coordinate> changes; // To track changes in the Minecraft world

std::vector<std::string> currentMaze;
int mazeX = 0, mazeY = 0, mazeZ = 0;

int main(int argc, char* argv[]) {
    bool mode = NORMAL_MODE;
    if (argc > 1 && std::string(argv[1]) == "-testmode") {
        mode = TESTING_MODE;
    }

    mcpp::MinecraftConnection mc;
    mc.doCommand("time set day");

    if (mode == TESTING_MODE) {
        mc.doCommand("tp 4848 71 4369");
    }

    printStartText();

    States curState = ST_Main;

    while (curState != ST_Exit) {
        switch (curState) {
            case ST_Main: {
                printMainMenu();
                int choice;
                std::cin >> choice;
                if (std::cin.fail() || choice < 1 || choice > 5) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Input Error: Enter a number between 1 and 5 ...." << std::endl;
                } else {
                    switch (choice) {
                        case 1:
                            curState = ST_GetMaze;
                            break;
                        case 2:
                            curState = ST_BuildMaze;
                            break;
                        case 3:
                            curState = ST_SolveMaze;
                            break;
                        case 4:
                            curState = ST_Creators;
                            break;
                        case 5:
                            curState = ST_Exit;
                            break;
                    }
                }
                break;
            }
            case ST_GetMaze: {
                printGenerateMazeMenu();
                int choice;
                std::cin >> choice;
                if (std::cin.fail() || choice < 1 || choice > 3) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Input Error: Enter a number between 1 and 3 ...." << std::endl;
                } else {
                    switch (choice) {
                        case 1:
                            readMazeFromTerminal(mc, mode);
                            curState = ST_Main;
                            break;
                        case 2:
                            generateRandomMaze(mc, mode);
                            curState = ST_Main;
                            break;
                        case 3:
                            curState = ST_Main;
                            break;
                    }
                }
                break;
            }
            case ST_BuildMaze: {
                if (!currentMaze.empty()) {
                    buildMazeInMinecraft(mc, currentMaze, mazeX, mazeY, mazeZ);
                } else {
                    std::cout << "No maze available to build. Generate a maze first." << std::endl;
                }
                curState = ST_Main;
                break;
            }
            case ST_SolveMaze: {
                printSolveMazeMenu();
                int choice;
                std::cin >> choice;
                if (std::cin.fail() || choice < 1 || choice > 3) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Input Error: Enter a number between 1 and 3 ...." << std::endl;
                } else {
                    switch (choice) {
                        case 1:
                            solveMazeManually(mc, currentMaze, mazeX, mazeY, mazeZ, mode);
                            curState = ST_Main;
                            break;
                        case 2:
                            // Show escape route logic
                            if (!currentMaze.empty()) {
                                showEscapeRoute(mc, currentMaze, mazeX + currentMaze[0].size() - 1, mazeY, mazeZ + currentMaze.size() - 1, mode);
                            } else {
                                std::cout << "No maze available to solve. Generate a maze first." << std::endl;
                            }
                            curState = ST_Main;
                            break;
                        case 3:
                            curState = ST_Main;
                            break;
                    }
                }
                break;
            }
            case ST_Creators: {
                printTeamInfo();
                curState = ST_Main;
                break;
            }
            case ST_Exit: {
                cleanWorld(mc);
                break;
            }
        }
    }

    //printExitMessage();
    return EXIT_SUCCESS;
}

void readMazeFromTerminal(mcpp::MinecraftConnection& mc, bool mode) {
    std::string done;
    int x, y, z;

    std::cout << "In Minecraft, navigate to where you need the maze to be built in Minecraft and type - done: ";
    std::cin >> done;

    if (done == "done") {
        if (mode == TESTING_MODE) {
            x = 4848;
            y = 71;
            z = 4369;
        } else {
            // Get the player's current position
            mcpp::Coordinate position = mc.getPlayerPosition();
            x = position.x;
            y = mc.getHeight(position.x, position.z) + 1;
            z = position.z;
        }

        std::cout << "Enter the length and width of maze: ";
        int length, width;
        std::cin >> length >> width;

        if (!isOdd(length) || !isOdd(width)) {
            std::cout << "Length and width must be odd numbers." << std::endl;
            return;
        }

        std::cout << "Enter the maze structure:" << std::endl;
        std::vector<std::string> maze;
        std::string row;
        for (int i = 0; i < length; ++i) {
            std::cin >> row;
            maze.push_back(row);
        }

        if (!isValidMazeStructure(maze, length, width)) {
            std::cout << "Invalid maze structure." << std::endl;
            return;
        }

        std::cout << "Maze read successfully" << std::endl;
        std::cout << "**Printing Maze**" << std::endl;
        std::cout << "BasePoint: (" << x << ", " << y << ", " << z << ")" << std::endl;
        std::cout << "Structure:" << std::endl;
        for (const auto& line : maze) {
            std::cout << line << std::endl;
        }
        std::cout << "**End Printing Maze**" << std::endl;

        // Store the maze and base point for later building
        currentMaze = maze;
        mazeX = x;
        mazeY = y;
        mazeZ = z;
    }
}

void generateRandomMaze(mcpp::MinecraftConnection& mc, bool mode) {
    std::string done;
    int x, y, z;

    std::cout << "In Minecraft, navigate to where you need the maze to be built in Minecraft and type - done: ";
    std::cin >> done;

    if (done == "done") {
        if (mode == TESTING_MODE) {
            x = 4848;
            y = 71;
            z = 4369;
        } else {
            // Get the player's current position
            mcpp::Coordinate position = mc.getPlayerPosition();
            x = position.x;
            y = mc.getHeight(position.x, position.z) + 1;
            z = position.z;
        }

        std::cout << "Enter the length and width of maze: ";
        int length, width;
        std::cin >> length >> width;

        if (!isOdd(length) || !isOdd(width)) {
            std::cout << "Length and width must be odd numbers." << std::endl;
            return;
        }

        std::vector<std::string> maze(length, std::string(width, '.'));
        for (int i = 0; i < length; ++i) {
            maze[i][0] = 'x';
            maze[i][width - 1] = 'x';
        }
        for (int i = 0; i < width; ++i) {
            maze[0][i] = 'x';
            maze[length - 1][i] = 'x';
        }

        srand(mode == NORMAL_MODE ? time(NULL) : 0);
        recursiveDivision(maze, 1, 1, width - 2, length - 2, mode == TESTING_MODE ? true : rand() % 2 == 0, mode);

        // Ensure there's an exit at the bottom
        maze[length - 1][width - 2] = '.';

        std::cout << "Maze generated successfully" << std::endl;
        std::cout << "**Printing Maze**" << std::endl;
        std::cout << "BasePoint: (" << x << ", " << y << ", " << z << ")" << std::endl;
        std::cout << "Structure:" << std::endl;
        for (const auto& line : maze) {
            std::cout << line << std::endl;
        }
        std::cout << "**End Printing Maze**" << std::endl;

        // Store the maze and base point for later building
        currentMaze = maze;
        mazeX = x;
        mazeY = y;
        mazeZ = z;
    }
}


void buildMazeInMinecraft(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z) {
    int length = maze.size();
    int width = maze[0].size();

    flattenTerrain(mc, x, y, z, length, width);

    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < width; ++j) {
            if (maze[i][j] == 'x') {
                for (int k = 0; k < 3; ++k) {
                    mc.setBlock(mcpp::Coordinate(x + j, y + k, z + i), mcpp::Blocks::ACACIA_WOOD_PLANK);
                    changes.emplace_back(x + j, y + k, z + i);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        }
    }

    // Mark the exit with blue carpet
    for (int i = 0; i < width; ++i) {
        if (maze[length - 1][i] == '.') {
            mc.setBlock(mcpp::Coordinate(x + i, y + 1, z + length - 1), mcpp::Blocks::BLUE_CARPET);
            changes.emplace_back(x + i, y + 1, z + length - 1);
            break;
        }
    }

    std::cout << "Maze built successfully" << std::endl;
}

void flattenTerrain(mcpp::MinecraftConnection& mc, int x, int y, int z, int length, int width) {
    for (int i = -1; i <= length; ++i) {
        for (int j = -1; j <= width; ++j) {
            int height = mc.getHeight(x + j, z + i);
            if (height > y) {
                for (int k = height; k > y; --k) {
                    mc.setBlock(mcpp::Coordinate(x + j, k, z + i), mcpp::Blocks::AIR);
                    changes.emplace_back(x + j, k, z + i);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            } else if (height < y) {
                for (int k = height; k < y; ++k) {
                    mc.setBlock(mcpp::Coordinate(x + j, k + 1, z + i), mcpp::Blocks::GRASS); // Assuming you want to use grass block to fill
                    changes.emplace_back(x + j, k + 1, z + i);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        }
    }
}

bool isOdd(int number) {
    return number % 2 != 0;
}

void recursiveDivision(std::vector<std::string>& maze, int x1, int y1, int x2, int y2, bool horizontal, bool mode) {
    if (x2 - x1 < 2 || y2 - y1 < 2) return;

    bool divideHorizontally = mode == TESTING_MODE ? horizontal : rand() % 2 == 0;
    if (divideHorizontally) {
        int y = y1 + 2 * (rand() % ((y2 - y1) / 2)) + 1;
        for (int x = x1; x <= x2; ++x) {
            maze[y][x] = 'x';
        }
        int passage = x1 + 2 * (rand() % ((x2 - x1) / 2));
        maze[y][passage] = '.';

        recursiveDivision(maze, x1, y1, x2, y - 1, !divideHorizontally, mode);
        recursiveDivision(maze, x1, y + 1, x2, y2, !divideHorizontally, mode);
    } else {
        int x = x1 + 2 * (rand() % ((x2 - x1) / 2)) + 1;
        for (int y = y1; y <= y2; ++y) {
            maze[y][x] = 'x';
        }
        int passage = y1 + 2 * (rand() % ((y2 - y1) / 2));
        maze[passage][x] = '.';

        recursiveDivision(maze, x1, y1, x - 1, y2, !divideHorizontally, mode);
        recursiveDivision(maze, x + 1, y1, x2, y2, !divideHorizontally, mode);
    }
}

bool isValidMazeStructure(const std::vector<std::string>& maze, int length, int width) {
    if (maze.size() != length) return false;
    for (const auto& row : maze) {
        if (row.size() != width) return false;
        for (char c : row) {
            if (c != 'x' && c != '.') return false;
        }
    }
    return true;
}

void solveMazeManually(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool mode) {
    if (maze.empty()) {
        std::cout << "No maze available to solve. Generate a maze first." << std::endl;
        return;
    }

    int length = maze.size();
    int width = maze[0].size();
    mcpp::Coordinate playerPosition;

    if (mode == TESTING_MODE) {
        // Place the player in the empty cell furthest from the base point (lower-right edge of the maze)
        for (int i = length - 1; i >= 0; --i) {
            for (int j = width - 1; j >= 0; --j) {
                if (maze[i][j] == '.') {
                    playerPosition = mcpp::Coordinate(x + j, y, z + i);
                    break;
                }
            }
        }
    } else {
        // Place the player in a random empty cell within the maze
        std::vector<mcpp::Coordinate> emptyCells;
        for (int i = 0; i < length; ++i) {
            for (int j = 0; j < width; ++j) {
                if (maze[i][j] == '.') {
                    emptyCells.emplace_back(x + j, y, z + i);
                }
            }
        }
        if (!emptyCells.empty()) {
            srand(time(NULL));
            playerPosition = emptyCells[rand() % emptyCells.size()];
        }
    }

    mc.doCommand("tp " + std::to_string(playerPosition.x) + " " + std::to_string(playerPosition.y) + " " + std::to_string(playerPosition.z));
    std::cout << "Player teleported to the maze at (" << playerPosition.x << ", " << playerPosition.y << ", " << playerPosition.z << ")." << std::endl;
}
void showEscapeRoute(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode) {
    if (maze.empty()) return;

    std::cout << "Showing escape route..." << std::endl;

    mcpp::Coordinate position = mc.getPlayerPosition();
    int px = position.x - baseX;
    int py = baseY;
    int pz = position.z - baseZ;

    // Initialize direction (0: North, 1: East, 2: South, 3: West)
    int direction = 0;
    bool escapeFound = false;

    // Lambda function to check if a move is valid
    auto isValidMove = [&](int x, int z) {
        auto block = mc.getBlock(mcpp::Coordinate(x + baseX, py, z + baseZ));
        return block == mcpp::Blocks::AIR || block == mcpp::Blocks::BLUE_CARPET;
    };

    // Track the previous carpet position to remove it
    mcpp::Coordinate prevCarpetPosition = position;

    while (!escapeFound) {
        // Place the lime carpet at the ground level (y-coordinate)
        mc.setBlock(mcpp::Coordinate(px + baseX, py, pz + baseZ), mcpp::Blocks::LIME_CARPET);

        // Remove the previous lime carpet
        if (!(prevCarpetPosition.x == px + baseX && prevCarpetPosition.z == pz + baseZ)) {
            mc.setBlock(prevCarpetPosition, mcpp::Blocks::AIR);
        }

        // Update the previous carpet position
        prevCarpetPosition = mcpp::Coordinate(px + baseX, py, pz + baseZ);

        // Calculate potential moves
        int leftDir = (direction + 3) % 4;
        int forwardX = px, forwardZ = pz;
        int leftX = px, leftZ = pz;

        // Determine coordinates for the left move
        if (leftDir == 0) {
            leftZ -= 1;
        } else if (leftDir == 1) {
            leftX += 1;
        } else if (leftDir == 2) {
            leftZ += 1;
        } else if (leftDir == 3) {
            leftX -= 1;
        }

        // Attempt to move left
        if (isValidMove(leftX, leftZ)) {
            direction = leftDir;
            px = leftX;
            pz = leftZ;
        } else {
            // Determine coordinates for the forward move
            if (direction == 0) {
                forwardZ -= 1;
            } else if (direction == 1) {
                forwardX += 1;
            } else if (direction == 2) {
                forwardZ += 1;
            } else if (direction == 3) {
                forwardX -= 1;
            }
            if (isValidMove(forwardX, forwardZ)) {
                px = forwardX;
                pz = forwardZ;
            } else {
                // Turn right
                direction = (direction + 1) % 4;
            }
        }

        // Update position and print the step
        position.x = px + baseX;
        position.z = pz + baseZ;
        std::cout << "Step: (" << position.x << ", " << py << ", " << position.z << ")" << std::endl;

        // Check if the escape route is found
        if (mc.getBlock(mcpp::Coordinate(px + baseX, py, pz + baseZ)) == mcpp::Blocks::BLUE_CARPET) {
            escapeFound = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Increase delay for visibility
    }

    std::cout << "Escape route found!" << std::endl;
}

void cleanWorld(mcpp::MinecraftConnection& mc) {
    

    for (const auto& coord : changes) {
        mc.setBlock(mcpp::Coordinate(coord.x, coord.y, coord.z), mcpp::Blocks::AIR);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    changes.clear();
    
}