#include "Maze.h"
#include <queue>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <utility> // for std::pair

#include <unordered_set>

#include <algorithm>

std::vector<std::string> currentMaze; // Define the currentMaze variable
int mazeX, mazeY, mazeZ;              // Define the mazeX, mazeY, and mazeZ variables
std::vector<mcpp::Coordinate> changes; // To track changes in the Minecraft world
std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>> removedBlocks; // Declare removedBlocks
std::vector<mcpp::Coordinate> mazeChanges; // Dedicated vector for maze changes
// Function to build maze in Minecraft
void buildMazeInMinecraft(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool avoidObstacles, std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks) {
    if (avoidObstacles) {
        avoidObstaclesAndBuildMaze(mc, const_cast<std::vector<std::string>&>(maze), x, y, z);
    } else {
        flattenTerrain(mc, x, y, z, maze.size(), maze[0].size(), storedBlocks);
        for (int i = 0; i < maze.size(); ++i) {
            for (int j = 0; j < maze[0].size(); ++j) {
                if (maze[i][j] == 'x') {
                    for (int k = 0; k < 4; ++k) {
                        mcpp::Coordinate coord(x + j, y + k, z + i);
                        changes.emplace_back(coord); // Track changes
                        mc.setBlock(coord, mcpp::Blocks::ACACIA_WOOD_PLANK);
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                }
            }
        }
        for (int i = 0; i < maze[0].size(); ++i) {
            if (maze[maze.size() - 1][i] == '.') {
                mcpp::Coordinate coord(x + i, y + 1, z + maze.size() - 1);
                changes.emplace_back(coord); // Track changes
                mc.setBlock(coord, mcpp::Blocks::BLUE_CARPET);
                break;
            }
        }
    }
    std::cout << "Maze built successfully" << std::endl;
}



// Avoid obstacles method
void avoidObstaclesAndBuildMaze(mcpp::MinecraftConnection& mc, std::vector<std::string>& maze, int x, int y, int z) {
    int length = maze.size();
    int width = maze[0].size();
    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < width; ++j) {
            int currentHeight = mc.getHeight(x + j, z + i);
            if (maze[i][j] == 'x') {
                for (int k = 0; k < 3; ++k) {
                    if (mc.getBlock(mcpp::Coordinate(x + j, currentHeight + k, z + i)) == mcpp::Blocks::AIR) {
                        mc.setBlock(mcpp::Coordinate(x + j, currentHeight + k, z + i), mcpp::Blocks::ACACIA_WOOD_PLANK);
                        changes.emplace_back(mcpp::Coordinate(x + j, currentHeight + k, z + i)); // Track changes
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                }
            } else if (maze[i][j] == '.') {
                int nextHeight = mc.getHeight(x + j, z + i);
                if (std::abs(nextHeight - currentHeight) > 1) {
                    maze[i][j] = 'x';
                } else {
                    for (int k = 0; k < 3; ++k) {
                        if (k < nextHeight - y) {
                            mc.setBlock(mcpp::Coordinate(x + j, y + k, z + i), mcpp::Blocks::GRASS);
                        } else {
                            mc.setBlock(mcpp::Coordinate(x + j, y + k, z + i), mcpp::Blocks::AIR);
                        }
                        changes.emplace_back(mcpp::Coordinate(x + j, y + k, z + i)); // Track changes
                    }
                }
            }
        }
    }
    for (int i = 0; i < width; ++i) {
        if (maze[length - 1][i] == '.') {
            int height = mc.getHeight(x + i, z + length - 1);
            mc.setBlock(mcpp::Coordinate(x + i, height + 1, z + length - 1), mcpp::Blocks::BLUE_CARPET);
            changes.emplace_back(mcpp::Coordinate(x + i, height + 1, z + length - 1)); // Track changes
            break;
        }
    }
    std::cout << "Maze built successfully" << std::endl;
}

