#ifndef UTILS_H
#define UTILS_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>

using namespace sf;
using namespace std;

// Constants
const int M = 20; // Rows
const int N = 10; // Columns
const int TILE_SIZE = 18;
const int MAX_HIGH_SCORES = 10;

// Extern Variables (Declared here, Defined in functionality.h)
extern int gameGrid[M][N];
extern int point_1[4][2];
extern int point_2[4][2];
extern int next_block[4][2];
extern int colorNum;
extern int colorNext;
extern int Game_score;
extern int g_level;
extern int nextShapeIndex;
extern int highScores[MAX_HIGH_SCORES];
extern float difficultyTimer;
extern int garbageRowCount;

// Function Prototypes
bool anamoly();
void checkLines();
void fallingPiece(float& timer, float delay, int& cNum, bool& hard, float gameTime, int& cNext, bool& gameOver);
void move(int dx);
void rotate_blocks();
void addGarbageRow();
void loadHighScores();
void saveHighScores();
void updateHighScores(int score);

#endif