#define _POSIX_C_SOURCE 200809L

#include <curses.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define DX 7
#define DY 3
#define MIN_FRAME_HEIGHT 3
#define MIN_FRAME_WIDTH 3

typedef struct {
    char **items;
    size_t count;
} Lines;

static void free_lines(Lines *lines) {
    if (lines == NULL) {
        return;
    }

    for (size_t i = 0; i < lines->count; i++) {
        free(lines->items[i]);
    }
    free(lines->items);
    lines->items = NULL;
    lines->count = 0;
}

static int add_line(Lines *lines, char *line, size_t *capacity) {
    if (lines->count == *capacity) {
        size_t new_capacity = (*capacity == 0) ? 16 : *capacity * 2;
        char **new_items = realloc(lines->items, new_capacity * sizeof(*new_items));

        if (new_items == NULL) {
            return -1;
        }

        lines->items = new_items;
        *capacity = new_capacity;
    }

    lines->items[lines->count] = line;
    lines->count++;
    return 0;
}

static int read_lines(const char *path, Lines *lines) {
    FILE *file = fopen(path, "r");
    char *line = NULL;
    size_t line_capacity = 0;
    size_t capacity = 0;
    ssize_t length;

    if (file == NULL) {
        return -1;
    }

    while ((length = getline(&line, &line_capacity, file)) != -1) {
        while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
            line[length - 1] = '\0';
            length--;
        }

        char *copy = malloc((size_t)length + 1);
        if (copy == NULL) {
            free(line);
            fclose(file);
            free_lines(lines);
            return -2;
        }

        memcpy(copy, line, (size_t)length + 1);
        if (add_line(lines, copy, &capacity) != 0) {
            free(copy);
            free(line);
            fclose(file);
            free_lines(lines);
            return -2;
        }
    }

    free(line);
    fclose(file);
    return 0;
}

static int parse_positive_int(const char *text, int *value) {
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed <= 0 || parsed > INT_MAX) {
        return -1;
    }

    *value = (int)parsed;
    return 0;
}

static int clamp_int(int value, int min_value, int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static int max_top_line(const Lines *lines, int window_height) {
    if (lines->count <= (size_t)window_height) {
        return 0;
    }
    return (int)(lines->count - (size_t)window_height);
}

static int max_line_length(const Lines *lines) {
    size_t max_length = 0;

    for (size_t i = 0; i < lines->count; i++) {
        size_t length = strlen(lines->items[i]);

        if (length > max_length) {
            max_length = length;
        }
    }

    return (max_length > (size_t)INT_MAX) ? INT_MAX : (int)max_length;
}

static void draw_frame(WINDOW *frame) {
    int height;
    int width;
    const char *title = "Frame";

    getmaxyx(frame, height, width);
    (void)height;
    werase(frame);
    box(frame, 0, 0);
    mvwaddstr(frame, 0, (width - (int)strlen(title)) / 2, title);
    wrefresh(frame);
}

static void draw_view(WINDOW *win, const Lines *lines, int top_line, int left_column) {
    int height;
    int width;

    getmaxyx(win, height, width);
    werase(win);

    for (int row = 0; row < height; row++) {
        size_t line_index = (size_t)(top_line + row);

        if (line_index < lines->count) {
            const char *line = lines->items[line_index];
            size_t length = strlen(line);

            if ((size_t)left_column < length) {
                mvwaddnstr(win, row, 0, line + left_column, width);
            }
        }
    }

    wrefresh(win);
}

static void usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <file> [height width]\n", program_name);
}

int main(int argc, char *argv[]) {
    Lines lines = {0};
    WINDOW *frame;
    WINDOW *win;
    int requested_height = 0;
    int requested_width = 0;
    int frame_height;
    int frame_width;
    int frame_y;
    int frame_x;
    int window_height;
    int window_width;
    int top_line = 0;
    int left_column = 0;
    int max_column;
    int ch;
    int read_result;

    if (argc != 2 && argc != 4) {
        usage(argv[0]);
        return 1;
    }

    if (argc == 4 &&
        (parse_positive_int(argv[2], &requested_height) != 0 ||
         parse_positive_int(argv[3], &requested_width) != 0)) {
        usage(argv[0]);
        return 1;
    }

    read_result = read_lines(argv[1], &lines);
    if (read_result == -1) {
        perror(argv[1]);
        return 1;
    }
    if (read_result == -2) {
        fprintf(stderr, "Not enough memory to read '%s'\n", argv[1]);
        return 1;
    }

    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    frame_height = (requested_height > 0) ? requested_height : LINES - 2 * DY;
    frame_width = (requested_width > 0) ? requested_width : COLS - 2 * DX;
    frame_height = clamp_int(frame_height, MIN_FRAME_HEIGHT, LINES);
    frame_width = clamp_int(frame_width, MIN_FRAME_WIDTH, COLS);
    frame_y = (requested_height > 0) ? (LINES - frame_height) / 2 : DY;
    frame_x = (requested_width > 0) ? (COLS - frame_width) / 2 : DX;
    window_height = frame_height - 2;
    window_width = frame_width - 2;
    max_column = max_line_length(&lines);

    mvprintw(0, 0, "Window: %dx%d", frame_height, frame_width);
    refresh();

    frame = newwin(frame_height, frame_width, frame_y, frame_x);
    win = newwin(window_height, window_width, frame_y + 1, frame_x + 1);
    keypad(win, TRUE);

    draw_frame(frame);
    draw_view(win, &lines, top_line, left_column);

    while ((ch = wgetch(win)) != 27) {
        switch (ch) {
            case KEY_RIGHT:
                left_column = clamp_int(left_column + 1, 0, max_column);
                break;
            case KEY_LEFT:
                left_column = clamp_int(left_column - 1, 0, max_column);
                break;
            case KEY_NPAGE:
                top_line = clamp_int(top_line + window_height, 0, max_top_line(&lines, window_height));
                break;
            case KEY_PPAGE:
                top_line = clamp_int(top_line - window_height, 0, max_top_line(&lines, window_height));
                break;
            case ' ':
            case KEY_DOWN:
                top_line = clamp_int(top_line + 1, 0, max_top_line(&lines, window_height));
                break;
            case KEY_UP:
                top_line = clamp_int(top_line - 1, 0, max_top_line(&lines, window_height));
                break;
            default:
                break;
        }

        draw_view(win, &lines, top_line, left_column);
    }

    delwin(win);
    delwin(frame);
    endwin();
    free_lines(&lines);
    return 0;
}
