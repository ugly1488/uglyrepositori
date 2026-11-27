#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char my_board[10][10];
char enemy_board[10][10];
int fleet[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

int can_place(char b[10][10], int x, int y, int len, int dx, int dy) {
    if (x < 0 || y < 0 || (!dx && !dy) || x + dx * (len - 1) > 9 || y + dy * (len - 1) > 9)
        return 0;
    for (int k = 0; k < len; k++) {
        int cx = x + dx * k;
        int cy = y + dy * k;
        for (int ny = cy - 1; ny <= cy + 1; ny++) {
            for (int nx = cx - 1; nx <= cx + 1; nx++) {
                if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10) {
                    if (b[ny][nx] != '~') return 0;
                }
            }
        }
    }
    return 1;
}

void place_ship(char b[10][10], int x, int y, int len, int dx, int dy) {
    for (int k = 0; k < len; k++) {
        b[y + dy * k][x + dx * k] = '#';
    }
}

void random_place(char b[10][10]) {
    memset(b, '~', 100);
    for (int i = 0; i < 10; ) {
        int x = rand() % 10;
        int y = rand() % 10;
        int dir = rand() % 2;
        int dx = (dir == 0);
        int dy = (dir == 1);
        if (can_place(b, x, y, fleet[i], dx, dy)) {
            place_ship(b, x, y, fleet[i], dx, dy);
            i++;
        }
    }
}

void print_board(char b[10][10], int hide, const char* title) {
    printf("\n   === %s ===\n", title);
    printf("   1 2 3 4 5 6 7 8 9 10\n");
    for (int y = 0; y < 10; y++) {
        printf("%2d ", y + 1);
        for (int x = 0; x < 10; x++) {
            char c = b[y][x];
            if (hide && c == '#') c = '~';
            printf("%c ", c);
        }
        printf("\n");
    }
}

void show_game(int hide_enemy) {
    system("cls");
    print_board(enemy_board, hide_enemy, "ПОЛЕ ПРОТИВНИКА");
    print_board(my_board, 0, "ВАШЕ ПОЛЕ");
    printf("\n~ вода | # корабль | * промах | X ранен | @ убит\n");
}

int is_ship_sunk(char b[10][10], int x, int y, int vis[10][10]) {
    if (x < 0 || x > 9 || y < 0 || y > 9 || vis[y][x]) return 1;
    if (b[y][x] == '#') return 0;
    if (b[y][x] != 'X') return 1;
    vis[y][x] = 1;
    return is_ship_sunk(b, x + 1, y, vis) &&
        is_ship_sunk(b, x - 1, y, vis) &&
        is_ship_sunk(b, x, y + 1, vis) &&
        is_ship_sunk(b, x, y - 1, vis);
}

void mark_sunk(char b[10][10], int x, int y) {
    if (x < 0 || x > 9 || y < 0 || y > 9 || b[y][x] != 'X') return;
    b[y][x] = '@';
    mark_sunk(b, x + 1, y);
    mark_sunk(b, x - 1, y);
    mark_sunk(b, x, y + 1);
    mark_sunk(b, x, y - 1);
}

int shoot(char b[10][10], int x, int y, int* hits) {
    if (b[y][x] == '*' || b[y][x] == 'X' || b[y][x] == '@') return -1;
    if (b[y][x] == '#') {
        b[y][x] = 'X';
        (*hits)++;
        int vis[10][10] = { 0 };
        if (is_ship_sunk(b, x, y, vis)) {
            mark_sunk(b, x, y);
        }
        return 1;
    }
    else {
        b[y][x] = '*';
        return 0;
    }
}

int shoot_human_enemy(char b[10][10], int x, int y, int* hits) {
    if (b[y][x] != '~') return -1;

    int res = 0;
    while (res < 1 || res > 3) {
        printf("Результат выстрела (1 - Мимо, 2 - Ранил, 3 - Убил): ");
        if (scanf("%d", &res) != 1) {
            scanf("%*[^\n]");
            getchar();
        }
    }

    if (res == 1) {
        b[y][x] = '*';
        return 0;
    }
    else if (res == 2) {
        b[y][x] = 'X';
        (*hits)++;
        return 1;
    }
    else {
        b[y][x] = 'X';
        (*hits)++;
        mark_sunk(b, x, y);
        return 1;
    }
}

