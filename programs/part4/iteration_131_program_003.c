/* omp_array_section_coverage.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible across scopes */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with array base and constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple operation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Another section with different bounds */
    #pragma omp target data map(global_arr[0:SIZE])
    {
        for (int i = 0; i < SIZE; i++) {
            global_arr[i] = local_arr[i] / 2;
        }
    }
}

__attribute__((noinline, used))
void func_depend(int n, int size) {
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Section with arithmetic expression in bounds */
    int offset = 5;
    #pragma omp task depend(inout: global_arr2[offset*2:CHUNK])
    {
        for (int i = offset*2; i < offset*2 + CHUNK; i++) {
            global_arr2[i] += 1;
        }
    }
}

__attribute__((noinline, used))
void func_update(int len) {
    /* OMP_ARRAY_SECTION in target update directive */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Section with pointer arithmetic base */
    int *middle = dynamic_arr + len/2;
    #pragma omp target update to(middle[0:len/4])
}

__attribute__((noinline, used))
void func_mixed(void) {
    /* Complex expression in array section */
    int matrix[SIZE][SIZE];
    
    #pragma omp target data map(matrix[5][10:20])
    {
        for (int j = 10; j < 30; j++) {
            matrix[5][j] = j;
        }
    }
    
    /* Multiple array sections in same directive */
    #pragma omp target data map(global_arr[0:50], global_arr2[50:50])
    {
        for (int i = 0; i < 50; i++) {
            global_arr[i] += global_arr2[i+50];
        }
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * i;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (mode > 0) {
        func_map();
    }
    
    if (mode > 1) {  /* Always false but compiler doesn't know */
        func_depend(20, 30);
    } else {
        func_depend(0, CHUNK);
    }
    
    func_update(SIZE/2);
    func_mixed();
    
    /* Final computation to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        result += global_arr[i] + global_arr2[i] + dynamic_arr[i];
    }
    
    printf("Result checksum: %d\n", result);
    
    free(dynamic_arr);
    return 0;
}
