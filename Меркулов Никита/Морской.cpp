#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #define CLEAR_SCREEN() system("cls")
#else
    #define CLEAR_SCREEN() printf("\033[H\033[J")
#endif

#define N 10
#define DIRS 4
#define BUF_LEN 128

char my_board[N][N];
char enemy_board[N][N];
int fleet[N] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void wait_enter(void) {
    printf("\nНажмите Enter, чтобы продолжить...");
    clear_input();
}

int can_place(char b[N][N], int x, int y, int len, int dx, int dy) {
    if (x < 0 || y < 0 || (!dx && !dy) || x + dx * (len - 1) >= N || y + dy * (len - 1) >= N)
        return 0;

    for (int k = 0; k < len; k++) {
        int cx = x + dx * k;
        int cy = y + dy * k;
        for (int ny = cy - 1; ny <= cy + 1; ny++) {
            for (int nx = cx - 1; nx <= cx + 1; nx++) {
                if (nx >= 0 && nx < N && ny >= 0 && ny < N) {
                    if (b[ny][nx] != '~') return 0;
                }
            }
        }
    }
    return 1;
}

void place_ship(char b[N][N], int x, int y, int len, int dx, int dy) {
    for (int k = 0; k < len; k++) {
        b[y + dy * k][x + dx * k] = '#';
    }
}

void random_place(char b[N][N]) {
    while (1) {
        memset(b, '~', N * N);
        int placed_all = 1;
        for (int i = 0; i < N; i++) {
            int attempts = 0;
            int placed = 0;
            while (attempts < 200) {
                int x = rand() % N;
                int y = rand() % N;
                int dir = rand() % 2;
                int dx = (dir == 0);
                int dy = (dir == 1);
                if (can_place(b, x, y, fleet[i], dx, dy)) {
                    place_ship(b, x, y, fleet[i], dx, dy);
                    placed = 1;
                    break;
                }
                attempts++;
            }
            if (!placed) {
                placed_all = 0;
                break;
            }
        }
        if (placed_all) break;
    }
}

void print_board(char b[N][N], int hide, const char* title) {
    printf("\n   === %s ===\n", title);
    printf("   1 2 3 4 5 6 7 8 9 10\n");
    for (int y = 0; y < N; y++) {
        printf("%2d ", y + 1);
        for (int x = 0; x < N; x++) {
            char c = b[y][x];
            if (hide && c == '#') c = '~';
            printf("%c ", c);
        }
        printf("\n");
    }
}

void show_game(int hide_enemy) {
    CLEAR_SCREEN();
    print_board(enemy_board, hide_enemy, "ПОЛЕ ПРОТИВНИКА");
    print_board(my_board, 0, "ВАШЕ ПОЛЕ");
    printf("\n~ вода | # корабль | * промах | X ранен | @ убит\n");
}

int is_ship_sunk(char b[N][N], int x, int y, int vis[N][N]) {
    if (x < 0 || x >= N || y < 0 || y >= N || vis[y][x]) return 1;
    if (b[y][x] == '#') return 0;
    if (b[y][x] != 'X') return 1;
    vis[y][x] = 1;
    return is_ship_sunk(b, x + 1, y, vis) &&
           is_ship_sunk(b, x - 1, y, vis) &&
           is_ship_sunk(b, x, y + 1, vis) &&
           is_ship_sunk(b, x, y - 1, vis);
}

void mark_sunk(char b[N][N], int x, int y) {
    if (x < 0 || x >= N || y < 0 || y >= N || b[y][x] != 'X') return;
    b[y][x] = '@';

    for (int ny = y - 1; ny <= y + 1; ny++) {
        for (int nx = x - 1; nx <= x + 1; nx++) {
            if (nx >= 0 && nx < N && ny >= 0 && ny < N) {
                if (b[ny][nx] == '~') {
                    b[ny][nx] = '*';
                }
            }
        }
    }

    mark_sunk(b, x + 1, y);
    mark_sunk(b, x - 1, y);
    mark_sunk(b, x, y + 1);
    mark_sunk(b, x, y - 1);
}

