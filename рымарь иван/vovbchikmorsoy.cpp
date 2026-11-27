#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define SIZE 10

char myField[SIZE][SIZE];      // моё поле: '.' вода, 's' корабль, 'X' попадание врага, 'o' промах врага
char myShipId[SIZE][SIZE];     // номер моего корабля в клетке ('0'..'9')
char enemyField[SIZE][SIZE];   // поле врага: '.' вода, 's' корабль, 'X' моё попадание
char enemyShipId[SIZE][SIZE];  // номер корабля врага в клетке
char myShots[SIZE][SIZE];      // мои выстрелы: '.' не стреляли, 'X' попадание, 'o' промах

const int ships[10] = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };
const int TOTAL_DECKS = 20;

// панель ходов: у каждого игрока храним прошлый и текущий ход
char myPrev[64], myCur[64];     // мои ходы
char enPrev[64], enCur[64];     // ходы врага

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Буква -> номер столбца (0..9).
   Понимает русские А-К и а-к (кодировка CP1251) и английские A-J. */
int letterToCol(char c) {
    unsigned char u = (unsigned char)c;
    if (u >= 0xC0 && u <= 0xC9) return u - 0xC0;   // А..К (заглавные)
    if (u >= 0xE0 && u <= 0xE9) return u - 0xE0;   // а..к (строчные)
    if (u >= 'A' && u <= 'J')   return u - 'A';    // латиница на всякий случай
    return -1;
}

// запомнить координату в виде "Б7"
void makeCoord(int x, int y, char* buf) {
    sprintf(buf, "%c%d", (char)(0xC0 + x), y);
}

// предыдущий ход становится прошлым, новый — текущий
void pushMove(char* prev, char* cur, const char* coord, const char* result) {
    strcpy(prev, cur);
    sprintf(cur, "%s - %s", coord, result);
}

void resetMoves(void) {
    strcpy(myPrev, "-");
    strcpy(myCur, "-");
    strcpy(enPrev, "-");
    strcpy(enCur, "-");
}

void clearAll(void) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            myField[i][j] = '.';
            myShipId[i][j] = '.';
            enemyField[i][j] = '.';
            enemyShipId[i][j] = '.';
            myShots[i][j] = '.';
        }
}

/* Проверка по правилам морского боя:
   корабль ЦЕЛИКОМ должен уместиться в границах поля,
   а вокруг него (включая диагонали) не должно быть других кораблей. */
int canPlace(const char b[SIZE][SIZE], int x, int y, int len, int dir) {
    // 1) сам корабль не должен вылезать за край поля
    if (dir == 1) {                        // горизонтальный
        if (x + len - 1 > SIZE - 1)        // упирается в правый край
            return 0;
    }
    else {                                 // вертикальный
        if (y - 1 + len - 1 > SIZE - 1)    // упирается в нижний край
            return 0;
    }

    // 2) вокруг корабля (включая диагонали) не должно быть других кораблей
    int x1 = x - 1;
    int x2 = (dir == 1) ? x + len : x + 1;
    int y1 = y - 2;
    int y2 = (dir == 1) ? y : y - 1 + len;

    for (int i = y1; i <= y2; i++) {
        for (int j = x1; j <= x2; j++) {
            if (i < 0 || i > SIZE - 1 || j < 0 || j > SIZE - 1)
                continue;           // за краем поля мешать нечему
            if (b[i][j] == 's')
                return 0;           // нашли соседний корабль — ставить нельзя
        }
    }
    return 1;
}

void putShip(char b[SIZE][SIZE], char ids[SIZE][SIZE],
    int x, int y, int len, int dir, char id) {
    for (int k = 0; k < len; k++) {
        int cx = (dir == 1) ? x + k : x;
        int cy = (dir == 1) ? y - 1 : y - 1 + k;
        b[cy][cx] = 's';
        ids[cy][cx] = id;
    }
}

