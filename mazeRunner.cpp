#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <mcpp/mcpp.h>
#include "menuUtils.h"
#include "Maze.h"
#include "Agent.h"

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

int main(int argc, char* argv[]) {
    bool showEscapeWithBFS = false;
    bool mode = NORMAL_MODE;
    bool avoidObstacles = false;

    if (argc > 1) {
        if (std::string(argv[1]) == "-testmode") {
            mode = TESTING_MODE;
        }
        if ((std::string(argv[1]) == "-avoidObstacles") || (argc > 2 && std::string(argv[2]) == "-avoidObstacles")) {
            avoidObstacles = true;
        }
        if ((std::string(argv[1]) == "-showEscapeWithBFS") || (argc > 2 && std::string(argv[2]) == "-showEscapeWithBFS")) {
            showEscapeWithBFS = true;
    }
    }

    mcpp::MinecraftConnection mc;
    std::vector<std::pair<mcpp::Coordinate, mcpp::BlockType>> storedBlocks;
    mc.doCommand("time set day");

    if (mode == TESTING_MODE) {
        mc.doCommand("tp 4848 71 4369");
    }

    printStartText();

    States curState = ST_Main;

    while (curState != ST_Exit) {
        if (curState == ST_Main) {
            printMainMenu();
            int choice;
            std::cin >> choice;
            if (std::cin.fail() || choice < 1 || choice > 5) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Input Error: Enter a number between 1 and 5 ...." << std::endl;
            } else {
                if (choice == 1) {
                    curState = ST_GetMaze;
                } else if (choice == 2) {
                    curState = ST_BuildMaze;
                } else if (choice == 3) {
                    curState = ST_SolveMaze;
                } else if (choice == 4) {
                    curState = ST_Creators;
                } else if (choice == 5) {
                    curState = ST_Exit;
                }
            }
        } else if (curState == ST_GetMaze) {
            printGenerateMazeMenu();
            int choice;
            std::cin >> choice;
            if (std::cin.fail() || choice < 1 || choice > 3) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Input Error: Enter a number between 1 and 3 ...." << std::endl;
            } else {
                if (choice == 1) {
                    readMazeFromTerminal(mc, mode);
                    curState = ST_Main;
                } else if (choice == 2) {
                    generateRandomMaze(mc, mode);
                    curState = ST_Main;
                } else if (choice == 3) {
                    curState = ST_Main;
                }
            }
        } else if (curState == ST_BuildMaze) {
            if (!currentMaze.empty()) {
                buildMazeInMinecraft(mc, currentMaze, mazeX, mazeY, mazeZ, avoidObstacles,storedBlocks);
            } else {
                std::cout << "No maze available to build. Generate a maze first." << std::endl;
            }
            curState = ST_Main;
        } else if (curState == ST_SolveMaze) {
            printSolveMazeMenu();
            int choice;
            std::cin >> choice;
            if (std::cin.fail() || choice < 1 || choice > 3) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Input Error: Enter a number between 1 and 3 ...." << std::endl;
            } else {
                if (choice == 1) {
                    solveMazeManually(mc, currentMaze, mazeX, mazeY, mazeZ, mode);
                    curState = ST_Main;
                } else if (choice == 2) {
                    if (!currentMaze.empty()) {
                        showEscapeRouteFinal(mc,currentMaze, mazeX, mazeY, mazeZ, mode, showEscapeWithBFS);
                        //showEscapeRoute(mc, currentMaze, mazeX, mazeY, mazeZ, mode);
                        //showEscapeRouteBFS(mc, currentMaze, mazeX, mazeY, mazeZ, mode);
                    } else {
                        std::cout << "No maze available to solve. Generate a maze first." << std::endl;
                    }
                    curState = ST_Main;
                } else if (choice == 3) {
                    curState = ST_Main;
                }
            }
        } else if (curState == ST_Creators) {
            printTeamInfo();
            curState = ST_Main;
        } else if (curState == ST_Exit) {
            //std::cout << "Cleaning the world..." << std::endl;
            std::vector<mcpp::Coordinate> mazeChanges; // Vector to store maze changes
            for (const auto& pair : storedBlocks) {
                mazeChanges.push_back(pair.first);
    }
    // cleanWorld(mc, mazeChanges);
        }
    }

    // Only clean the world when exiting
    printExitMessage();
    //std::cout << "Cleaning the world..." << std::endl;
    std::vector<mcpp::Coordinate> mazeChanges; // Vector to store maze changes
    for (const auto& pair : storedBlocks) {
        mazeChanges.push_back(pair.first);
    }
    //revertBlocks(mc, storedBlocks);
    cleanWorld(mc, changes);

    // std::cout << "Reverting changes..." << std::endl;
    revertBlocks(mc, storedBlocks);

    return EXIT_SUCCESS;
}