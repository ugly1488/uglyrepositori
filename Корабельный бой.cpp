#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#define N 10
#define HP 20   /* всего палуб во флоте */

int my[N][N], en[N][N];

typedef struct { int x[4][2], len, hit; } Ship;
Ship ms[10];
int mc, ph, pm, shot, om;   /* om - попадания противника в ваши корабли */
int wins = 0, losses = 0;
const int fl[10] = { 4,3,3,2,2,2,1,1,1,1 };

/* шапка таблицы на кириллице */
const char* COLS = "А Б В Г Д Е Ж З И К";

void clr(int b[N][N]) { for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) b[r][c] = 0; }

/* буква колонки -> номер 0..9. Понимает латиницу и кириллицу (CP1251) */
int col_index(char cl) {
    unsigned char u = (unsigned char)cl;
    if (u >= 'A' && u <= 'J') return u - 'A';            /* латиница большие */
    if (u >= 'a' && u <= 'j') return u - 'a';            /* латиница строчные */
    if (u >= 0xC0 && u <= 0xCA) return u - 0xC0;         /* кириллица А..К */
    if (u >= 0xE0 && u <= 0xEA) return u - 0xE0;         /* кириллица а..к */
    return -1;
}

int can(int b[N][N], int r, int c, int L, int hz) {
    for (int i = 0; i < L; i++) {
        int rr = r + (hz ? 0 : i), cc = c + (hz ? i : 0);
        if (rr < 0 || rr >= N || cc < 0 || cc >= N || b[rr][cc]) return 0;
        for (int dr = -1; dr < 2; dr++) for (int dc = -1; dc < 2; dc++) {
            int nr = rr + dr, nc = cc + dc;
            if (nr >= 0 && nr < N && nc >= 0 && nc < N && b[nr][nc] == 1) return 0;
        }
    }
    return 1;
}

void put(int b[N][N], Ship s[], int* cnt, int r, int c, int L, int hz) {
    for (int i = 0; i < L; i++) {
        int rr = r + (hz ? 0 : i), cc = c + (hz ? i : 0);
        b[rr][cc] = 1; s[*cnt].x[i][0] = rr; s[*cnt].x[i][1] = cc;
    }
    s[*cnt].len = L; s[*cnt].hit = 0; (*cnt)++;
}

void rnd(int b[N][N], Ship s[], int* cnt) {
    clr(b); *cnt = 0;
    for (int k = 0; k < 10; k++)
        while (*cnt == k) {
            int r = rand() % N, c = rand() % N, hz = rand() % 2;
            if (can(b, r, c, fl[k], hz)) put(b, s, cnt, r, c, fl[k], hz);
        }
}

/* печать одного символа заданным цветом, после - возврат к белому */
void pc(int col, char c) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, col);
    printf("%c ", c);
    SetConsoleTextAttribute(h, 7);
}

/* печать строки цветом (для сообщений о победе/поражении) */
void pcol(int col, const char* s) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, col);
    printf("%s", s);
    SetConsoleTextAttribute(h, 7);
}

void show(void) {
    system("cls");   /* поле перерисовывается, а не дублируется */
    puts("      МОИ ВЫСТРЕЛЫ              МОИ КОРАБЛИ");
    printf("   %s      %s\n", COLS, COLS);
    for (int r = 0; r < N; r++) {
        printf("%2d ", r + 1);
        for (int c = 0; c < N; c++) {
            switch (en[r][c]) {          /* левое поле: ваши выстрелы */
            case 0: pc(8, '.'); break;   /* серый   - не стреляли */
            case 1: pc(9, 'o'); break;   /* голубой - промах */
            case 2: pc(12, 'X'); break;   /* красный - попадание */
            case 3: pc(14, '#'); break;   /* жёлтый  - потоплен */
            }
        }
        printf("  %2d ", r + 1);
        for (int c = 0; c < N; c++) {
            switch (my[r][c]) {          /* правое поле: ваши корабли */
            case 0: pc(8, '.'); break;   /* серый   - пусто */
            case 1: pc(10, '#'); break;   /* зелёный - ваш корабль */
            case 2: pc(9, 'o'); break;   /* голубой - противник промахнулся */
            case 3: pc(12, 'X'); break;   /* красный - в ваш корабль попали */
            }
        }
        putchar('\n');
    }
    printf("Мои выстрелы: %d (попаданий %d, промахов %d). В меня попали: %d\n", shot, ph, pm, om);
    puts("Легенда: . - не стреляли, o - промах, X - попадание, # - потопленный корабль/ваш корабль");
    fflush(stdout);
}

