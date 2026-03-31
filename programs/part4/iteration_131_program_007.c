/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to test different base types */
int global_arr[SIZE];
int *global_ptr;

/* Prevent dead code elimination */
volatile int use_all = 0;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Copy to global for verification */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function 2: Depend clause with array section using variables */
__attribute__((noinline, used))
void func_depend(int start, int length) {
    int dynamic_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: dynamic_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            dynamic_arr[i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    /* Accumulate to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += dynamic_arr[i];
    }
}

/* Function 3: Update directive with pointer array section */
__attribute__((noinline, used))
void func_update(void) {
    int heap_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        heap_arr[i] = i * 4;
    }
    
    global_ptr = heap_arr;
    
    /* OMP_ARRAY_SECTION on pointer with expression bounds */
    #pragma omp target update from(global_ptr[2*CHUNK:SIZE/2])
    
    /* Process the updated section */
    for (int i = 2*CHUNK; i < 2*CHUNK + SIZE/2; i++) {
        global_arr[i] += heap_arr[i];
    }
}

/* Function 4: Complex array section with ARRAY_REF as base */
__attribute__((noinline, used))
void func_complex_section(void) {
    int matrix[SIZE][SIZE];
    int n = 5;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    
    /* OMP_ARRAY_SECTION where base is an ARRAY_REF (matrix[n]) */
    #pragma omp target data map(matrix[n][10:50])
    {
        for (int j = 10; j < 60; j++) {
            matrix[n][j] += 100;
        }
    }
    
    /* Accumulate to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += matrix[n][i % SIZE];
    }
}

/* Function 5: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int arr1[SIZE], arr2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
    }
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(arr1[0:SIZE/2], arr2[SIZE/4:3*SIZE/4])
    {
        for (int i = 0; i < SIZE/2; i++) arr1[i] *= 3;
        for (int i = SIZE/4; i < SIZE; i++) arr2[i] *= 2;
    }
    
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += arr1[i] + arr2[i];
    }
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
    }
    
    /* Force all functions to be called based on volatile variable */
    if (use_all) {
        func_map();
        func_depend(20, 75);
        func_update();
        func_complex_section();
        func_multiple_sections();
    } else {
        /* Call them anyway but with different conditions */
        for (int i = 0; i < 3; i++) {
            func_map();
            func_depend(i * 10, 30);
        }
        func_update();
        func_complex_section();
        func_multiple_sections();
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
