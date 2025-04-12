#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 100

int array[ARRAY_SIZE];
int partial_sums[NUM_THREADS] = {0};

// 线程函数，计算数组的一部分和
void* calculate_partial_sum(void* arg) {
    int thread_id = *(int*)arg;
    int start = thread_id * (ARRAY_SIZE / NUM_THREADS);
    int end = start + (ARRAY_SIZE / NUM_THREADS);

    for (int i = start; i < end; i++) {
        partial_sums[thread_id] += array[i];
    }

    printf("Thread %d calculated partial sum: %d\n", thread_id, partial_sums[thread_id]);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // 初始化数组
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i + 1;
    }

    // 创建线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, calculate_partial_sum, (void*)&thread_ids[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 汇总部分和
    int total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_sum += partial_sums[i];
    }

    printf("Total sum of array: %d\n", total_sum);

    return 0;
}