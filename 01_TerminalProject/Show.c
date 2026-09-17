#include <curses.h>
#include <locale.h>

#define DX 7
#define DY 3

int main(int argc, char *argv[]) {
        WINDOW *frame, *win;
        int c = 0;

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
        while((c = wgetch(win)) != 27)
                wprintw(win, "\n%d: %s", c, keyname(c));
        delwin(win);
        delwin(frame);
        endwin();
        return 0;
}