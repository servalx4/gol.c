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
        printf("\ngol.c v2.5\n");
        printf("originally by John Conway, written in C by plasmaCotton\n");
        printf("\nInstructions:\n");
        printf("--help | show this menu\n");
        printf("k/K    | pause/unpause\n");
        printf("q/Q    | quit\n");
        printf("up     | delay up\n");
        printf("down   | delay down\n");
        printf("s/S    | one step - MUST BE PAUSED\n");
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

    void UpdateGame() {
        for (int y = 0; y < gameHeight; y++) {
            for (int x = 0; x < gameWidth; x++) {

                xyMatrix[x][y].surround = CountSurroundingCells(x, y);

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
                if (xyMatrix[x][y].type == 1) { attron(COLOR_PAIR(xyMatrix[x][y].surround)); }
                printw("%c", CHARLIST[xyMatrix[x][y].type]);
                if (xyMatrix[x][y].type == 1) { attroff(COLOR_PAIR(xyMatrix[x][y].surround)); }

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

    while (1) { //game loop
      if (paused == 0) { clear(); } //clear the screen
      move(0, 0); //move drawing cursor to the top left
      int drawX = 1; //reset drawing X coordinate
      int drawY = 1; //reset drawing Y coordinate

      //draw the game itself
      if (paused == 0) {
          UpdateGame();
      }

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
              if (paused == 1) { UpdateGame(); delay -= 10000; } //
          case KEY_UP:
              delay +=10000;
              break;
          case KEY_DOWN:
              delay -=10000;
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
