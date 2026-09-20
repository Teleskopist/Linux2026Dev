#include <curses.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>

#define DX 7
#define DY 3

char* get_line(FILE* file) {
    static char line[256];
    
    if (file == NULL) {
        return "";
    }
    if (fgets(line, sizeof(line), file) == NULL) {
        return "\0";
    }
    // Remove newline character if present
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
    return line;
}

int main(int argc, char *argv[]) {
    WINDOW *frame, *win;
    int c = 0;

    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE* file = fopen(argv[1], "r");

    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    printw("Окно:");
    refresh();

    frame = newwin(LINES - 2*DY, COLS - 2*DX, DY, DX);
    box(frame, 0, 0);
    mvwaddstr(frame, 0, (int)((COLS - 2*DX - 5) / 2), "Рамка");
    wrefresh(frame);

    win = newwin(LINES - 2*DY - 2, COLS - 2*DX-2, DY+1, DX+1);
    keypad(win, TRUE);
    scrollok (win, TRUE);
    char* buffer[LINES - 2*DY - 2];
    for (int i = 0; i < LINES - 2*DY - 2; i++) {
        char* line = get_line(file);
        buffer[i] = line;
        wprintw(win, "%s\n", line);
    }
    while((c = wgetch(win)) != 27) {
        if (c == 32) {
            char* line = get_line(file);
            if (line[0] == '\0') {
                wprintw(win, "\0");
            } else {
                wprintw(win, "%s\n", line);
            }
        } else if (c == KEY_PPAGE) {
            wscrl(win, -1);
        } else if (c == KEY_NPAGE) {
            wscrl(win, 1);
        }
    }
    if (file != NULL) {
        fclose(file);
    }
    delwin(win);
    delwin(frame);
    endwin();
    return 0;
}