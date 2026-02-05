#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "utils.h"         
#include "pieces.h"        
#include "functionality.h" 

using namespace std;
using namespace sf;

// Game States Enum
enum GameState { MAIN_MENU, HIGH_SCORES, LEVEL_SELECT, IN_GAME, HELP_SCREEN, PAUSE_MENU, GAME_OVER };
GameState currentState = MAIN_MENU;
bool hardDrop = false;

// --- UI Function Prototypes ---
void showMainMenu(RenderWindow& window, Font& font);
void showLevelSelect(RenderWindow& window, Font& font);
void showHighScoresMenu(RenderWindow& window, Font& font);
void showHelp(RenderWindow& window, Font& font);
void showPauseMenu(RenderWindow& window, Font& font);
void showGameOver(RenderWindow& window, Font& font, int finalScore);

// Helper to reset game state
void resetGame() {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            gameGrid[i][j] = 0;
        }
    }
    Game_score = 0;
    garbageRowCount = 0;
    // Reset next piece
    nextShapeIndex = rand() % 7;
    colorNext = 1 + rand() % 7;
    for (int i = 0; i < 4; i++) {
        next_block[i][0] = BLOCKS[nextShapeIndex][i] % 2;
        next_block[i][1] = BLOCKS[nextShapeIndex][i] / 2;
    }
}

