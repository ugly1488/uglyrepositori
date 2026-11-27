#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int arr[10]; //Задаем индекс массиву
    //Это цикл работает нормально
    printf("Исходный масиив:"); //Вывод текста
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % (30 - 10 + 1) + 10; //Обращаемя к массиву и присваиваем значение
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf("Обратный порядок:");
    for (int i = 9; i >= 0; i--) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf(" Задача 6.2 \n");
    int arr2[30];

    printf("Массив: ");
    for (int i = 0; i < 30; i++) {
        arr2[i] = 1 + rand() % 500;
        printf("%d ", arr2[i]);
    }
    printf("\n");

    int min_val = arr2[0];
    int max_val = arr2[0];

    for (int i = 1; i < 30; i++) {
        if (arr2[i] < min_val) min_val = arr2[i];
        if (arr2[i] > max_val) max_val = arr2[i];
    }
    printf("min: %d, max: %d\n\n", min_val, max_val);
    printf(" Задача 6.3 \n");
    int arr3[10];

    printf("Исходный массив: ");
    for (int i = 0; i < 10; i++) {
        arr3[i] = -500 + rand() % (500 - (-500) + 1);
        printf("%d ", arr3[i]);
    }
    printf("\n");

    printf("Модули чисел:    ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", abs(arr3[i]));
    }
    printf("\n\n");

    printf(" Задача 6.4 ");
    int A[10][10];
    int B[10][10];
    int C[10][10]; 

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
        }
    }

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    printf("\nЗадача 6.4 :\n");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%4d ", C[i][j]);
        }
        printf("\n");
    }

    int matrix[10][10];

    printf("\nЗадача 6.5 :\n");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = rand() % 100;
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < 10; i++) {
        int min_row = matrix[i][0];
        for (int j = 1; j < 10; j++) {
            if (matrix[i][j] < min_row) {
                min_row = matrix[i][j];
            }
        }
        printf("Минимум в строке %d: %d\n", i, min_row);
    }
    printf("\n");

    for (int j = 0; j < 10; j++) {
        int min_col = matrix[0][j];
        for (int i = 1; i < 10; i++) {
            if (matrix[i][j] < min_col) {
                min_col = matrix[i][j];
            }
        }
        printf("Минимум в столбце %d: %d\n", j, min_col);
    }
    printf("Задача 6.6");
        printf("   ******   \n");
        printf(" *  .  .  * \n");
        printf("*    --    *\n");
        printf("*   \\__/   *\n");
        printf(" *        * \n");
        printf("   ******   \n");


    return 0;
}