#ifndef FUNCTIONALITY_H
#define FUNCTIONALITY_H

#include <SFML/Graphics.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>
#include "utils.h"
#include "pieces.h"

using namespace std;

// --- DEFINITION OF GLOBAL VARIABLES ---
// Use 'inline' to prevent multiple definition errors during linking
inline int gameGrid[M][N] = { 0 };
inline int point_1[4][2] = { 0 };
inline int point_2[4][2] = { 0 };
inline int next_block[4][2] = { 0 };
inline int colorNum = 1;
inline int colorNext = 1;
inline int Game_score = 0;
inline int g_level = 1;
inline int nextShapeIndex = 0;
inline int highScores[MAX_HIGH_SCORES] = { 0 };
inline int garbageRowCount = 0;

// --- HIGH SCORE LOGIC ---
inline void loadHighScores() {
    ifstream file("highscores.txt");
    if (file.is_open()) {
        for (int i = 0; i < MAX_HIGH_SCORES; i++) {
            if (!(file >> highScores[i])) highScores[i] = 0;
        }
        file.close();
    }
}

// Updated to accept 'score' to fix C2660 error
inline void saveHighScores(int score) {
    // Check and update local array first
    if (score > highScores[MAX_HIGH_SCORES - 1]) {
        highScores[MAX_HIGH_SCORES - 1] = score;
        sort(highScores, highScores + MAX_HIGH_SCORES, greater<int>());
    }

    ofstream file("highscores.txt");
    if (file.is_open()) {
        for (int i = 0; i < MAX_HIGH_SCORES; i++) {
            file << highScores[i] << " ";
        }
        file.close();
    }
}

// --- DIFFICULTY LOGIC (5 Minute Rule) ---
inline void addGarbageRow() {
    int rowToFill = M - 1 - garbageRowCount;
    if (rowToFill >= 0) {
        for (int j = 0; j < N; j++) {
            gameGrid[rowToFill][j] = 8; // 8 = Gray/Garbage block
        }
        garbageRowCount++;
    }
}

// --- CORE GAME FUNCTIONS ---

inline bool anamoly() {
    for (int i = 0; i < 4; i++) {
        int x = point_1[i][0];
        int y = point_1[i][1];
        if (x < 0 || x >= N || y >= M) return false;
        if (y >= 0 && gameGrid[y][x]) return false;
    }
    return true;
}

inline void checkLines() {
    int linesCleared = 0;
    for (int i = M - 1; i >= 0; i--) {
        int count = 0;
        bool isGarbageRow = false;
        for (int j = 0; j < N; j++) {
            if (gameGrid[i][j]) {
                count++;
                if (gameGrid[i][j] == 8) isGarbageRow = true;
            }
        }

        if (count == N && !isGarbageRow) {
            linesCleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < N; j++) {
                    gameGrid[k][j] = gameGrid[k - 1][j];
                }
            }
            for (int j = 0; j < N; j++) gameGrid[0][j] = 0;
            i++;
        }
    }

    if (linesCleared > 0) {
        // FIXED: Removed '* g_level' so that Level 1 and Level 2 give the same base points
        if (linesCleared == 1) Game_score += 10;
        else if (linesCleared == 2) Game_score += 30;
        else if (linesCleared == 3) Game_score += 60;
        else if (linesCleared >= 4) Game_score += 100;
    }
}

inline void fallingPiece(float& timer, float delay, int& cNum, bool& hard, float gameTime, int& cNext, bool& gameOver) {
    static float lastDifficultyCheck = 0;
    if (gameTime - lastDifficultyCheck >= 300.0f) {
        addGarbageRow();
        lastDifficultyCheck = gameTime;
    }

    if (timer > delay) {
        for (int i = 0; i < 4; i++) {
            point_2[i][0] = point_1[i][0];
            point_2[i][1] = point_1[i][1];
        }

        for (int i = 0; i < 4; i++) point_1[i][1]++;

        if (!anamoly()) {
            for (int i = 0; i < 4; i++) {
                gameGrid[point_2[i][1]][point_2[i][0]] = cNum;
            }

            checkLines();

            // Prepare for Spawning New Piece
            cNum = cNext;
            int n = nextShapeIndex;

            // --- SPAWN NEW PIECE & CHECK GAME OVER ---
            for (int i = 0; i < 4; i++) {
                point_1[i][0] = 3 + BLOCKS[n][i] % 2;
                point_1[i][1] = BLOCKS[n][i] / 2;

                // Check if spawn point is already filled
                if (gameGrid[point_1[i][1]][point_1[i][0]] != 0) {
                    gameOver = true;
                    saveHighScores(Game_score);
                    return;
                }
            }

            // Generate New "Next" Shape
            nextShapeIndex = (g_level == 2) ? rand() % 7 : rand() % 4;
            cNext = 1 + rand() % 7;
            for (int i = 0; i < 4; i++) {
                next_block[i][0] = BLOCKS[nextShapeIndex][i] % 2;
                next_block[i][1] = BLOCKS[nextShapeIndex][i] / 2;
            }
        }
        timer = 0;
    }
}

inline void move(int dx) {
    for (int i = 0; i < 4; i++) {
        point_2[i][0] = point_1[i][0];
        point_2[i][1] = point_1[i][1];
    }
    for (int i = 0; i < 4; i++) point_1[i][0] += dx;
    if (!anamoly()) {
        for (int i = 0; i < 4; i++) point_1[i][0] = point_2[i][0];
    }
}

inline void rotate_blocks() {
    for (int i = 0; i < 4; i++) {
        point_2[i][0] = point_1[i][0];
        point_2[i][1] = point_1[i][1];
    }
    int px = point_1[1][0];
    int py = point_1[1][1];
    for (int i = 0; i < 4; i++) {
        int x = point_1[i][1] - py;
        int y = point_1[i][0] - px;
        point_1[i][0] = px - x;
        point_1[i][1] = py + y;
    }
    if (!anamoly()) {
        for (int i = 0; i < 4; i++) {
            point_1[i][0] = point_2[i][0];
            point_1[i][1] = point_2[i][1];
        }
    }
}

#endif