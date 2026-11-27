#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <string.h>
#include <stdarg.h>

char field[10][10];
char shipId[10][10];
char shotField[10][10];

char statusMsg[256] = "Начало игры. Расставьте корабли.";
int myAliveCount = 20;   // 4+3+3+2+2+2+1+1+1+1 = 20 клеток-палуб
int enemySunkCount = 0;  // сколько кораблей соперника потоплено

void setMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(statusMsg, fmt, args);
    va_end(args);
}

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int letterToCol(char c) {
    static const char* lat = "ABCDEFGHIJ";
    static const char* rusUp = "АБВГДЕЖЗИК";
    static const char* rusLo = "абвгдежзик";

    const char* p = strchr(lat, (char)toupper((unsigned char)c));
    if (p) return (int)(p - lat);

    p = strchr(rusUp, c);
    if (p) return (int)(p - rusUp);

    p = strchr(rusLo, c);
    if (p) return (int)(p - rusLo);

    return -1;
}

// печать клетки с символом и цветом:
// '.'  -> ♥  тёмно-серый (пусто / не стреляли)
// 's'  -> ■  ярко-белый  (целый корабль)
// 'X'  -> ✖  красный     (попадание)
// 'o'  -> ·  тёмно-серый (промах)
void printGlyph(char c) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    wchar_t wch;
    WORD attr = 7;
    DWORD written;

    switch (c) {
    case '.': wch = L'\x2665'; attr = 8;  break;
    case 's': wch = L'\x25A0'; attr = 15; break;
    case 'X': wch = L'\x2716'; attr = 12; break;
    case 'o': wch = L'\x00B7'; attr = 8;  break;
    default:  wch = (wchar_t)(unsigned char)c; break;
    }

    fflush(stdout);
    SetConsoleTextAttribute(h, attr);
    WriteConsoleW(h, &wch, 1, &written, NULL);
    SetConsoleTextAttribute(h, 7);   // вернуть обычный цвет
}

void matrx(void) {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++) {
            field[i][j] = '.';
            shipId[i][j] = '.';
            shotField[i][j] = '.';
        }
}

void showBoth(void) {
    system("cls");
    printf("       ВАШИ КОРАБЛИ                  ВАШИ ВЫСТРЕЛЫ\n");
    printf("    А Б В Г Д Е Ж З И К           А Б В Г Д Е Ж З И К\n");
    printf("   ......................         ....................\n");
    for (int i = 0; i < 10; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < 10; j++) {
            printGlyph(field[i][j]);
            printf(" ");
        }
        printf("|     ");
        printf("%2d |", i + 1);
        for (int j = 0; j < 10; j++) {
            printGlyph(shotField[i][j]);
            printf(" ");
        }
        printf("|\n");
    }
    printf("   ......................         ....................\n");

    printf("\nВаших клеток живых: %d из 20 | Потоплено у соперника: %d из 10\n",
        myAliveCount, enemySunkCount);
    printf("> %s\n", statusMsg);
}

void show(void) {
    system("cls");
    printf("    А Б В Г Д Е Ж З И К\n");
    printf("   ......................\n");
    for (int i = 0; i < 10; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < 10; j++) {
            printGlyph(field[i][j]);
            printf(" ");
        }
        printf("|\n");
    }
    printf("   ......................\n");
    printf("\n> %s\n", statusMsg);
}

void placeShip(void) {
    int ships[] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

    for (int s = 0; s < 10; s++) {
        show();
        char colCh;
        int x, y, dir;

        while (1) {
            printf("\nКораблик размером %d. Введите столбец (А-К) и строку (1-10): ", ships[s]);

            if (scanf_s(" %c %d", &colCh, 1, &y) != 2) {
                setMsg("Ошибка: нужна буква и число!");
                clearInput();
                show();
                continue;
            }
            clearInput();

            printf("Направление (1-горизонт, 2-вертик): ");
            if (scanf_s("%d", &dir) != 1) {
                setMsg("Ошибка: нужно число 1 или 2!");
                clearInput();
                show();
                continue;
            }
            clearInput();

            x = letterToCol(colCh);

            if (x < 0 || dir < 1 || dir > 2 || y < 1 || y > 10) {
                setMsg("Ошибка: столбец А-К, строка 1-10, направление 1 или 2.");
                show();
                continue;
            }
            if ((dir == 1 && x + ships[s] - 1 > 9) || (dir == 2 && y + ships[s] - 1 > 10)) {
                setMsg("Ошибка: корабль не помещается на поле!");
                show();
                continue;
            }

            int busy = 0;
            for (int k = 0; k < ships[s]; k++) {
                int cx = (dir == 1) ? x + k : x;
                int cy = (dir == 1) ? y - 1 : y - 1 + k;
                if (field[cy][cx] != '.') { busy = 1; break; }
            }
            if (busy) {
                setMsg("Ошибка: здесь уже есть корабль!");
                show();
                continue;
            }
            break;
        }

        for (int k = 0; k < ships[s]; k++) {
            if (dir == 1) {
                field[y - 1][x + k] = 's';
                shipId[y - 1][x + k] = (char)('0' + s);
            }
            else {
                field[y - 1 + k][x] = 's';
                shipId[y - 1 + k][x] = (char)('0' + s);
            }
        }
        setMsg("Корабль №%d (%d клеток) установлен. Осталось: %d.", s + 1, ships[s], 9 - s);
    }
}

