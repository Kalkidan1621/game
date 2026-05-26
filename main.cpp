#include <graphics.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cmath>

const int ROWS = 10;
const int COLS = 10;
const int BLOCK_SIZE = 30;

int grid[ROWS][COLS];
int score = 0;
int level = 1;
int target = 1000;
int bestScore = 0;

int colors[6] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA};
bool redrawNeeded = true;
bool stageCleared = false;
bool paused = false;

int lastRow = -1;
int lastCol = -1;

// Pause button area
int pauseX1, pauseY1, pauseX2, pauseY2;

void drawMenu() {
    cleardevice();
    setbkcolor(BLACK);
    cleardevice();

    // Title
    setcolor(YELLOW);
    settextstyle(4,0,4);
    outtextxy(100, 50,"POP STAR CLASSIC");

    // Best Score
    setcolor(WHITE);
    settextstyle(3,0,2);
    char buf[50];
    sprintf(buf, "BEST SCORE %d", bestScore);
    outtextxy(100, 120, buf);

    // Buttons
    setfillstyle(SOLID_FILL, RED);
    bar(120, 150, 280, 190);
    setcolor(WHITE);
    outtextxy(150, 160, (char*)"NEW GAME");

    bar(120, 200, 280, 240);
    outtextxy(140, 210, (char*)"RESUME");

    bar(120, 250, 280, 290);
    outtextxy(160, 260, (char*)"RATE");

    bar(120, 300, 280, 340);
    outtextxy(150, 310, (char*)"NO ADS");
}

void drawStatusBar() {
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, COLS*BLOCK_SIZE + 200, 100);

    setcolor(WHITE);
    settextstyle(3,0,1);

    char buf1[50];
    sprintf(buf1, "BEST SCORE %d", bestScore);
    outtextxy(20, 15, buf1);

    char buf2[80];
    sprintf(buf2, "STAGE %d   TARGET %d", level, target);
    outtextxy(20, 45, buf2);

    char buf3[50];
    sprintf(buf3, "SCORE %d", score);
    outtextxy(20, 75, buf3);

    // Draw pause symbol (two vertical bars)
    setcolor(RED);
    pauseX1 = COLS*BLOCK_SIZE+50;
    pauseY1 = 20;
    pauseX2 = COLS*BLOCK_SIZE+100;
    pauseY2 = 60;
    rectangle(pauseX1, pauseY1, pauseX1+20, pauseY2);
    rectangle(pauseX1+30, pauseY1, pauseX2, pauseY2);
}

// Improved star drawing: filled polygon with outline
void drawStar(int cx, int cy, int size, int color) {
    int points[20];
    for (int i=0; i<5; i++) {
        points[i*4]   = cx + (int)(size * cos(i*72 * M_PI/180));
        points[i*4+1] = cy + (int)(size * sin(i*72 * M_PI/180));
        points[i*4+2] = cx + (int)(size/2 * cos((i*72+36) * M_PI/180));
        points[i*4+3] = cy + (int)(size/2 * sin((i*72+36) * M_PI/180));
    }
    setfillstyle(SOLID_FILL, color);
    fillpoly(10, points);
    setcolor(RED);
    drawpoly(10, points);
}

void drawGrid() {
    cleardevice();
    drawStatusBar();
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] != -1) {
                setfillstyle(SOLID_FILL, grid[r][c]);
                bar(c*BLOCK_SIZE, r*BLOCK_SIZE+110,
                    c*BLOCK_SIZE+BLOCK_SIZE, r*BLOCK_SIZE+BLOCK_SIZE+110);

                int cx = c*BLOCK_SIZE + BLOCK_SIZE/2;
                int cy = r*BLOCK_SIZE + 110 + BLOCK_SIZE/2;
                drawStar(cx, cy, BLOCK_SIZE/2 - 5, grid[r][c]);
            }
        }
    }
}

void dfs(int r, int c, int color, std::vector<std::pair<int,int> > &group) {
    if (r<0 || r>=ROWS || c<0 || c>=COLS) return;
    if (grid[r][c] != color) return;
    for (size_t i=0; i<group.size(); i++) {
        if (group[i].first==r && group[i].second==c) return;
    }
    group.push_back(std::pair<int,int>(r,c));
    dfs(r+1,c,color,group);
    dfs(r-1,c,color,group);
    dfs(r,c+1,color,group);
    dfs(r,c-1,color,group);
}