int main() {
    srand(time(0));
    loadHighScores(); // Load scores from file at startup

    // Window Size: 460x500 fits the UI and frame comfortably
    RenderWindow window(VideoMode(460, 500), "Tetris - Air University");

    Texture bgTex, tileTex, frameTex;
    // Loading Textures (Make sure these files exist in your folder)
    if (!bgTex.loadFromFile("background.png")) { /* Handle Error or close */ }
    if (!tileTex.loadFromFile("tiles.png")) { /* Handle Error or close */ }
    if (!frameTex.loadFromFile("frame.png")) { /* Handle Error or close */ }

    Sprite background(bgTex);
    background.setScale(460.f / bgTex.getSize().x, 500.f / bgTex.getSize().y);

    Sprite mainFrame(frameTex);
    // Frame Positioning
    const int FRAME_X = 20;
    const int FRAME_Y = 20;
    mainFrame.setPosition(FRAME_X, FRAME_Y);
    mainFrame.setScale(1.f, 1.f);

    Sprite tileSprite(tileTex);

    Font font;
    if (!font.loadFromFile("arial.ttf")) { /* Handle Error or close */ }

    // Initialize first piece
    nextShapeIndex = rand() % 7;
    colorNext = 1 + rand() % 7;
    for (int i = 0; i < 4; i++) {
        next_block[i][0] = BLOCKS[nextShapeIndex][i] % 2;
        next_block[i][1] = BLOCKS[nextShapeIndex][i] / 2;
    }

    float timer = 0, delay = 0.3, gameTime = 0;
    Clock clock;

    // Calculate Drawing Offsets to center grid inside the pink frame
    // Frame: 219x394. Grid: 10x18=180w, 20x18=360h.
    const int DRAW_OFFSET_X = FRAME_X + (219 - (N * TILE_SIZE)) / 2; // Center X
    const int DRAW_OFFSET_Y = FRAME_Y + (394 - (M * TILE_SIZE)) / 2; // Center Y

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;
        gameTime += time;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();

            if (e.type == Event::KeyPressed) {
                if (currentState == MAIN_MENU) {
                    if (e.key.code == Keyboard::Num1) currentState = LEVEL_SELECT;
                    else if (e.key.code == Keyboard::Num2) currentState = HIGH_SCORES;
                    else if (e.key.code == Keyboard::Num3) currentState = HELP_SCREEN;
                    else if (e.key.code == Keyboard::Num4) window.close();
                }
                else if (currentState == HIGH_SCORES) {
                    if (e.key.code == Keyboard::Escape) currentState = MAIN_MENU;
                }
                else if (currentState == LEVEL_SELECT) {
                    if (e.key.code == Keyboard::Num1) {
                        g_level = 1;
                        resetGame(); // FIXED: Reset game before starting
                        currentState = IN_GAME;
                    }
                    else if (e.key.code == Keyboard::Num2) {
                        g_level = 2;
                        resetGame(); // FIXED: Reset game before starting
                        currentState = IN_GAME;
                    }
                    else if (e.key.code == Keyboard::Escape) currentState = MAIN_MENU;
                }
                else if (currentState == HELP_SCREEN) {
                    if (e.key.code == Keyboard::Num1 || e.key.code == Keyboard::Escape) currentState = MAIN_MENU;
                }
                else if (currentState == IN_GAME) {
                    if (e.key.code == Keyboard::Left) { int dx = -1; move(dx); }
                    else if (e.key.code == Keyboard::Right) { int dx = 1; move(dx); }
                    else if (e.key.code == Keyboard::Up) rotate_blocks();
                    else if (e.key.code == Keyboard::P) currentState = PAUSE_MENU;
                }
                else if (currentState == PAUSE_MENU) {
                    if (e.key.code == Keyboard::Num1) {
                        // Start New Game
                        resetGame();
                        currentState = LEVEL_SELECT;
                    }
                    else if (e.key.code == Keyboard::Num2) currentState = HIGH_SCORES;
                    else if (e.key.code == Keyboard::Num3) currentState = HELP_SCREEN;
                    else if (e.key.code == Keyboard::Num4) window.close();
                    else if (e.key.code == Keyboard::Num5 || e.key.code == Keyboard::Escape) currentState = IN_GAME;
                }
                else if (currentState == GAME_OVER) {
                    if (e.key.code == Keyboard::Num1) currentState = LEVEL_SELECT; // Play Again -> Level Select
                    else if (e.key.code == Keyboard::Num2) currentState = MAIN_MENU;
                    else if (e.key.code == Keyboard::Num3) window.close();
                }
            }
        }

        if (currentState == IN_GAME) {
            // Logic handled in functionality.h
            delay = (g_level == 1) ? 0.5 : 0.2;
            if (Keyboard::isKeyPressed(Keyboard::Down)) delay = 0.05;

            bool isGameOver = false;
            fallingPiece(timer, delay, colorNum, hardDrop, gameTime, colorNext, isGameOver);

            if (isGameOver) currentState = GAME_OVER;
        }

        window.clear();
        window.draw(background);

        if (currentState == MAIN_MENU) showMainMenu(window, font);
        else if (currentState == HIGH_SCORES) showHighScoresMenu(window, font);
        else if (currentState == LEVEL_SELECT) showLevelSelect(window, font);
        else if (currentState == HELP_SCREEN) showHelp(window, font);
        else if (currentState == PAUSE_MENU) {
            // Draw Frame and Frozen Game in Background
            window.draw(mainFrame);
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (gameGrid[i][j] == 0) continue;
                    tileSprite.setTextureRect(IntRect(gameGrid[i][j] * 18, 0, 18, 18));
                    tileSprite.setPosition(j * TILE_SIZE + DRAW_OFFSET_X, i * TILE_SIZE + DRAW_OFFSET_Y);
                    window.draw(tileSprite);
                }
            }
            showPauseMenu(window, font);
        }
        else if (currentState == GAME_OVER) showGameOver(window, font, Game_score);
        else if (currentState == IN_GAME) {
            // --- IN GAME RENDERING ---
            window.draw(mainFrame);

            // 1. Draw Right Side - Next Box (MOVED DOWN)
            Text nextLbl("NEXT", font, 18);
            nextLbl.setPosition(260, 60); // Was 30
            nextLbl.setFillColor(Color::White);
            nextLbl.setStyle(Text::Bold);
            window.draw(nextLbl);

            RectangleShape nextBoxBorder(Vector2f(100, 100));
            nextBoxBorder.setPosition(250, 85); // Was 55
            nextBoxBorder.setFillColor(Color(50, 50, 50, 200));
            nextBoxBorder.setOutlineColor(Color::White);
            nextBoxBorder.setOutlineThickness(2);
            window.draw(nextBoxBorder);

            int nextBoxCenterX = 250 + 50;
            int nextBoxCenterY = 85 + 50;
            for (int i = 0; i < 4; i++) {
                tileSprite.setTextureRect(IntRect(colorNext * 18, 0, 18, 18));
                tileSprite.setPosition(nextBoxCenterX + (next_block[i][0] * 18) - 18, nextBoxCenterY + (next_block[i][1] * 18) - 18);
                window.draw(tileSprite);
            }

            // 2. Draw Right Side - Score Box
            RectangleShape scoreBox(Vector2f(140, 50));
            scoreBox.setPosition(260, 200); // Moved down slightly to make room
            scoreBox.setFillColor(Color(40, 40, 40, 220));
            scoreBox.setOutlineColor(Color::Yellow);
            scoreBox.setOutlineThickness(2);
            window.draw(scoreBox);

            Text scoreTxt("SCORE: " + to_string(Game_score), font, 18);
            scoreTxt.setPosition(275, 210);
            scoreTxt.setFillColor(Color::Yellow);
            scoreTxt.setStyle(Text::Bold);
            window.draw(scoreTxt);

            // 3. Draw Right Side - Level Box
            RectangleShape levelBox(Vector2f(140, 50));
            levelBox.setPosition(260, 260);
            levelBox.setFillColor(Color(40, 40, 40, 220));
            levelBox.setOutlineColor(Color::Cyan);
            levelBox.setOutlineThickness(2);
            window.draw(levelBox);

            Text levelTxt("LEVEL: " + to_string(g_level), font, 18);
            levelTxt.setPosition(275, 270);
            levelTxt.setFillColor(Color::Cyan);
            levelTxt.setStyle(Text::Bold);
            window.draw(levelTxt);

            // 4. Controls Hint
            Text controlsTxt("P - Pause", font, 16);
            controlsTxt.setPosition(265, 460);
            controlsTxt.setFillColor(Color(200, 200, 200));
            window.draw(controlsTxt);

            // 5. Draw Locked Grid Blocks
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (gameGrid[i][j] == 0) continue;

                    int colorIndex = gameGrid[i][j];
                    if (colorIndex == 8) colorIndex = 8;

                    tileSprite.setTextureRect(IntRect(colorIndex * 18, 0, 18, 18));
                    tileSprite.setPosition(j * TILE_SIZE + DRAW_OFFSET_X, i * TILE_SIZE + DRAW_OFFSET_Y);
                    window.draw(tileSprite);
                }
            }

            // 6. Draw Shadow (Ghost Piece)
            int shadowY[4];
            for (int i = 0; i < 4; i++) shadowY[i] = point_1[i][1];
            bool canDrop = true;
            while (canDrop) {
                for (int i = 0; i < 4; i++) shadowY[i]++;
                for (int i = 0; i < 4; i++) {
                    int x = point_1[i][0], y = shadowY[i];
                    if (y >= M || (y >= 0 && gameGrid[y][x])) { canDrop = false; for (int j = 0; j < 4; j++) shadowY[j]--; break; }
                }
            }
            for (int i = 0; i < 4; i++) {
                if (shadowY[i] >= 0 && shadowY[i] != point_1[i][1]) {
                    tileSprite.setTextureRect(IntRect(colorNum * 18, 0, 18, 18));
                    tileSprite.setColor(Color(255, 255, 255, 80));
                    tileSprite.setPosition(point_1[i][0] * TILE_SIZE + DRAW_OFFSET_X, shadowY[i] * TILE_SIZE + DRAW_OFFSET_Y);
                    window.draw(tileSprite);
                }
            }
            tileSprite.setColor(Color::White);

            // 7. Draw Active Piece
            for (int i = 0; i < 4; i++) {
                tileSprite.setTextureRect(IntRect(colorNum * 18, 0, 18, 18));
                tileSprite.setPosition(point_1[i][0] * TILE_SIZE + DRAW_OFFSET_X, point_1[i][1] * TILE_SIZE + DRAW_OFFSET_Y);
                window.draw(tileSprite);
            }
        }
        window.display();
    }
    return 0;
}