// случайная расстановка одного флота (по правилам, без касаний)
void placeShipsRandom(char b[SIZE][SIZE], char ids[SIZE][SIZE]) {
    for (int s = 0; s < 10; s++) {
        int guard = 0;
        while (1) {
            int x = rand() % SIZE;
            int y = rand() % SIZE + 1;
            int dir = rand() % 2 + 1;
            if (canPlace(b, x, y, ships[s], dir)) {
                putShip(b, ids, x, y, ships[s], dir, (char)('0' + s));
                break;
            }
            if (++guard > 10000) {   // на всякий случай: начать поле заново
                for (int i = 0; i < SIZE; i++)
                    for (int j = 0; j < SIZE; j++) {
                        b[i][j] = '.';
                        ids[i][j] = '.';
                    }
                s = -1;              // расставляем сначала
                break;
            }
        }
    }
}

/* ---------- печать ---------- */

void printPairHeader(void) {
    printf("    ВАШИ КОРАБЛИ                       ВЫСТРЕЛЫ ПО ВРАГУ\n");
    printf("    А Б В Г Д Е Ж З И К                А Б В Г Д Е Ж З И К\n");
    printf("   +--------------------+             +--------------------+\n");
}

void printMovePanel(void) {
    printf("   +--------------------+             +--------------------+\n");
    printf("    ПРОШЛЫЙ ХОД:  вы: %-13s враг: %s\n", myPrev, enPrev);
    printf("    ТЕКУЩИЙ ХОД:  вы: %-13s враг: %s\n", myCur, enCur);
    printf("   ---------------------------------------------------------\n");
}

// два поля рядом + панель ходов
void showBoth(void) {
    printPairHeader();
    for (int i = 0; i < SIZE; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < SIZE; j++)
            printf("%c ", myField[i][j]);
        printf("|");
        printf("          ");                       // ровно 10 пробелов между полями
        printf("%2d |", i + 1);
        for (int j = 0; j < SIZE; j++)
            printf("%c ", myShots[i][j]);
        printf("|\n");
    }
    printMovePanel();
}

// одно поле (при расстановке)
void showMy(void) {
    printf("\n    А Б В Г Д Е Ж З И К\n");
    printf("   +--------------------+\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < SIZE; j++)
            printf("%c ", myField[i][j]);
        printf("|\n");
    }
    printf("   +--------------------+\n");
}

// раскрыть поле врага в конце игры
void revealEnemy(void) {
    printf("\n    ПОЛЕ ПРОТИВНИКА (раскрыто)\n");
    printf("    А Б В Г Д Е Ж З И К\n");
    printf("   +--------------------+\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d |", i + 1);
        for (int j = 0; j < SIZE; j++)
            printf("%c ", enemyField[i][j]);
        printf("|\n");
    }
    printf("   +--------------------+\n");
}

/* ---------- расстановка ---------- */

void placePlayerShips(void) {
    int mode;
    char colCh;
    int x, y, dir;

    printf("\nКак расставить свои корабли?\n");
    printf("1 - вручную, 2 - случайно: ");

    if (scanf("%d", &mode) != 1)
        mode = 1;
    clearInput();

    if (mode == 2) {
        placeShipsRandom(myField, myShipId);
        showMy();
        printf("Ваши корабли расставлены случайно!\n");
        return;
    }

    for (int s = 0; s < 10; s++) {
        showMy();

        while (1) {
            printf("\nКорабль размером %d. Введите столбец (А-К), строку (1-10) и направление (1-гориз, 2-верт): ", ships[s]);

            if (scanf(" %c %d %d", &colCh, &y, &dir) != 3) {
                printf("Ошибка: нужна буква и два числа!\n");
                clearInput();
                continue;
            }
            clearInput();

            x = letterToCol(colCh);

            if (x < 0 || dir < 1 || dir > 2 || y < 1 || y > 10) {
                printf("Ошибка: столбец А-К, строка 1-10, направление 1 или 2.\n");
                continue;
            }
            if (!canPlace(myField, x, y, ships[s], dir)) {
                printf("Ошибка: корабль не помещается, пересекает другой или касается его!\n");
                continue;
            }
            break;
        }

        putShip(myField, myShipId, x, y, ships[s], dir, (char)('0' + s));
    }
}

