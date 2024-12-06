#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>

int main(int argc, char *argv[]) {
    //check for the --help flag
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        printf("\ngol.c v3\n");
        printf("originally by John Conway, written in C by sunlitHorizon\n");
        printf("\nInstructions:\n");
        printf("--help | show this menu\n");
        printf("k/K    | toggle paused/unpaused mode\n");
        printf("q/Q    | quit\n");
        printf("up     | delay up / cursor up in paused mode\n");
        printf("down   | delay down / cursor down in paused mode\n");
        printf("left   | cursor left in paused mode\n");
        printf("right  | cursor right in paused mode\n");
        printf("space  | toggle on/off cell in paused mode\n");
        printf("s/S    | one step - MUST BE PAUSED\n");
        printf("c/C    | clear dots\n");
        return 0;
    }

    typedef struct {
        int type;
        int surround;
    } cell;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    srand((int)time(NULL));

    const char CHARLIST[] = " #";
    const int gameWidth = w.ws_col - 1;
    const int gameHeight = w.ws_row;
    int cursorXY[] = {0, 0};
    int paused = 0;
    int delay = 50000;
    cell xyMatrix[gameWidth][gameHeight];
    cell NextXYmatrix[gameWidth][gameHeight];

    for (int i = 0; i < gameWidth; i++) { //make sure the matrix isnt fucked up
        for (int j = 0; j < gameHeight; j++) {
            if ((rand() + 1) % 5 == 0) { xyMatrix[i][j].type = 1; } else { xyMatrix[i][j].type = 0; }
            xyMatrix[i][j].surround = 0;
            NextXYmatrix[i][j].type = 0;
            NextXYmatrix[i][j].surround = 0;
        }
    }

    int CountSurroundingCells(int x, int y) {
        int counted = 0;
        int top = (y - 1 + gameHeight) % gameHeight;
        int bottom = (y + 1) % gameHeight;
        int left = (x - 1 + gameWidth) % gameWidth;
        int right = (x + 1) % gameWidth;

        if (xyMatrix[x][top].type == 1) counted += 1;
        if (xyMatrix[right][top].type == 1) counted += 1;
        if (xyMatrix[left][top].type == 1) counted += 1;
        if (xyMatrix[right][y].type == 1) counted += 1;
        if (xyMatrix[left][y].type == 1) counted += 1;
        if (xyMatrix[x][bottom].type == 1) counted += 1;
        if (xyMatrix[right][bottom].type == 1) counted += 1;
        if (xyMatrix[left][bottom].type == 1) counted += 1;

        return counted;
    }

    void UpdateGame(int active) {
        for (int y = 0; y < gameHeight; y++) {
            for (int x = 0; x < gameWidth; x++) {

                xyMatrix[x][y].surround = CountSurroundingCells(x, y);

                if (active == 0) {

                if (xyMatrix[x][y].surround < 2 && xyMatrix[x][y].type == 1) {
                    NextXYmatrix[x][y].type = 0;
                }
                if (xyMatrix[x][y].surround == 2 && xyMatrix[x][y].type == 1 || xyMatrix[x][y].surround == 3 && xyMatrix[x][y].type == 1) {
                    NextXYmatrix[x][y].type = 1;
                }
                if (xyMatrix[x][y].surround > 3 && xyMatrix[x][y].type == 1) {
                    NextXYmatrix[x][y].type = 0;
                }
                if (xyMatrix[x][y].surround == 3 && xyMatrix[x][y].type == 0) {
                    NextXYmatrix[x][y].type = 1;
                }

                }

                if (xyMatrix[x][y].type == 1) { attron(COLOR_PAIR(xyMatrix[x][y].surround)); }
                if (x == cursorXY[0] && y == cursorXY[1] && active == 1) { attron(COLOR_PAIR(9)); }
                printw("%c", CHARLIST[xyMatrix[x][y].type]);
                if (xyMatrix[x][y].type == 1) { attroff(COLOR_PAIR(xyMatrix[x][y].surround)); }
                if (x == cursorXY[0] && y == cursorXY[1]) { attroff(COLOR_PAIR(9)); }

            }
            printw("\n");
        }
    }

    //initialize ncurses
    initscr();
    start_color();
    timeout(0);
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    init_pair(0, COLOR_BLACK, COLOR_BLACK);
    init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_RED, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_WHITE);
    init_pair(8, COLOR_WHITE, COLOR_BLACK);
    init_pair(9, COLOR_BLACK, COLOR_WHITE);

    while (1) { //game loop
      move(0, 0); //move drawing cursor to the top left
      int drawX = 1; //reset drawing X coordinate
      int drawY = 1; //reset drawing Y coordinate

      //draw the game itself
      UpdateGame(paused);

      int key = getch(); //read dumbass input

      switch (key) {

          case 'k':
          case 'K':
              if (paused == 0) {
                  paused = 1;
              } else paused = 0;
              break;
          case 's':
          case 'S':
              if (paused == 1) { UpdateGame(0); }
              break;
          case 'c':
          case 'C':
              for (int y = 0; y < gameHeight; y++) {
                  for (int x = 0; x < gameWidth; x++) {
                      xyMatrix[x][y].type = 0;
                      NextXYmatrix[x][y].type = 0;
                  }
              }
              refresh();
              break;
          case KEY_UP:
              if (paused == 0) { delay +=10000; }
              else if (cursorXY[1] > 0) { cursorXY[1] -= 1; }
              break;
          case KEY_DOWN:
              if (paused == 0) { delay -=10000; }
              else if (cursorXY[1] < gameHeight) { cursorXY[1] += 1; }
              break;
          case KEY_RIGHT:
              if (paused == 1 && cursorXY[0] < gameWidth - 1) { cursorXY[0] += 1; }
              break;
          case KEY_LEFT:
              if (paused == 1 && cursorXY[0] > 0) { cursorXY[0] -= 1; }
              break;
          case 32:
              if (paused == 1) {
                  if (NextXYmatrix[cursorXY[0]][cursorXY[1]].type == 1) { NextXYmatrix[cursorXY[0]][cursorXY[1]].type = 0; } else { NextXYmatrix[cursorXY[0]][cursorXY[1]].type = 1; }
              }
              break;
      }

      if (key == 'q' || key == 'Q') { //check if player wants to get out of the basement
          break;
      }
      for (int y = 0; y < gameHeight; y++) {
          for (int x = 0; x < gameWidth; x++) {
              xyMatrix[x][y].type = NextXYmatrix[x][y].type;
          }
      }

      refresh();
      usleep(delay);
      }

  endwin();

  return 0; //if this ever happens, my brain has at least 2 braincells

}