// --- UI Functions Implementations ---

void showMainMenu(RenderWindow& window, Font& font) {
    Text title("TETRIS", font, 60);
    title.setPosition(150, 50);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    title.setOutlineColor(Color::Blue);
    title.setOutlineThickness(3);
    window.draw(title);

    auto drawBtn = [&](int y, Color c, string str, Color txtC) {
        RectangleShape btn(Vector2f(300, 60));
        btn.setFillColor(c);
        btn.setPosition(100, y);
        btn.setOutlineColor(Color::White);
        btn.setOutlineThickness(2);
        Text txt(str, font, 24);
        txt.setPosition(130, y + 15);
        txt.setFillColor(txtC);
        txt.setStyle(Text::Bold);
        window.draw(btn);
        window.draw(txt);
        };
    drawBtn(180, Color(0, 200, 0), "1. New Game", Color::Black);
    drawBtn(260, Color(0, 100, 200), "2. High Scores", Color::White);
    drawBtn(340, Color(200, 50, 50), "3. Help", Color::White);
    drawBtn(420, Color(220, 220, 0), "4. Exit", Color::Black);
}

void showHighScoresMenu(RenderWindow& window, Font& font) {
    Text title("HIGH SCORES", font, 40);
    title.setPosition(130, 50);
    title.setFillColor(Color::Yellow);
    title.setStyle(Text::Bold);
    window.draw(title);

    for (int i = 0; i < MAX_HIGH_SCORES; i++) {
        Text txt(to_string(i + 1) + ". " + to_string(highScores[i]), font, 20);
        txt.setPosition(160, 120 + i * 30);
        txt.setFillColor(Color::White);
        window.draw(txt);
    }

    Text back("Press ESC to return", font, 18);
    back.setPosition(150, 450);
    back.setFillColor(Color(200, 200, 200));
    window.draw(back);
}