/* ---------- игра ---------- */

int isSunk(const char b[SIZE][SIZE], const char ids[SIZE][SIZE], char id) {
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (ids[i][j] == id && b[i][j] == 's')
                return 0; // нашли целую палубу — корабль ещё жив
    return 1;
}

void play(void) {
    int myHits = 0;    // мои попадания по врагу
    int enemyHits = 0; // попадания врага по мне
    char colCh;
    int x, y;

    resetMoves();

    while (1) {
        showBoth();
        printf("\nВаш выстрел (столбец строка, например Б 7): ");

        if (scanf(" %c %d", &colCh, &y) != 2) {
            printf("Ошибка: нужна буква и число!\n");
            clearInput();
            continue;
        }
        clearInput();

        x = letterToCol(colCh);

        if (x < 0 || y < 1 || y > 10) {
            printf("Мимо поля! Столбец А-К, строка 1-10.\n");
            continue;
        }

        if (myShots[y - 1][x] != '.') {
            printf("Вы уже стреляли сюда! Нажмите Enter...\n");
            getchar();
            continue;
        }

        // мой ход
        char coord[16];
        makeCoord(x, y, coord);

        if (enemyField[y - 1][x] == 's') {
            char id = enemyShipId[y - 1][x];
            enemyField[y - 1][x] = 'X';
            myShots[y - 1][x] = 'X';
            myHits++;

            if (isSunk(enemyField, enemyShipId, id)) {
                printf("УБИЛ! Корабль врага потоплен полностью!\n");
                pushMove(myPrev, myCur, coord, "УБИЛ");
            }
            else {
                printf("ПОПАЛ!\n");
                pushMove(myPrev, myCur, coord, "ПОПАЛ");
            }
        }
        else {
            printf("МИМО!\n");
            myShots[y - 1][x] = 'o';
            pushMove(myPrev, myCur, coord, "МИМО");
        }

        if (myHits == TOTAL_DECKS) {
            showBoth();
            printf("\nПОБЕДА! Все корабли врага уничтожены!\n");
            revealEnemy();
            return;
        }

        // ход врага: случайная клетка, куда он ещё не стрелял
        printf("Ход противника...\n");
        int cx, cy;
        do {
            cx = rand() % SIZE;
            cy = rand() % SIZE;
        } while (myField[cy][cx] == 'X' || myField[cy][cx] == 'o');

        char eCoord[16];
        makeCoord(cx, cy, eCoord);

        if (myField[cy][cx] == 's') {
            char id = myShipId[cy][cx];
            myField[cy][cx] = 'X';
            enemyHits++;

            if (isSunk(myField, myShipId, id)) {
                printf("Враг потопил ваш корабль в %s!\n", eCoord);
                pushMove(enPrev, enCur, eCoord, "УБИЛ");
            }
            else {
                printf("Враг попал в ваш корабль в %s!\n", eCoord);
                pushMove(enPrev, enCur, eCoord, "ПОПАЛ");
            }
        }
        else {
            myField[cy][cx] = 'o';
            printf("Враг промахнулся (%s).\n", eCoord);
            pushMove(enPrev, enCur, eCoord, "МИМО");
        }

        if (enemyHits == TOTAL_DECKS) {
            showBoth();
            printf("\nПОРАЖЕНИЕ! Враг уничтожил все ваши корабли.\n");
            revealEnemy();
            return;
        }

        printf("Нажмите Enter, чтобы продолжить...\n");
        getchar();
    }
}

int main(void) {
    system("chcp 1251 > nul");
    srand((unsigned)time(NULL));

    int again;
    do {
        clearAll();
        placePlayerShips();  // свои корабли: вручную или случайно
        placeShipsRandom(enemyField, enemyShipId); // враг расставляет случайно
        play();

        printf("\nИграть снова? (1 - да, 0 - нет): ");
        if (scanf("%d", &again) != 1)
            again = 0;
        clearInput();

    } while (again == 1);

    printf("Спасибо за игру!\n");
    return 0;
}
