/* omp_array_sections.c
 * Generates OpenMP array sections to cover OMP_ARRAY_SECTION pretty-printing
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_sections.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible across scopes */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Map directive with array section on global array */
NOINLINE_USED
void func_map_section(void) {
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
    int length = CHUNK;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] += 1;
        }
    }
}

/* Function 2: Depend clause with array section on local array */
NOINLINE_USED
void func_depend_section(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Task with depend clause using array section */
    #pragma omp task depend(inout: local_arr[20:30])
    {
        for (int i = 20; i < 50; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Another task with computed bounds */
    int idx = 5;
    int count = 25;
    #pragma omp task depend(inout: local_arr[idx:count])
    {
        for (int i = idx; i < idx + count; i++) {
            local_arr[i] += 3;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section on dynamic array */
NOINLINE_USED
void func_update_section(void) {
    if (!dynamic_arr) {
        dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    }
    
    /* Initialize dynamic array */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Update with array section - to clause */
    #pragma omp target update to(dynamic_arr[0:CHUNK])
    
    /* Update with array section - from clause */
    int offset = CHUNK;
    #pragma omp target update from(dynamic_arr[offset:SIZE-offset])
}

/* Function 4: Complex array section with ARRAY_REF as base */
NOINLINE_USED
void func_complex_section(void) {
    int matrix[SIZE][SIZE];
    
    /* Array section on 2D array access - creates ARRAY_REF as base */
    #pragma omp target data map(matrix[10][20:30])
    {
        for (int j = 20; j < 50; j++) {
            matrix[10][j] = 10 * 100 + j;
        }
    }
    
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[0:50]) \
        map(global_arr2[25:75])
    {
        for (int i = 0; i < 50; i++) {
            global_arr[i] = i;
            global_arr2[i+25] = i * 2;
        }
    }
}

/* Function 5: Array section in target enter/exit data */
NOINLINE_USED
void func_target_data_section(void) {
    int local_data[SIZE];
    
    /* Target enter data with array section */
    #pragma omp target enter data map(to: local_data[10:90])
    
    /* Modify data on device would go here */
    
    /* Target exit data with array section */
    #pragma omp target exit data map(from: local_data[10:90])
}

/* Main function that calls all test functions */
int main(void) {
    volatile int choice = 1; /* volatile to prevent constant folding */
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (choice > 0) {
        func_map_section();
    }
    if (choice > 0) {
        func_depend_section();
    }
    if (choice > 0) {
        func_update_section();
    }
    if (choice > 0) {
        func_complex_section();
    }
    if (choice > 0) {
        func_target_data_section();
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    if (dynamic_arr) {
        for (int i = 0; i < SIZE; i++) {
            checksum += dynamic_arr[i];
        }
        free(dynamic_arr);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