// Function to flatten terrain
void flattenTerrain(mcpp::MinecraftConnection& mc, int x, int y, int z, int length, int width,std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks) {
    for (int i = -1; i <= length; ++i) {
        for (int j = -1; j <= width; ++j) {
            int height = mc.getHeight(x + j, z + i);
            if (height > y) {
                for (int k = height; k > y; --k) {
                    mcpp::Coordinate coord(x + j, k, z + i);

                    mazeChanges.emplace_back(coord);
                    storeBlocks(mc, mazeChanges, storedBlocks); // Store block before changing it
                    mazeChanges.clear(); // Clear changes after storing
                    changes.emplace_back(mcpp::Coordinate(x + j, k, z + i));
                    mc.setBlock(mcpp::Coordinate(x + j, k, z + i), mcpp::Blocks::AIR);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            } else if (height < y) {
                for (int k = height; k < y; ++k) {
                    mcpp::Coordinate coord(x + j, k + 1, z + i);
                    
                    mazeChanges.emplace_back(coord);
                    storeBlocks(mc, mazeChanges, storedBlocks); // Store block before changing it
                    mazeChanges.clear(); // Clear changes after storing
                    changes.emplace_back(mcpp::Coordinate(x + j, k + 1, z + i));
                    mc.setBlock(mcpp::Coordinate(x + j, k + 1, z + i), mcpp::Blocks::GRASS);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        }
    }
}

// Function to read maze from terminal
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
        std::cout << "Enter the maze structure (x for wall, . for path):" << std::endl;
        std::vector<std::string> maze(length);
        for (int i = 0; i < length; ++i) {
            std::cin >> maze[i];
        }
        if (!isValidMazeStructure(maze, length, width)) {
            std::cout << "Invalid maze structure." << std::endl;
            return;
        }
        std::cout << "Maze read successfully" << std::endl;
        std::cout << "**Printing Maze**" << std::endl;
        std::cout << "BasePoint: (" << x << ", " << y << ", " << z << ")" << std::endl;
        for (const auto& line : maze) {
            std::cout << line << std::endl;
        }
        std::cout << "**End Printing Maze**" << std::endl;
        currentMaze = maze;
        mazeX = x;
        mazeY = y;
        mazeZ = z;
    }
}

// Function to generate random maze
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
        maze[length - 1][width - 2] = '.';
        std::cout << "Maze generated successfully" << std::endl;
        std::cout << "**Printing Maze**" << std::endl;
        std::cout << "BasePoint: (" << x << ", " << y << ", " << z << ")" << std::endl;
        for (const auto& line : maze) {
            std::cout << line << std::endl;
        }
        std::cout << "**End Printing Maze**" << std::endl;
        currentMaze = maze;
        mazeX = x;
        mazeY = y;
        mazeZ = z;
    }
}

// Function to check if a number is odd
bool isOdd(int number) {
    return number % 2 != 0;
}

// Recursive division algorithm for maze generation
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

// Function to validate maze structure
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