void showLevelSelect(RenderWindow& window, Font& font) {
    RectangleShape overlay(Vector2f(460, 500));
    overlay.setFillColor(Color(0, 0, 0, 200));
    window.draw(overlay);

    Text title("SELECT DIFFICULTY", font, 32);
    title.setPosition(90, 80);
    title.setFillColor(Color::Yellow);
    title.setStyle(Text::Bold);
    window.draw(title);

    auto drawBtn = [&](int y, Color c, string str, string sub) {
        RectangleShape btn(Vector2f(300, 100));
        btn.setFillColor(c);
        btn.setPosition(90, y);
        btn.setOutlineColor(Color::White);
        btn.setOutlineThickness(3);
        window.draw(btn);

        Text txt(str, font, 28);
        txt.setPosition(120, y + 20);
        txt.setFillColor(Color::White);
        txt.setStyle(Text::Bold);
        window.draw(txt);

        Text desc(sub, font, 18);
        desc.setPosition(130, y + 60);
        desc.setFillColor(Color(220, 220, 220));
        window.draw(desc);
        };

    drawBtn(160, Color(0, 150, 0), "1. BEGINNER", "(4 shapes, slower)");
    drawBtn(280, Color(200, 0, 0), "2. ADVANCED", "(7 shapes, faster)");

    Text back("Press ESC to go back", font, 16);
    back.setPosition(150, 420);
    back.setFillColor(Color(150, 150, 150));
    window.draw(back);
}

