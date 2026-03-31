/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int *global_ptr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section with map clause on global array */
NOINLINE_USED
void func_map(void) {
    int i;
    /* Initialize section */
    for (i = 10; i < 110; i++) {
        global_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple computation inside region */
        for (i = 10; i < 110; i++) {
            global_arr[i] *= 2;
        }
    }
}

/* Function 2: Array section with depend clause using variables */
NOINLINE_USED
void func_depend(int n, int size) {
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] += i;
        }
    }
}

/* Function 3: Array section with update clause on pointer */
NOINLINE_USED
void func_update(int len) {
    /* OMP_ARRAY_SECTION on pointer with arithmetic in bounds */
    #pragma omp target update from(global_ptr[0:len])
    
    /* Another section with more complex expression */
    int offset = len / 2;
    #pragma omp target update to(global_ptr[offset:len - offset])
}

/* Function 4: Nested array section (ARRAY_REF as base) */
NOINLINE_USED
void func_nested(void) {
    int matrix[100][100];
    
    /* This creates ARRAY_REF as base of OMP_ARRAY_SECTION */
    #pragma omp target data map(matrix[10][20:40])
    {
        for (int i = 0; i < 40; i++) {
            matrix[10][20 + i] = i * 2;
        }
    }
}

/* Function 5: Multiple array sections in same directive */
NOINLINE_USED
void func_multiple(void) {
    int arr1[SIZE], arr2[SIZE];
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(arr1[5:25], arr2[30:40])
    {
        for (int i = 0; i < 25; i++) arr1[5 + i] = i;
        for (int i = 0; i < 40; i++) arr2[30 + i] = i * 3;
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int i, checksum = 0;
    
    /* Initialize global arrays */
    for (i = 0; i < SIZE; i++) {
        global_arr[i] = i % 100;
        global_arr2[i] = (i * 2) % 100;
    }
    
    /* Allocate and initialize pointer array */
    global_ptr = (int*)malloc(SIZE * sizeof(int));
    for (i = 0; i < SIZE; i++) {
        global_ptr[i] = i * 3;
    }
    
    /* Call different functions based on volatile variable
       to ensure all are compiled */
    if (mode & 1) func_map();
    if (mode & 2) func_depend(5, CHUNK);
    if (mode & 4) func_update(SIZE / 2);
    if (mode & 8) func_nested();
    if (mode & 16) func_multiple();
    
    /* Ensure tasks complete */
    #pragma omp taskwait
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < SIZE / 2) checksum += global_ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(global_ptr);
    return 0;
}