void show_score(void) {
    printf("\n=== СЧЁТ: победы %d : %d поражения ===\n", wins, losses);
}

void rules(void) {
    puts("\n================= ПРАВИЛА =================");
    puts("Игроки по очереди стреляют по полю противника.");
    puts("Флот каждого: 1 корабль 4 палубы, 2 корабля 3 палубы,");
    puts("3 корабля 2 палубы, 4 корабля 1 палуба.");
    puts("Корабли не касаются друг друга даже углами.");
    puts("Координаты вводятся как колонка + строка, например Б2.");
    puts("Можно вводить и русские, и латинские буквы (Б2 или B2).");
    puts("Вы стреляете и сами честно отмечаете результат:");
    puts("  x - попадание, 0 - промах.");
    puts("После вашего промаха ход переходит противнику:");
    puts("отмечайте его выстрелы по своему полю так же (x/0).");
    puts("Победа - потопить все 20 палуб противника.");
    puts("Поражение - когда в вас попали 20 раз.");
    puts("й или q - сдаться (засчитывается поражение).");
    puts("===========================================");
    printf("Нажмите Enter, чтобы вернуться в меню... ");
    while (getchar() != '\n');
}

/* ручная отметка результата: 1 - попадание, 0 - промах */
int mark(void) {
    while (1) {
        printf("Результат: x - попадание, 0 - промах: ");
        char a;
        if (scanf(" %c", &a) != 1) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
        while (getchar() != '\n');
        a = (char)tolower((unsigned char)a);
        if (a == 'x' || a == (char)0xF5 || a == (char)0xD5) return 1;   /* лат. и рус. x/Х/х */
        if (a == '0' || a == 'o' || a == (char)0xEE || a == (char)0xCE) return 0; /* 0, лат. и рус. о/О */
        puts("Введите x или 0!");
    }
}

int you(void) {
    puts("--- Ваш ход ---");
    while (1) {
        printf("Выстрел (Пример: Б2), й - сдаться: ");
        char cl; int rw;
        if (scanf(" %c", &cl) != 1) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
        /* сдаться: латинская q, русская й или Й */
        if (tolower((unsigned char)cl) == 'q' ||
            (unsigned char)cl == 0xE9 || (unsigned char)cl == 0xC9) {
            while (getchar() != '\n'); return -1;
        }
        if (scanf("%d", &rw) != 1) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
        while (getchar() != '\n');
        int r = rw - 1, c = col_index(cl);
        if (r < 0 || r >= N || c < 0) { puts("Ошибка: координаты вне поля!"); continue; }
        if (en[r][c] != 0) { puts("Сюда уже стреляли!"); continue; }

        shot++;
        int res = mark();
        if (res) { ph++; pcol(12, "Попадание!\n"); }
        else { pm++; pcol(9, "Мимо!\n"); }
        en[r][c] = res ? 2 : 1;    /* 2 = X (попадание), 1 = o (промах) */
        show();
        if (ph == HP) return 1;    /* флот противника потоплен */
        if (res == 0) return 0;    /* промах - ход противника */
    }
}

/* отмечаем выстрелы противника по вашему полю.
   возвращает 1 - ваш флот потоплен (поражение), 0 - противник промахнулся */
int opp_move(void) {
    puts("--- Ход противника: отмечайте его выстрелы ---");
    while (1) {
        printf("Куда стрелял противник (Пример: Б2): ");
        char cl; int rw;
        if (scanf(" %c", &cl) != 1) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
        if (scanf("%d", &rw) != 1) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
        while (getchar() != '\n');
        int r = rw - 1, c = col_index(cl);
        if (r < 0 || r >= N || c < 0) { puts("Ошибка: координаты вне поля!"); continue; }
        if (my[r][c] > 1) { puts("Этот выстрел уже отмечен!"); continue; }

        int res = mark();
        if (res) {
            om++;
            pcol(12, "Попадание в ваш корабль!\n");
            if (my[r][c] == 0) pcol(14, "Внимание: там не было вашего корабля - проверьте честность отметки!\n");
            my[r][c] = 3;          /* X - в ваш корабль попали */
        }
        else {
            pcol(9, "Мимо!\n");
            if (my[r][c] == 1) pcol(14, "Внимание: там стоит ваш корабль, а отмечен промах - проверьте отметку!\n");
            my[r][c] = 2;          /* o - противник промахнулся */
        }
        show();
        if (om == HP) return 1;    /* все 20 ваших палуб поражены */
        if (res == 0) return 0;    /* противник промахнулся - снова ваш ход */
    }
}

