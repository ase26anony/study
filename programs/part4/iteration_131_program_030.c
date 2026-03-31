/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section in map clause */
NOINLINE_USED
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Copy to global for verification */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function 2: Array section in depend clause with variable bounds */
NOINLINE_USED
void func_depend(int start, int length) {
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_arr2[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Ensure task completes */
    #pragma omp taskwait
}

/* Function 3: Array section in target update with pointer base */
NOINLINE_USED
void func_update(void) {
    int len = SIZE / 2;
    
    /* OMP_ARRAY_SECTION with pointer base */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Also test with arithmetic expression in bounds */
    #pragma omp target update to(dynamic_arr[len/2:len/4])
}

/* Function 4: Nested array section (ARRAY_REF as base) */
NOINLINE_USED
void func_nested(void) {
    int matrix[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* OMP_ARRAY_SECTION on array subscript result */
    #pragma omp target data map(matrix[10][20:30])
    {
        for (int j = 20; j < 50; j++) {
            matrix[10][j] += 1000;
        }
    }
    
    /* Copy to global */
    for (int j = 0; j < 100; j++) {
        global_arr[j] += matrix[10][j];
    }
}

/* Function 5: Multiple array sections in same directive */
NOINLINE_USED
void func_multiple(void) {
    int a[SIZE], b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
    }
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(a[0:SIZE/2]) map(b[SIZE/4:SIZE/2])
    {
        for (int i = 0; i < SIZE/2; i++) {
            a[i] += b[i + SIZE/4];
        }
    }
    
    /* Update global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += a[i];
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int sum = 0;
    
    /* Allocate dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Initialize globals */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (mode > 0) {
        func_map();
        func_depend(5, 25);
        func_update();
        func_nested();
        func_multiple();
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += global_arr[i] + global_arr2[i];
        if (i < SIZE) sum += dynamic_arr[i]; /* Conditional to prevent optimization */
    }
    
    printf("Checksum: %d\n", sum);
    
    free(dynamic_arr);
    return 0;
}
