/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define SECTION_SIZE 50

/* Global arrays for different base cases */
int global_arr[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section with global array and constant bounds */
NOINLINE_USED
void func_map_global(void) {
    /* Use target data with array section on global array */
    #pragma omp target data map(global_arr[10:SECTION_SIZE])
    {
        /* Simple computation to prevent dead code elimination */
        for (int i = 10; i < 10 + SECTION_SIZE; i++) {
            global_arr[i] += i;
        }
    }
}

/* Function 2: Array section with pointer and variable bounds */
NOINLINE_USED
void func_map_dynamic(int start, int length) {
    /* Use target data with array section on dynamic array */
    #pragma omp target data map(dynamic_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            dynamic_arr[i] *= 2;
        }
    }
}

/* Function 3: Array section within task depend clause */
NOINLINE_USED
void func_task_depend(int *arr, int n, int size) {
    #pragma omp task depend(inout: arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
}

/* Function 4: Array section with complex subscript expression */
NOINLINE_USED
void func_complex_expr(int *arr, int idx) {
    int lower = idx * 2;
    int length = SIZE / 4;
    
    #pragma omp target update to(arr[lower:length])
    {
        /* The update directive doesn't create a block, so we add one */
        #pragma omp target
        for (int i = lower; i < lower + length; i++) {
            arr[i] -= 5;
        }
    }
}

/* Function 5: Nested array section (ARRAY_REF as base) */
NOINLINE_USED
void func_nested_section(int matrix[][SIZE], int row) {
    /* The base is matrix[row], which is itself an array reference */
    #pragma omp target data map(matrix[row][10:30])
    {
        for (int i = 10; i < 40; i++) {
            matrix[row][i] = row * 100 + i;
        }
    }
}

/* Function 6: Multiple array sections in same directive */
NOINLINE_USED
void func_multiple_sections(int *a, int *b, int *c) {
    #pragma omp target data map(a[0:20], b[5:15], c[10:30])
    {
        for (int i = 0; i < 20; i++) a[i]++;
        for (int i = 5; i < 20; i++) b[i]--;
        for (int i = 10; i < 40; i++) c[i] *= 2;
    }
}

/* Main function with volatile control to prevent dead code elimination */
int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int local_arr[SIZE];
    int matrix[5][SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        local_arr[i] = i * 2;
        for (int j = 0; j < 5; j++) {
            matrix[j][i] = j * 1000 + i;
        }
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Call different functions based on volatile variable */
    /* This ensures all functions are compiled even if not all are called */
    if (mode > 0) {
        func_map_global();
    }
    
    if (mode > 1) {
        func_map_dynamic(20, 40);
    }
    
    #pragma omp parallel
    #pragma omp single
    {
        if (mode > 2) {
            func_task_depend(local_arr, 5, 25);
        }
    }
    
    if (mode > 3) {
        func_complex_expr(global_arr, 3);
    }
    
    if (mode > 4) {
        func_nested_section(matrix, 2);
    }
    
    if (mode > 5) {
        func_multiple_sections(global_arr, local_arr, dynamic_arr);
    }
    
    /* Force execution of tasks */
    #pragma omp taskwait
    
    /* Compute checksum to verify execution and prevent optimization */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + local_arr[i] + dynamic_arr[i];
        for (int j = 0; j < 5; j++) {
            checksum += matrix[j][i];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