void setup(void) {
    printf("1 - расставить самому, 2 - случайно: ");
    int t = 0;
    if (scanf("%d", &t) == 1 && t == 1) {
        clr(my); mc = 0;
        puts("Формат: колонка строка h/v (пример: б 5 h), u - отменить");
        while (mc < 10) {
            HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            printf("   %s\n", COLS);
            for (int i = 0; i < N; i++) {
                printf("%2d ", i + 1);
                for (int j = 0; j < N; j++) {
                    if (my[i][j]) pc(10, '#');   /* зелёные корабли при расстановке */
                    else { SetConsoleTextAttribute(h, 8); putchar('.'); putchar(' '); SetConsoleTextAttribute(h, 7); }
                }
                putchar('\n');
            }
            SetConsoleTextAttribute(h, 7);
            printf("%d палуб, стоит %d/10: ", fl[mc], mc);
            char c1, c2; int rw;
            if (scanf(" %c", &c1) != 1) continue;
            if (tolower((unsigned char)c1) == 'u' || (unsigned char)c1 == 0xE3 || (unsigned char)c1 == 0xC3) {
                if (mc) {
                    mc--;
                    for (int j = 0; j < ms[mc].len; j++) my[ms[mc].x[j][0]][ms[mc].x[j][1]] = 0;
                }
                else puts("Нечего удалять!");
                continue;
            }
            if (scanf("%d %c", &rw, &c2) != 2) { while (getchar() != '\n'); puts("Ошибка ввода!"); continue; }
            while (getchar() != '\n');
            c2 = (char)tolower((unsigned char)c2);
            int r = rw - 1, c = col_index(c1), hz;
            /* ориентация: h/v латиницей, г/в кириллицей (в = горизонталь, г = вертикаль) */
            if (c2 == 'h' || c2 == 'v') hz = (c2 == 'h');
            else if ((unsigned char)c2 == 0xE2 || (unsigned char)c2 == 0xC2) hz = 1;      /* в */
            else if ((unsigned char)c2 == 0xE3 || (unsigned char)c2 == 0xC3) hz = 0;      /* г */
            else hz = -1;
            if (r < 0 || r >= N || c < 0 || hz < 0) { puts("Ошибка!"); continue; }
            if (!can(my, r, c, fl[mc], hz)) { puts("Нельзя: выйдет за поле или коснётся другого!"); continue; }
            put(my, ms, &mc, r, c, fl[mc], hz);
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        puts("Все корабли расставлены!\n");
    }
    else rnd(my, ms, &mc);
}

void game(void) {
    ph = pm = shot = om = 0;
    clr(my); clr(en);
    mc = 0;

    setup();
    puts("\nВы стреляете по полю противника и сами отмечаете результат (x/0).");
    puts("После вашего промаха отмечайте выстрелы противника по своему полю.");
    puts("Победа - 20 попаданий в противника. Поражение - 20 попаданий в вас. й - сдаться.\n");
    show();

    while (1) {
        int res = you();
        if (res == 1) {
            pcol(10, "\nПобеда! Флот противника потоплен.\n");
            wins++;
            break;
        }
        if (res == -1) {
            pcol(14, "\nПартия сдана. Засчитано поражение.\n");
            losses++;
            break;
        }
        /* ваш промах - ход противника */
        puts("Ход противника... Нажмите Enter, когда он выстрелит.");
        while (getchar() != '\n');
        if (opp_move()) {
            pcol(12, "\nПоражение! Ваш флот потоплен.\n");
            losses++;
            break;
        }
    }

    show();
    show_score();
}

int main(void) {
    SetConsoleOutputCP(1251); SetConsoleCP(1251);
    srand((unsigned)time(NULL));

    while (1) {
        system("cls");
        puts("========================================");
        puts("           МОРСКОЙ БОЙ");
        puts("========================================");
        printf("  СЧЁТ: победы %d : поражения %d\n", wins, losses);
        puts("----------------------------------------");
        puts("  1 - Начать игру");
        puts("  2 - Правила");
        puts("  3 - Выход");
        puts("========================================");
        printf("Ваш выбор: ");

        int t = 0;
        if (scanf("%d", &t) != 1) {
            while (getchar() != '\n');
            puts("Ошибка ввода! Введите 1, 2 или 3.");
            continue;
        }
        while (getchar() != '\n');

        if (t == 1) {
            game();   /* после партии возвращаемся в меню */
        }
        else if (t == 2) {
            rules();
        }
        else if (t == 3) {
            show_score();
            puts("Спасибо за игру!");
            break;
        }
        else {
            puts("Нет такого пункта! Введите 1, 2 или 3.");
        }
    }
    return 0;
}