void showPauseMenu(RenderWindow& window, Font& font) {
    RectangleShape overlay(Vector2f(460, 500));
    overlay.setFillColor(Color(0, 0, 0, 180));
    window.draw(overlay);

    RectangleShape menuBox(Vector2f(360, 420));
    menuBox.setPosition(80, 50);
    menuBox.setFillColor(Color(30, 30, 30, 250));
    menuBox.setOutlineColor(Color::Magenta);
    menuBox.setOutlineThickness(4);
    window.draw(menuBox);

    Text title("PAUSED", font, 50);
    title.setPosition(150, 80);
    title.setFillColor(Color::Magenta);
    title.setStyle(Text::Bold);
    window.draw(title);

    auto drawBtn = [&](int y, Color c, string str, Color txtC) {
        RectangleShape btn(Vector2f(300, 55));
        btn.setFillColor(c);
        btn.setPosition(110, y);
        btn.setOutlineColor(Color::White);
        btn.setOutlineThickness(2);
        Text txt(str, font, 22);
        txt.setPosition(130, y + 12);
        txt.setFillColor(txtC);
        txt.setStyle(Text::Bold);
        window.draw(btn);
        window.draw(txt);
        };

    drawBtn(170, Color(0, 200, 0), "1. New Game", Color::Black);
    drawBtn(235, Color(0, 100, 200), "2. High Scores", Color::White);
    drawBtn(300, Color(200, 50, 50), "3. Help", Color::White);
    drawBtn(365, Color(220, 220, 0), "4. Exit", Color::Black);

    // Continue button moved to BOTTOM
    RectangleShape continueBtn(Vector2f(300, 55));
    continueBtn.setFillColor(Color(0, 255, 0));
    continueBtn.setPosition(110, 430); // Moved down to 430
    continueBtn.setOutlineColor(Color::Yellow);
    continueBtn.setOutlineThickness(3);
    window.draw(continueBtn);
    Text continueTxt("5. CONTINUE", font, 24);
    continueTxt.setPosition(145, 440); // Adjusted text position
    continueTxt.setFillColor(Color::Black);
    continueTxt.setStyle(Text::Bold);
    window.draw(continueTxt);
}

void showGameOver(RenderWindow& window, Font& font, int finalScore) {
    RectangleShape overlay(Vector2f(460, 500));
    overlay.setFillColor(Color(0, 0, 0, 220));
    window.draw(overlay);

    // FIXED: Box Position to fit screen nicely
    RectangleShape gameOverBox(Vector2f(400, 350));
    gameOverBox.setPosition(30, 70); // X=30 to center horizontally in 460px window
    gameOverBox.setFillColor(Color(40, 0, 0, 250));
    gameOverBox.setOutlineColor(Color::Red);
    gameOverBox.setOutlineThickness(5);
    window.draw(gameOverBox);

    Text title("GAME OVER", font, 48);
    title.setPosition(100, 95);
    title.setFillColor(Color::Red);
    title.setStyle(Text::Bold);
    title.setOutlineColor(Color(150, 0, 0));
    title.setOutlineThickness(2);
    window.draw(title);

    RectangleShape scoreBox(Vector2f(320, 70));
    scoreBox.setPosition(70, 180);
    scoreBox.setFillColor(Color(60, 60, 0, 200));
    scoreBox.setOutlineColor(Color::Yellow);
    scoreBox.setOutlineThickness(3);
    window.draw(scoreBox);

    Text scoreTxt("FINAL SCORE: " + to_string(finalScore), font, 28);
    scoreTxt.setPosition(90, 200);
    scoreTxt.setFillColor(Color::Yellow);
    scoreTxt.setStyle(Text::Bold);
    window.draw(scoreTxt);

    // Functional Options
    Text opt1("1. Play Again", font, 24);
    opt1.setPosition(160, 280);
    opt1.setFillColor(Color::Green);
    window.draw(opt1);
    Text opt2("2. Main Menu", font, 24);
    opt2.setPosition(160, 320);
    opt2.setFillColor(Color::Cyan);
    window.draw(opt2);
    Text opt3("3. Exit", font, 24);
    opt3.setPosition(160, 360);
    opt3.setFillColor(Color::Red);
    window.draw(opt3);
}

void showHelp(RenderWindow& window, Font& font) {
    Text title("HOW TO PLAY", font, 40);
    title.setPosition(120, 50);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    window.draw(title);

    string helpText =
        "CONTROLS:\n\n"
        "  LEFT Arrow  - Move Left\n"
        "  RIGHT Arrow - Move Right\n"
        "  UP Arrow    - Rotate Block\n"
        "  DOWN Arrow  - Drop Faster\n"
        "  P Key       - Pause Game\n\n"
        "OBJECTIVE:\n"
        "  Fill complete rows to clear\n"
        "  them and earn points!\n\n"
        "Note: Every 5 mins, a row fills up!\n\n"
        "Press 1 or ESC to return";

    Text helpTxt(helpText, font, 18);
    helpTxt.setPosition(70.f, 130.f);
    helpTxt.setFillColor(Color::White);
    helpTxt.setLineSpacing(1.3f);
    window.draw(helpTxt);
}