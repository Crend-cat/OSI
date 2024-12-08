#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <time.h>



#define ROWS 5
#define COLS 5

double matrix[ROWS][COLS]; // матрица
double temp_matrix_er[ROWS][COLS];
double temp_matrix_nar[ROWS][COLS];


typedef struct {
    int start_row;
    int end_row;
    int window_size;
} ThreadArgs;


int compare(const void* a, const void* b);
double find_min(double* window, int size);
double find_max(double* window, int size);
void* filters(void* args);
void copy_temp_to_matrix();
void generate_matrix();
void print_matrix(double mat[ROWS][COLS]);

sem_t start_sem;
sem_t end_sem;

int stop_threads = 0;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./a.out <iterations> <max_threads>\n");
        return EXIT_FAILURE;
    }

    int window_size = 3;
    int iterations = atoi(argv[1]);
    int max_threads = atoi(argv[2]);


    if (iterations < 1 || max_threads < 1) {
        fprintf(stderr, "Invalid input. Iterations and threads > 0.\n");
        return EXIT_FAILURE;
    }

    // Ограничиваем количество потоков размером матрицы (не более ROWS чтобы пустых не было)
    if (max_threads > ROWS) {
        max_threads = ROWS;
    }


    generate_matrix();
    copy_temp_to_matrix();


    printf("Original matrix:\n");
    print_matrix(matrix);

    // Создание массива для хранения id потоков и их аргументов
    pthread_t threads[max_threads];
    ThreadArgs thread_args[max_threads];
    int rows_per_thread = ROWS / max_threads;

    sem_init(&start_sem, 0, 0);
    sem_init(&end_sem, 0, 0);

    for (int i = 0; i < max_threads; i++) {
        thread_args[i].start_row = i * rows_per_thread;
        thread_args[i].end_row = (i == max_threads - 1) ? ROWS : (i + 1) * rows_per_thread;
        thread_args[i].window_size = window_size;

        if (pthread_create(&threads[i], NULL, filters, &thread_args[i]) != 0) {
            perror("pthread_create failed");
            return EXIT_FAILURE;
        }
    }


    clock_t start_time = clock();

    // Основной цикл
    for (int iter = 0; iter < iterations; iter++) {

        for (int i = 0; i < max_threads; i++) {
            sem_post(&start_sem); // увеличиваем значение семафора
        }




        for (int i = 0; i < max_threads; i++) {
            sem_wait(&end_sem);  // уменьшаем значение семафора
        }

    }

    // Останавливаем потоки
    stop_threads = 1;
    for (int i = 0; i < max_threads; i++) {
        sem_post(&start_sem);
    }


    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&start_sem);
    sem_destroy(&end_sem);

    clock_t end_time = clock();

    printf("\n");
    printf("Result erosion matrix:\n");
    print_matrix(temp_matrix_er);

    printf("\n");

    printf("Result building up matrix:\n");
    print_matrix(temp_matrix_nar);
    printf("\n");

    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time: %f seconds\n", time_spent);

    return EXIT_SUCCESS;
}

int compare(const void* a, const void* b) {
    return (*(double*)a - *(double*)b - 0.00000001 > 0.0);  // Сортируем по возрастанию
}


double find_min(double* window, int size) {
    qsort(window, size, sizeof(double), compare);
    return window[0];  // Возвращаем min
}
double find_max(double* window, int size) {
    qsort(window, size, sizeof(double), compare);
    return window[size - 1];  // Возвращаем max
}


void* filters(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    int start_row = thread_args->start_row;
    int end_row = thread_args->end_row;
    int window_size = thread_args->window_size;
    int offset = window_size / 2;  // Половина размера окна

    while (1) {
        sem_wait(&start_sem); // ( семафор - 1 )

        if (stop_threads) {
            break;
        }

        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i < offset || i >= ROWS - offset || j < offset || j >= COLS - offset) { // краевые эл-ты
                    temp_matrix_er[i][j] = temp_matrix_er[i][j];
                    temp_matrix_nar[i][j] = temp_matrix_nar[i][j];
                } else {
                    // Для остальных элементов вычисляем
                    double window[window_size * window_size];
                    int idx = 0;
                    for (int wi = -offset; wi <= offset; wi++) {
                        for (int wj = -offset; wj <= offset; wj++) {
                            window[idx++] = matrix[i + wi][j + wj];
                        }
                    }
                    temp_matrix_er[i][j] = find_min(window, window_size * window_size);
                    temp_matrix_nar[i][j] = find_max(window, window_size * window_size);
                }
            }
        }

        sem_post(&end_sem); // (семафор +1)
    }

    return NULL;
}

void copy_temp_to_matrix() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            temp_matrix_er[i][j] = matrix[i][j];
            temp_matrix_nar[i][j] = matrix[i][j];
        }
    }
}

void generate_matrix() {
    srand(time(NULL));
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = 1.0 + (rand() / (double)RAND_MAX) * 99.0;
        }
    }
}


void print_matrix(double mat[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%.1f ", mat[i][j]);
        }
        printf("\n");
    }
}
