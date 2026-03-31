/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP directives */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent dead code elimination */
volatile int use_func = 1;

/* Function 1: Map directive with array section on global array */
__attribute__((noinline, used))
void func_map(void) {
    /* Use array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i * 2;
        }
    }
    
    /* Another map with different bounds */
    int start = 5;
    int length = 75;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] += 1;
        }
    }
}

/* Function 2: Depend directive with array section on pointer array */
__attribute__((noinline, used))
void func_depend(void) {
    int n = 20;
    int size = 80;
    
    /* Task with depend clause using array section */
    #pragma omp task depend(inout: dynamic_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            if (i < SIZE) dynamic_arr[i] = i * 3;
        }
    }
    
    /* Another depend with computed bounds */
    int offset = SIZE/4;
    #pragma omp task depend(inout: dynamic_arr[2*offset:CHUNK])
    {
        for (int i = 2*offset; i < 2*offset + CHUNK; i++) {
            if (i < SIZE) dynamic_arr[i] += 2;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(void) {
    int len = SIZE/2;
    
    /* Update from array section */
    #pragma omp target update from(global_arr2[0:len])
    
    /* Update to array section with expression */
    int half = len/2;
    #pragma omp target update to(global_arr2[half:len-half])
}

/* Function 4: Combined construct with multiple array sections */
__attribute__((noinline, used))
void func_combined(void) {
    /* Target with multiple map clauses containing array sections */
    #pragma omp target map(global_arr[0:SIZE/2]) \
                       map(global_arr2[SIZE/4:3*SIZE/4]) \
                       map(dynamic_arr[10:SIZE-20])
    {
        for (int i = 0; i < SIZE/2; i++) {
            global_arr[i] = global_arr2[i + SIZE/4] + dynamic_arr[i + 10];
        }
    }
}

/* Function 5: Array section with ARRAY_REF as base */
__attribute__((noinline, used))
void func_array_ref_base(void) {
    int matrix[100][100];
    
    /* This should create OMP_ARRAY_SECTION with ARRAY_REF as operand 0 */
    #pragma omp target data map(matrix[10][20:30])
    {
        for (int i = 0; i < 30; i++) {
            matrix[10][20 + i] = i;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * i;
    }
    
    /* Call functions based on volatile variable to prevent optimization */
    if (use_func & 1) func_map();
    if (use_func & 2) func_depend();
    if (use_func & 4) func_update();
    if (use_func & 8) func_combined();
    if (use_func & 16) func_array_ref_base();
    
    /* Compute checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < SIZE) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
