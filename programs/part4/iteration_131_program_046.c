/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they persist */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section with map clause on global array */
NOINLINE_USED
void func_map(void) {
    /* Use array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple operation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i * 2;
        }
    }
    
    /* Another array section with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] += 3;
        }
    }
}

/* Function 2: Array section with depend clause */
NOINLINE_USED
void func_depend(void) {
    int n = 5;
    int size = 30;
    
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * i;
        }
    }
    
    /* Wait for task completion */
    #pragma omp taskwait
    
    /* Nested array section expression */
    int offset = 2;
    #pragma omp task depend(inout: global_arr2[offset*3:size/2])
    {
        for (int i = offset*3; i < offset*3 + size/2; i++) {
            global_arr2[i] += 1;
        }
    }
    #pragma omp taskwait
}

/* Function 3: Array section with update clause on dynamic array */
NOINLINE_USED
void func_update(void) {
    int len = CHUNK * 2;
    
    /* Allocate and initialize dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i % 10;
    }
    
    /* Update to device with array section */
    #pragma omp target update to(dynamic_arr[0:len])
    
    /* Modify on host */
    for (int i = 0; i < len; i++) {
        dynamic_arr[i] += 100;
    }
    
    /* Update from device with different section */
    #pragma omp target update from(dynamic_arr[CHUNK:len/2])
    
    free(dynamic_arr);
}

/* Function 4: Complex array section with ARRAY_REF base */
NOINLINE_USED
void func_complex_section(void) {
    int matrix[100][100];
    
    /* Array section on multi-dimensional array access */
    #pragma omp target data map(matrix[10][20:40])
    {
        for (int i = 20; i < 60; i++) {
            matrix[10][i] = 42;
        }
    }
    
    /* Pointer-based array section */
    int *ptr = &matrix[0][0];
    #pragma omp target update to(ptr[100:50])
}

/* Function 5: Mixed directives with array sections */
NOINLINE_USED
void func_mixed(void) {
    int arr1[SIZE], arr2[SIZE];
    
    /* Multiple array sections in same directive */
    #pragma omp target data map(arr1[0:50], arr2[25:75])
    {
        for (int i = 0; i < 50; i++) arr1[i] = i;
        for (int i = 25; i < 100; i++) arr2[i] = i * 2;
    }
    
    /* Array section in task depend with complex expression */
    int base = 10;
    #pragma omp task depend(inout: arr1[base+5:base*2])
    {
        for (int i = base+5; i < base+5 + base*2; i++) {
            arr1[i] *= 2;
        }
    }
    #pragma omp taskwait
}

/* Main function with volatile control to prevent dead code elimination */
int main(void) {
    volatile int run_all = 1;
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions conditionally to ensure all are compiled */
    if (run_all) {
        func_map();
        func_depend();
        func_update();
        func_complex_section();
        func_mixed();
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