// Function to solve maze manually
void solveMazeManually(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int x, int y, int z, bool mode) {
    if (maze.empty()) {
        std::cout << "No maze available to solve. Generate a maze first." << std::endl;
        return;
    }
    int length = maze.size();
    int width = maze[0].size();
    mcpp::Coordinate playerPosition;
    if (mode == TESTING_MODE) {
        for (int i = length - 1; i >= 0; --i) {
            for (int j = width - 1; j >= 0; --j) {
                if (maze[i][j] == '.') {
                    playerPosition = mcpp::Coordinate(x + j, y, z + i);
                    break;
                }
            }
        }
    } else {
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

// Function to show escape route
void showEscapeRoute(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode) {
    if (maze.empty()) return;

    std::cout << "Showing escape route..." << std::endl;

    mcpp::Coordinate position = mc.getPlayerPosition();
    int px = position.x - baseX;
    int py = mc.getHeight(position.x, position.z);
    int pz = position.z - baseZ;

    // Initialize direction (0: North, 1: East, 2: South, 3: West)
    int direction = 0;
    bool escapeFound = false;

    // Lambda function to check if a move is valid
    auto isValidMove = [&](int x, int z) {
        auto block = mc.getBlock(mcpp::Coordinate(x + baseX, py + 1, z + baseZ));
        return block == mcpp::Blocks::AIR || block == mcpp::Blocks::BLUE_CARPET;
    };

    // Track the previous carpet position to remove it
    mcpp::Coordinate prevCarpetPosition = position;

    while (!escapeFound) {
        // Place the lime carpet at the ground level (y-coordinate)
        mc.setBlock(mcpp::Coordinate(px + baseX, py + 1, pz + baseZ), mcpp::Blocks::LIME_CARPET);

        // Remove the previous lime carpet
        if (!(prevCarpetPosition.x == px + baseX && prevCarpetPosition.z == pz + baseZ)) {
            mc.setBlock(prevCarpetPosition, mcpp::Blocks::AIR);
        }

        // Update the previous carpet position
        prevCarpetPosition = mcpp::Coordinate(px + baseX, py + 1, pz + baseZ);

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
        std::cout << "Step: (" << position.x << ", " << py + 1 << ", " << position.z << ")" << std::endl;

        // Check if the escape route is found
        if (mc.getBlock(mcpp::Coordinate(px + baseX, py + 1, pz + baseZ)) == mcpp::Blocks::BLUE_CARPET) {
            escapeFound = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Increase delay for visibility
    }

    std::cout << "Escape route found!" << std::endl;
}

// Function to clean the Minecraft world
// Function to clean the Minecraft world
void cleanWorld(mcpp::MinecraftConnection& mc, const std::vector<mcpp::Coordinate>& changes) {
    for (const auto& coord : changes) {
        mc.setBlock(coord, mcpp::Blocks::AIR);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small delay to prevent server overload
    }
    std::cout << "World cleaned successfully" << std::endl;
}

// Function to remove blocks and store them
void revertBlocks(mcpp::MinecraftConnection& mc, const std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks) {
    for (const auto& pair : storedBlocks) {
        const auto& coord = pair.first;
        const auto& blockType = pair.second;
        mc.setBlock(coord, blockType);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small delay to prevent server overload
    }
    std::cout << "Blocks reverted successfully" << std::endl;
}



// Function to rebuild the maze from stored removed blocks
void storeBlocks(mcpp::MinecraftConnection& mc, const std::vector<mcpp::Coordinate>& changes, std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>>& storedBlocks) {
    for (const auto& coord : changes) {
        mcpp::BlockType blockType = mc.getBlock(coord);
        storedBlocks.emplace_back(coord, blockType);
    }
}






#include <functional> // For std::hash

// namespace mcpp {
//     // Assuming the Coordinate class is already defined in mcpp/util.h and has the necessary operators.
//     class Coordinate {
//     public:
//         explicit Coordinate(int x = 0, int y = 0, int z = 0);
//         Coordinate(double x, double y, double z);
        
//         Coordinate operator+(const Coordinate& obj) const;
//         bool operator==(const Coordinate& obj) const;
//         Coordinate operator-(const Coordinate& obj) const;
//         [[nodiscard]] Coordinate clone() const;

//         friend std::ostream& operator<<(std::ostream& out, const Coordinate& coord);

//         int x;
//         int y;
//         int z;
//     };
// }

// Define the hash function for the Coordinate class
namespace std {
    template <>
    struct hash<mcpp::Coordinate> {
        size_t operator()(const mcpp::Coordinate& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1) ^ (std::hash<int>()(coord.z) << 2);
        }
    };
}

// Function to show the escape route
void showEscapeRouteBFS(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode) {
    if (maze.empty()) return;

    std::cout << "Showing escape route..." << std::endl;

    mcpp::Coordinate startPosition = mc.getPlayerPosition();
    int startX = startPosition.x - baseX;
    int startY = mc.getHeight(startPosition.x, startPosition.z);
    int startZ = startPosition.z - baseZ;

    // Data structure for storing visited coordinates using custom hash function
    std::unordered_set<mcpp::Coordinate, std::hash<mcpp::Coordinate>> visited;

    // Lambda function to check if a move is valid
    auto isValidMove = [&](int x, int z) {
        auto block = mc.getBlock(mcpp::Coordinate(x + baseX, startY + 1, z + baseZ));
        return block == mcpp::Blocks::AIR || block == mcpp::Blocks::BLUE_CARPET;
    };

    // Lambda function to get valid neighbors
    auto getNeighbors = [&](const mcpp::Coordinate& coord) {
        std::vector<mcpp::Coordinate> neighbors;
        int moves[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
        for (auto& move : moves) {
            int newX = coord.x + move[0];
            int newZ = coord.z + move[1];
            if (isValidMove(newX, newZ)) {
                neighbors.emplace_back(newX, coord.y, newZ);
            }
        }
        return neighbors;
    };

    // Queue for BFS
    std::queue<mcpp::Coordinate> queue;
    std::unordered_map<mcpp::Coordinate, mcpp::Coordinate, std::hash<mcpp::Coordinate>> cameFrom;

    // Mark the starting position as visited and add it to the queue
    mcpp::Coordinate start(startX, startY, startZ);
    visited.insert(start);
    queue.push(start);

    bool escapeFound = false;
    mcpp::Coordinate escapePosition;

    while (!queue.empty() && !escapeFound) {
        mcpp::Coordinate current = queue.front();
        queue.pop();

        // Check if current position is the escape route (blue carpet)
        if (mc.getBlock(mcpp::Coordinate(current.x + baseX, startY + 1, current.z + baseZ)) == mcpp::Blocks::BLUE_CARPET) {
            std::cout << "Escape route found!" << std::endl;
            escapeFound = true;
            escapePosition = current;
            break;
        }

        // Explore neighbors
        for (const auto& neighbor : getNeighbors(current)) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                queue.push(neighbor);
                cameFrom[neighbor] = current;
            }
        }
    }

    // Backtrack and place lime carpets to show the escape path
    if (escapeFound) {
    mcpp::Coordinate current = escapePosition;
    while (!(current == start)) {
        mc.setBlock(mcpp::Coordinate(current.x + baseX, startY+1, current.z + baseZ), mcpp::Blocks::LIME_CARPET);
        changes.emplace_back(mcpp::Coordinate(current.x + baseX, startY+1, current.z + baseZ)); // Add to changes
        current = cameFrom[current];
    }
    mc.setBlock(mcpp::Coordinate(start.x + baseX, startY+1, start.z + baseZ), mcpp::Blocks::LIME_CARPET); // Mark the start position
    changes.emplace_back(mcpp::Coordinate(start.x + baseX, startY+1, start.z + baseZ)); // Add to changes
}

}
void showEscapeRouteFinal(mcpp::MinecraftConnection& mc, const std::vector<std::string>& maze, int baseX, int baseY, int baseZ, bool mode, bool showEscapeWithBFS) {
    if (showEscapeWithBFS ) {
        // Show escape route using BFS
        showEscapeRouteBFS(mc, maze, baseX, baseY, baseZ, mode);
    } else {
        // Show escape route in normal mode or default behavior
        showEscapeRoute(mc, maze, baseX, baseY, baseZ, mode);
    }
}