int count_ship_len(char b[N][N], int x, int y, int vis[N][N]) {
    if (x < 0 || x >= N || y < 0 || y >= N || vis[y][x] || b[y][x] != 'X') return 0;
    vis[y][x] = 1;
    return 1 + count_ship_len(b, x + 1, y, vis)
             + count_ship_len(b, x - 1, y, vis)
             + count_ship_len(b, x, y + 1, vis)
             + count_ship_len(b, x, y - 1, vis);
}

int has_unexplored_gap(char b[N][N], int x, int y) {
    int dx_arr[DIRS] = { 1, -1, 0, 0 };
    int dy_arr[DIRS] = { 0, 0, 1, -1 };
    for (int d = 0; d < DIRS; d++) {
        int nx1 = x + dx_arr[d];
        int ny1 = y + dy_arr[d];
        int nx2 = x + dx_arr[d] * 2;
        int ny2 = y + dy_arr[d] * 2;
        if (nx2 >= 0 && nx2 < N && ny2 >= 0 && ny2 < N) {
            if (b[ny1][nx1] == '~' && b[ny2][nx2] == 'X') {
                return 1;
            }
        }
    }
    return 0;
}

int shoot(char b[N][N], int x, int y, int* hits) {
    if (b[y][x] == '*' || b[y][x] == 'X' || b[y][x] == '@') return -1;
    if (b[y][x] == '#') {
        b[y][x] = 'X';
        (*hits)++;
        int vis[N][N] = { 0 };
        if (is_ship_sunk(b, x, y, vis)) {
            mark_sunk(b, x, y);
            return 2;
        }
        return 1;
    } else {
        b[y][x] = '*';
        return 0;
    }
}

int shoot_human_enemy(char b[N][N], int x, int y, int* hits) {
    if (b[y][x] != '~') return -1;
    int res = 0;
    while (res < 1 || res > 3) {
        printf("Результат выстрела (1 - Мимо, 2 - Ранил, 3 - Убил): ");
        if (scanf("%d", &res) != 1) {
            printf("Ошибка: введите число от 1 до 3.\n");
        }
        clear_input();
    }
    if (res == 1) {
        b[y][x] = '*';
        return 0;
    } else if (res == 2) {
        b[y][x] = 'X';
        int vis[N][N] = { 0 };
        if (count_ship_len(b, x, y, vis) >= 4) {
            printf("[!] 4-я палуба подряд — корабль автоматически считается убитым!\n");
            (*hits)++;
            mark_sunk(b, x, y);
            return 2;
        }
        (*hits)++;
        return 1;
    } else {
        if (has_unexplored_gap(b, x, y)) {
            printf("[!] Между ранеными палубами есть нераскрытая клетка — статус изменен на «Ранил».\n");
            b[y][x] = 'X';
            (*hits)++;
            return 1;
        }
        b[y][x] = 'X';
        (*hits)++;
        mark_sunk(b, x, y);
        return 2;
    }
}