int main(void) {
    system("chcp 1251 > nul");
    srand((unsigned int)time(NULL));
    memset(my_board, '~', sizeof(my_board));
    memset(enemy_board, '~', sizeof(enemy_board));

    int mode = 0;
    while (mode != 1 && mode != 2) {
        system("cls");
        printf("Выберите соперника:\n1 - Бот\n2 - Человек\nВаш выбор: ");
        if (scanf("%d", &mode) != 1) {
            scanf("%*[^\n]");
            getchar();
        }
    }

    if (mode == 1) {
        random_place(enemy_board);
    }

    int choice = 0;
    while (choice != 1 && choice != 2) {
        system("cls");
        printf("Расстановка ваших кораблей:\n1 - Случайно\n2 - Вручную\nВаш выбор: ");
        if (scanf("%d", &choice) != 1) {
            scanf("%*[^\n]");
            getchar();
        }
    }

    if (choice == 1) {
        random_place(my_board);
    }
    else {
        int x, y, d;
        for (int i = 0; i < 10; ) {
            system("cls");
            print_board(my_board, 0, "ВАШЕ ПОЛЕ");
            printf("\nКорабль %d-пал. (X Y 1-гор/2-верт): ", fleet[i]);
            if (scanf("%d %d %d", &x, &y, &d) != 3) {
                scanf("%*[^\n]");
                getchar();
                continue;
            }
            x--; y--;
            int dx = (d == 1), dy = (d == 2);
            if (can_place(my_board, x, y, fleet[i], dx, dy)) {
                place_ship(my_board, x, y, fleet[i], dx, dy);
                i++;
            }
        }
    }

    int turn = 1;
    if (mode == 2) {
        int first = 0;
        while (first != 1 && first != 2) {
            system("cls");
            printf("Кто ходит первым?\n1 - Вы\n2 - Соперник\nВаш выбор: ");
            if (scanf("%d", &first) != 1) {
                scanf("%*[^\n]");
                getchar();
            }
        }
        turn = (first == 1) ? 1 : 0;
    }

    int my_hits = 0, enemy_hits = 0;
    while (my_hits < 20 && enemy_hits < 20) {
        show_game(mode == 1 ? 1 : 0);

        if (turn == 1) {
            int x, y;
            printf("\nВАШ ХОД. Введите координаты выстрела (X Y): ");
            if (scanf("%d %d", &x, &y) != 2) {
                scanf("%*[^\n]");
                getchar();
                continue;
            }
            x--; y--;
            if (x < 0 || x > 9 || y < 0 || y > 9) continue;

            int res;
            if (mode == 1) {
                res = shoot(enemy_board, x, y, &my_hits);
            }
            else {
                res = shoot_human_enemy(enemy_board, x, y, &my_hits);
            }

            if (res == -1) continue;
            if (res == 0) turn = 0;
        }
        else {
            if (mode == 1) {
                int x = rand() % 10;
                int y = rand() % 10;
                int res = shoot(my_board, x, y, &enemy_hits);
                if (res == -1) continue;
                if (res == 0) turn = 1;
            }
            else {
                int x, y;
                printf("\nХОД СОПЕРНИКА. Куда он выстрелил? (X Y): ");
                if (scanf("%d %d", &x, &y) != 2) {
                    scanf("%*[^\n]");
                    getchar();
                    continue;
                }
                x--; y--;
                if (x < 0 || x > 9 || y < 0 || y > 9) continue;

                int res = shoot(my_board, x, y, &enemy_hits);
                if (res == -1) continue;
                if (res == 0) turn = 1;
            }
        }
    }

    show_game(0);
    if (my_hits == 20) {
        printf("\nПобеда!\n");
    }
    else {
        printf("\nПоражение!\n");
    }

    return 0;
}