int isSunk(char id) {
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            if (shipId[i][j] == id && field[i][j] == 's')
                return 0;
    return 1;
}

int opponentTurn(void) {
    char colCh;
    int x, y;

    while (1) {
        showBoth();
        printf("\nХод СОПЕРНИКА. Введите клетку его выстрела (например Б 7): ");

        if (scanf_s(" %c %d", &colCh, 1, &y) != 2) {
            setMsg("Ошибка: нужна буква и число!");
            clearInput();
            continue;
        }
        clearInput();

        x = letterToCol(colCh);
        if (x < 0 || y < 1 || y > 10) {
            setMsg("Мимо поля! Столбец А-К, строка 1-10.");
            continue;
        }

        int cx = x, cy = y - 1;

        if (field[cy][cx] == 'X' || field[cy][cx] == 'o') {
            setMsg("Сюда уже стреляли! Пусть назовёт другую клетку.");
            continue;
        }

        if (field[cy][cx] == 's') {
            char id = shipId[cy][cx];
            field[cy][cx] = 'X';          // отметили попадание на своём поле
            myAliveCount--;
            if (isSunk(id))
                setMsg("УБИЛ! Соперник потопил ваш корабль и ходит снова.");
            else
                setMsg("ПОПАЛ! Соперник попал и ходит снова.");
            return 1;
        }
        else {
            field[cy][cx] = 'o';          // отметили промах на своём поле
            setMsg("МИМО! Ход переходит вам.");
            return 0;
        }
    }
}

int myTurn(void) {
    char colCh;
    int x, y, res;

    while (1) {
        showBoth();
        printf("\nВАШ ход. Введите клетку выстрела (например Б 7): ");

        if (scanf_s(" %c %d", &colCh, 1, &y) != 2) {
            setMsg("Ошибка: нужна буква и число!");
            clearInput();
            continue;
        }
        clearInput();

        x = letterToCol(colCh);
        if (x < 0 || y < 1 || y > 10) {
            setMsg("Мимо поля! Столбец А-К, строка 1-10.");
            continue;
        }

        int cx = x, cy = y - 1;

        if (shotField[cy][cx] != '.') {
            setMsg("Сюда вы уже стреляли! Введите другую клетку.");
            continue;
        }

        printf("Что сказал соперник? 1 - ПОПАЛ, 2 - УБИЛ, 0 - МИМО: ");
        if (scanf_s("%d", &res) != 1)
            res = -1;
        clearInput();

        if (res == 1 || res == 2) {
            shotField[cy][cx] = 'X';      // отметили попадание на поле выстрелов
            if (res == 2) {
                enemySunkCount++;
                setMsg("УБИЛ! Потоплено кораблей соперника: %d из 10.", enemySunkCount);
            }
            else {
                setMsg("ПОПАЛ! Вы ходите снова.");
            }
            return 1;
        }
        else if (res == 0) {
            shotField[cy][cx] = 'o';      // отметили промах на поле выстрелов
            setMsg("МИМО. Записали промах. Ход соперника.");
            return 0;
        }
        else {
            setMsg("Не понял ответ — введите 0, 1 или 2.");
            continue;
        }
    }
}

void play(void) {
    system("cls");
    printf("Играем с живым соперником! Стреляете по очереди.\n");
    printf("При попадании ходящий стреляет ещё раз.\n\n");
    system("pause");

    while (1) {
        while (opponentTurn()) {
            if (myAliveCount == 0) {
                showBoth();
                setMsg("УВЫ! Соперник потопил все ваши корабли. Вы проиграли.");
                printf("\n> %s\n", statusMsg);
                return;
            }
        }

        while (myTurn()) {
            if (enemySunkCount == 10) {
                showBoth();
                setMsg("ПОБЕДА! Вы потопили все 10 кораблей соперника!");
                printf("\n> %s\n", statusMsg);
                return;
            }
        }
    }
}

int main(void) {
    system("chcp 1251 > nul");

    matrx();
    placeShip();
    play();

    return 0;
}