void applyGravity() {
    for (int c=0;c<COLS;c++) {
        int writeRow = ROWS-1;
        for (int r=ROWS-1;r>=0;r--) {
            if (grid[r][c]!=-1) {
                grid[writeRow][c]=grid[r][c];
                if (writeRow!=r) grid[r][c]=-1;
                writeRow--;
            }
        }
    }
    int writeCol=0;
    for (int c=0;c<COLS;c++) {
        bool empty=true;
        for (int r=0;r<ROWS;r++) if (grid[r][c]!=-1) empty=false;
        if (!empty) {
            for (int r=0;r<ROWS;r++) {
                grid[r][writeCol]=grid[r][c];
                if (writeCol!=c) grid[r][c]=-1;
            }
            writeCol++;
        }
    }
}

bool hasMoves() {
    for (int r=0;r<ROWS;r++) {
        for (int c=0;c<COLS;c++) {
            if (grid[r][c]!=-1) {
                std::vector<std::pair<int,int> > group;
                dfs(r,c,grid[r][c],group);
                if (group.size()>=2) return true;
            }
        }
    }
    return false;
}

void initGrid() {
    for (int r=0;r<ROWS;r++)
        for (int c=0;c<COLS;c++)
            grid[r][c] = colors[rand()%6];
    redrawNeeded = true;
}

void showAppreciation(int groupSize) {
    setcolor(RED);
    settextstyle(0,0,2);
    if (groupSize >= 10)
        outtextxy(150, ROWS*BLOCK_SIZE+120, (char*)"Excellent!");
    else if (groupSize >= 6)
        outtextxy(150, ROWS*BLOCK_SIZE+120, (char*)"Very Good!");
    else if (groupSize >= 3)
        outtextxy(150, ROWS*BLOCK_SIZE+120, (char*)"Good!");
    delay(1000);
    redrawNeeded = true;
}