void bot_choose_target(char b[N][N], int* out_x, int* out_y) {
    int dx_arr[DIRS] = { 1, -1, 0, 0 };
    int dy_arr[DIRS] = { 0, 0, 1, -1 };

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            if (b[y][x] == 'X') {
                for (int d = 0; d < DIRS; d++) {
                    int nx = x + dx_arr[d];
                    int ny = y + dy_arr[d];
                    if (nx >= 0 && nx < N && ny >= 0 && ny < N && b[ny][nx] == 'X') {
                        int tx = x - dx_arr[d];
                        int ty = y - dy_arr[d];
                        while (tx >= 0 && tx < N && ty >= 0 && ty < N && b[ty][tx] == 'X') {
                            tx -= dx_arr[d];
                            ty -= dy_arr[d];
                        }
                        if (tx >= 0 && tx < N && ty >= 0 && ty < N && (b[ty][tx] == '~' || b[ty][tx] == '#')) {
                            *out_x = tx;
                            *out_y = ty;
                            return;
                        }
                    }
                }
            }
        }
    }

    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            if (b[y][x] == 'X') {
                for (int d = 0; d < DIRS; d++) {
                    int nx = x + dx_arr[d];
                    int ny = y + dy_arr[d];
                    if (nx >= 0 && nx < N && ny >= 0 && ny < N && (b[ny][nx] == '~' || b[ny][nx] == '#')) {
                        *out_x = nx;
                        *out_y = ny;
                        return;
                    }
                }
            }
        }
    }

    do {
        *out_x = rand() % N;
        *out_y = rand() % N;
    } while (b[*out_y][*out_x] != '~' && b[*out_y][*out_x] != '#');
}
int main(void) {
#ifdef _WIN32
    system("chcp 1251 > nul");
#endif
    srand((unsigned int)time(NULL));
    memset(my_board, '~', sizeof(my_board));
    memset(enemy_board, '~', sizeof(enemy_board));

    int mode = 0;
    while (mode != 1 && mode != 2) {
        CLEAR_SCREEN();
        printf("Выберите соперника:\n1 - Бот\n2 - Человек\nВаш выбор: ");
        if (scanf("%d", &mode) != 1) mode = 0;
        clear_input();
    }

    if (mode == 1) {
        random_place(enemy_board);
    }

    int choice = 0;
    while (choice != 1 && choice != 2) {
        CLEAR_SCREEN();
        printf("Расстановка ваших кораблей:\n1 - Случайно\n2 - Вручную\nВаш выбор: ");
        if (scanf("%d", &choice) != 1) choice = 0;
        clear_input();
    }

    if (choice == 1) {
        random_place(my_board);
    } else {
        int x, y, d;
        char error_msg[BUF_LEN] = "";
        for (int i = 0; i < N; ) {
            CLEAR_SCREEN();
            print_board(my_board, 0, "ВАШЕ ПОЛЕ");
            if (*error_msg != '\0') {
                printf("\n[!] %s\n", error_msg);
                *error_msg = '\0';
            }

            if (fleet[i] == 1) {
                printf("\nКорабль 1-палубный. Введите координаты (X Y): ");
                if (scanf("%d %d", &x, &y) != 2) {
                    clear_input();
                    snprintf(error_msg, sizeof(error_msg), "Некорректный ввод! Введите два числа через пробел.");
                    continue;
                }
                d = 1;
            } else {
                printf("\nКорабль %d-палубный. Введите X Y и направление (1-гор / 2-верт): ", fleet[i]);
                if (scanf("%d %d %d", &x, &y, &d) != 3) {
                    clear_input();
                    snprintf(error_msg, sizeof(error_msg), "Некорректный ввод! Введите три числа через пробел.");
                    continue;
                }
            }
            clear_input();

            x--; y--;
            int dx = (d == 1), dy = (d == 2);
            if (!dx && !dy) {
                snprintf(error_msg, sizeof(error_msg), "Направление должно быть 1 (горизонтально) или 2 (вертикально)!");
                continue;
            }
            if (can_place(my_board, x, y, fleet[i], dx, dy)) {
                place_ship(my_board, x, y, fleet[i], dx, dy);
                i++;
            } else {
                snprintf(error_msg, sizeof(error_msg), "Нельзя поставить корабль здесь (выход за край или касание другого корабля)!");
            }
        }
    }

    int turn = 1;
    if (mode == 2) {
        int first = 0;
        while (first != 1 && first != 2) {
            CLEAR_SCREEN();
            printf("Кто ходит первым?\n1 - Вы\n2 - Соперник\nВаш выбор: ");
            if (scanf("%d", &first) != 1) first = 0;
            clear_input();
        }
        turn = (first == 1) ? 1 : 0;
    }

    int my_hits = 0, enemy_hits = 0;
    char status_msg[BUF_LEN];
    snprintf(status_msg, sizeof(status_msg), "%s", (turn == 1) ? "Игра началась! Ваш ход." : "Игра началась! Первым ходит соперник.");

    while (my_hits < 20 && enemy_hits < 20) {
        show_game(mode == 1 ? 1 : 0);
        if (*status_msg != '\0') {
            printf("\n>> %s\n", status_msg);
        }

        if (turn == 1) {
            int x, y;
            printf("\nВАШ ХОД. Введите координаты выстрела (X Y): ");
            if (scanf("%d %d", &x, &y) != 2) {
                clear_input();
                snprintf(status_msg, sizeof(status_msg), "Ошибка ввода! Введите два числа (X и Y) от 1 до 10.");
                continue;
            }
            clear_input();

            x--; y--;
            if (x < 0 || x >= N || y < 0 || y >= N) {
                snprintf(status_msg, sizeof(status_msg), "Координаты должны быть от 1 до 10!");
                continue;
            }

            int res = (mode == 1) ? shoot(enemy_board, x, y, &my_hits)
                                  : shoot_human_enemy(enemy_board, x, y, &my_hits);

            if (res == -1) {
                snprintf(status_msg, sizeof(status_msg), "Вы уже стреляли в эту клетку!");
                continue;
            } else if (res == 0) {
                snprintf(status_msg, sizeof(status_msg), "Вы выстрелили в (%d, %d) — МИМО!", x + 1, y + 1);
                turn = 0;
            } else if (res == 1) {
                snprintf(status_msg, sizeof(status_msg), "Вы выстрелили в (%d, %d) — РАНИЛ! Стреляйте еще.", x + 1, y + 1);
            } else if (res == 2) {
                snprintf(status_msg, sizeof(status_msg), "Вы выстрелили в (%d, %d) — УБИЛ! Стреляйте еще.", x + 1, y + 1);
            }
        } else {
            if (mode == 1) {
                int x, y;
                bot_choose_target(my_board, &x, &y);
                int res = shoot(my_board, x, y, &enemy_hits);

                if (res == 0) {
                    snprintf(status_msg, sizeof(status_msg), "Бот выстрелил в (%d, %d) — МИМО! Теперь ваш ход.", x + 1, y + 1);
                    turn = 1;
                } else if (res == 1) {
                    snprintf(status_msg, sizeof(status_msg), "Бот выстрелил в (%d, %d) — РАНИЛ ваш корабль!", x + 1, y + 1);
                    show_game(1);
                    printf("\n>> %s\n", status_msg);
                    wait_enter();
                } else if (res == 2) {
                    snprintf(status_msg, sizeof(status_msg), "Бот выстрелил в (%d, %d) — УБИЛ ваш корабль!", x + 1, y + 1);
                    show_game(1);
                    printf("\n>> %s\n", status_msg);
                    if (enemy_hits < 20) wait_enter();
                }
            } else {
                int x, y;
                printf("\nХОД СОПЕРНИКА. Куда он выстрелил? (X Y): ");
                if (scanf("%d %d", &x, &y) != 2) {
                    clear_input();
                    snprintf(status_msg, sizeof(status_msg), "Ошибка ввода! Введите два числа (X и Y) от 1 до 10.");
                    continue;
                }
                clear_input();

                x--; y--;
                if (x < 0 || x >= N || y < 0 || y >= N) {
                    snprintf(status_msg, sizeof(status_msg), "Координаты должны быть от 1 до 10!");
                    continue;
                }

                int res = shoot(my_board, x, y, &enemy_hits);
                if (res == -1) {
                    snprintf(status_msg, sizeof(status_msg), "Соперник уже стрелял в эту клетку!");
                    continue;
                } else if (res == 0) {
                    snprintf(status_msg, sizeof(status_msg), "Соперник выстрелил в (%d, %d) — МИМО! Ваш ход.", x + 1, y + 1);
                    turn = 1;
                } else if (res == 1) {
                    snprintf(status_msg, sizeof(status_msg), "Соперник выстрелил в (%d, %d) — РАНИЛ! Его следующий выстрел.", x + 1, y + 1);
                } else if (res == 2) {
                    snprintf(status_msg, sizeof(status_msg), "Соперник выстрелил в (%d, %d) — УБИЛ! Его следующий выстрел.", x + 1, y + 1);
                }
            }
        }
    }

    show_game(0);
    if (my_hits == 20) {
        printf("\n=== ПОЗДРАВЛЯЕМ! ВЫ ПОБЕДИЛИ! ===\n");
    } else {
        printf("\n=== ВЫ ПРОИГРАЛИ. ВСЕ ВАШИ КОРАБЛИ ПОТОПЛЕНЫ. ===\n");
    }

    return 0;
}
