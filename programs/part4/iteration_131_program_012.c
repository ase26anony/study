/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int use_func = 1;

__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with VAR_DECL base */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Another section with different bounds */
    #pragma omp target data map(global_arr[0:SIZE])
    {
        for (int i = 0; i < SIZE; i++) {
            global_arr[i] = local_arr[i];
        }
    }
}

__attribute__((noinline, used))
void func_depend(int n, int size) {
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    
    if (!dynamic_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* OMP_ARRAY_SECTION with pointer base and variable bounds */
    #pragma omp task depend(inout: dynamic_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            dynamic_arr[i] *= 2;
        }
    }
    
    /* Section with arithmetic expression in bounds */
    #pragma omp task depend(inout: dynamic_arr[2*n:size/2])
    {
        for (int i = 2*n; i < 2*n + size/2; i++) {
            dynamic_arr[i] += 5;
        }
    }
    
    #pragma omp taskwait
    
    /* Copy to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr2[i] = dynamic_arr[i];
    }
    
    free(dynamic_arr);
}

__attribute__((noinline, used))
void func_update(int len) {
    int local_arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        local_arr3[i] = i * 4;
    }
    
    /* OMP_ARRAY_SECTION in target update */
    #pragma omp target update to(local_arr3[0:len])
    
    /* Modify on host */
    for (int i = 0; i < len; i++) {
        local_arr3[i] += 10;
    }
    
    #pragma omp target update from(local_arr3[0:len])
    
    /* Nested array section (ARRAY_REF as base) */
    int matrix[10][20];
    #pragma omp target data map(matrix[5][2:15])
    {
        for (int j = 2; j < 17; j++) {
            matrix[5][j] = j;
        }
    }
}

__attribute__((noinline, used))
void func_mixed(void) {
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[10:50]) \
        map(global_arr2[30:70])
    {
        for (int i = 10; i < 60; i++) global_arr[i]++;
        for (int i = 30; i < 100; i++) global_arr2[i]--;
    }
    
    /* Array section with complex subscript */
    int idx = 25;
    int length = 75;
    #pragma omp task depend(inout: global_arr[idx:length])
    {
        for (int i = idx; i < idx + length; i++) {
            global_arr[i] = global_arr2[i] * 2;
        }
    }
    #pragma omp taskwait
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_func > 0) {
        func_map();
    }
    if (use_func > 1) {
        func_depend(20, 60);
    }
    if (use_func > 2) {
        func_update(80);
    }
    if (use_func > 3) {
        func_mixed();
    }
    
    /* Compute checksum to ensure code executes */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