int main() {
    srand(time(0));// GENERATE DIFFERNT GRID EVERY RUN
    initwindow(COLS*BLOCK_SIZE + 200, ROWS*BLOCK_SIZE + 250, "Pop Star Clone");

    // Show menu
    bool startGame = false;
    while (!startGame) {
        drawMenu();
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx,my;
            getmouseclick(WM_LBUTTONDOWN,mx,my);
            if (mx>=120 && mx<=280 && my>=150 && my<=190) {
                initGrid();
                startGame = true;
            }
            else if (mx>=120 && mx<=280 && my>=200 && my<=240) {
                score = 0; level = 1; target = 1000;
                initGrid();
                startGame = true;
            }
            else if (mx>=120 && mx<=280 && my>=250 && my<=290) {
                outtextxy(120, 370, (char*)"Thanks for rating!");
                delay(1000);
            }
            else if (mx>=120 && mx<=280 && my>=300 && my<=340) {
                outtextxy(120, 370, (char*)"Ads removed!");
                delay(1000);
            }
        }
        delay(50);
    }

    // Main game loop
    while (true) {
        if (redrawNeeded) {
            drawGrid();
            redrawNeeded = false;
        }

        // Handle mouse clicks
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx,my;
            getmouseclick(WM_LBUTTONDOWN,mx,my);

            // Pause button clicked
            if (mx >= pauseX1 && mx <= pauseX2 && my >= pauseY1 && my <= pauseY2) {
                paused = !paused;
                if (paused) {
                    setcolor(WHITE);
                    settextstyle(3,0,3);
                    outtextxy(COLS*BLOCK_SIZE/2 - 60, ROWS*BLOCK_SIZE/2, (char*)"PAUSED");

                    setcolor(LIGHTRED);
                    rectangle(COLS*BLOCK_SIZE/2 - 80, ROWS*BLOCK_SIZE/2 + 50,
                              COLS*BLOCK_SIZE/2 - 20, ROWS*BLOCK_SIZE/2 + 90);
                    outtextxy(COLS*BLOCK_SIZE/2 - 70, ROWS*BLOCK_SIZE/2 + 60, (char*)"SAVE");

                    // ▶ Play icon
                    setcolor(LIGHTGREEN);
                    rectangle(COLS*BLOCK_SIZE/2 + 20, ROWS*BLOCK_SIZE/2 + 50,
                              COLS*BLOCK_SIZE/2 + 80, ROWS*BLOCK_SIZE/2 + 90);
                    int px = COLS*BLOCK_SIZE/2 + 35;
                    int py = ROWS*BLOCK_SIZE/2 + 55;
                    int size = 25;
                    line(px, py, px, py+size);
                    line(px, py, px+size, py+size/2);
                    line(px, py+size, px+size, py+size/2);
                } else {
                    redrawNeeded = true;
                }
            }

            // If paused, check SAVE/PLAY buttons
            if (paused) {
                // SAVE button
                if (mx >= COLS*BLOCK_SIZE/2 - 80 && mx <= COLS*BLOCK_SIZE/2 - 20 &&
                    my >= ROWS*BLOCK_SIZE/2 + 50 && my <= ROWS*BLOCK_SIZE/2 + 90) {
                    outtextxy(COLS*BLOCK_SIZE/2 - 60, ROWS*BLOCK_SIZE/2 + 120, (char*)"Game Saved!");
                    delay(1000);
                }
                // PLAY button (triangle area)
                if (mx >= COLS*BLOCK_SIZE/2 + 20 && mx <= COLS*BLOCK_SIZE/2 + 80 &&
                    my >= ROWS*BLOCK_SIZE/2 + 50 && my <= ROWS*BLOCK_SIZE/2 + 90) {
                    paused = false;
                    redrawNeeded = true;
                }
            }

            // Normal gameplay clicks only if not paused
            if (!paused) {
                int r=(my-110)/BLOCK_SIZE, c=mx/BLOCK_SIZE;
                if (r>=0 && r<ROWS && c<COLS && grid[r][c]!=-1) {
                    lastRow = r;
                    lastCol = c;

                    std::vector<std::pair<int,int> > group;
                    dfs(r,c,grid[r][c],group);
                    if (group.size()>=2) {
                        for (size_t i=0;i<group.size();i++)
                            grid[group[i].first][group[i].second] = -1;
                        score += group.size()*group.size()*5;
                        applyGravity();
                        showAppreciation(group.size());

                        if (!stageCleared && score >= target) {
                            setcolor(RED);
                            settextstyle(4,0,4);
                            outtextxy(COLS*BLOCK_SIZE/2 - 120, ROWS*BLOCK_SIZE/2, (char*)"STAGE CLEAR");
                            delay(1000);
                            redrawNeeded = true;
                            stageCleared = true;
                        }
                    }
                }
            }
        }

        // Check if no moves remain
        if (!paused && !hasMoves()) {
            if (score >= target) {
                // LEVEL UP
                cleardevice();
                setcolor(WHITE);
                settextstyle(3,0,3);
                outtextxy(120, 180, (char*)"LEVEL UP!");
                char msg1[100];
                sprintf(msg1, "Level Number: %d", level+1);
                outtextxy(120, 220, msg1);
                char msg2[100];
                sprintf(msg2, "Next Target: %d", target+1500);
                outtextxy(120, 260, msg2);
                setcolor(YELLOW);
                rectangle(120, 300, 280, 340);
                outtextxy(160, 310, (char*)"NEXT");

                bool goNext = false;
                while (!goNext) {
                    if (kbhit()) {
                        int key = getch();
                        if (key == 9) goNext = true;
                    }
                    if (ismouseclick(WM_LBUTTONDOWN)) {
                        int mx,my;
                        getmouseclick(WM_LBUTTONDOWN,mx,my);
                        if (mx>=120 && mx<=280 && my>=300 && my<=340) {
                            goNext = true;
                        }
                    }
                    delay(50);
                }

                level++;
                target += 1500;
                if (score > bestScore) bestScore = score;
                stageCleared = false;
                initGrid();
            } else {
                // GAME OVER
                cleardevice();
                setcolor(WHITE);
                settextstyle(3,0,3);
                outtextxy(120, 180, (char*)"GAME OVER!");
                char msg1[100];
                sprintf(msg1, "Your Score: %d", score);
                outtextxy(120, 220, msg1);
                char msg2[100];
                sprintf(msg2, "Best Score: %d", bestScore);
                outtextxy(120, 260, msg2);
                char msg3[100];
                sprintf(msg3, "Target Score: %d", target);
                outtextxy(120, 300, msg3);

                setcolor(LIGHTRED);
                rectangle(100, 360, 200, 400);
                outtextxy(120, 370, (char*)"SAVE");
                setcolor(LIGHTGREEN);
                rectangle(220, 360, 320, 400);
                outtextxy(230, 370, (char*)"RESTART");

                bool restartGame = false;
                while (!restartGame) {
                    if (kbhit()) {
                        int key = getch();
                        if (key == 9) restartGame = true;
                    }
                    if (ismouseclick(WM_LBUTTONDOWN)) {
                        int mx,my;
                        getmouseclick(WM_LBUTTONDOWN,mx,my);
                        if (mx>=100 && mx<=200 && my>=360 && my<=400) {
                            outtextxy(120, 420, (char*)"Best Score saved!");
                            delay(1000);
                        }
                        if (mx>=220 && mx<=320 && my>=360 && my<=400) {
                            restartGame = true;
                        }
                    }
                    delay(50);
                }

                level = 1;
                target = 1000;
                score = 0;
                stageCleared = false;
                initGrid();
            }
        }

        delay(50);
    }

    closegraph();
    return 0;
}
