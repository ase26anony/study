/* omp_array_section_test.c
 * Designed to trigger OMP_ARRAY_SECTION node pretty-printing
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're accessible */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Volatile variable to prevent optimization */
volatile int use_all = 1;

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
    int offset = 5;
    int length = CHUNK * 2;
    #pragma omp target data map(global_arr[offset:length])
    {
        for (int i = offset; i < offset + length; i++) {
            global_arr[i] += 1;
        }
    }
}

/* Function 2: Depend directive with array section on local array */
__attribute__((noinline, used))
void func_depend(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Task with depend clause using array section */
    #pragma omp task depend(inout: local_arr[0:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            local_arr[i] *= 3;
        }
    }
    
    /* Another task with expression in bounds */
    int n = 20;
    #pragma omp task depend(inout: local_arr[n:SIZE/4])
    {
        for (int i = n; i < n + SIZE/4; i++) {
            local_arr[i] += 5;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section on dynamic array */
__attribute__((noinline, used))
void func_update(void) {
    if (!dynamic_arr) return;
    
    int len = SIZE - 30;
    /* Update directive with array section */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Update with arithmetic expression in bounds */
    int start = 2 * CHUNK;
    #pragma omp target update to(dynamic_arr[start:CHUNK/2])
}

/* Function 4: Complex array section with ARRAY_REF as base */
__attribute__((noinline, used))
void func_complex_section(void) {
    int matrix[SIZE][SIZE];
    
    /* Array section where base is itself an array reference */
    #pragma omp target data map(matrix[10][5:CHUNK])
    {
        for (int j = 5; j < 5 + CHUNK; j++) {
            matrix[10][j] = 42;
        }
    }
    
    /* Multiple array sections in same directive */
    #pragma omp target data map(matrix[0][0:SIZE/2], matrix[1][SIZE/4:SIZE/4])
    {
        for (int j = 0; j < SIZE/2; j++) {
            matrix[0][j] = j;
            if (j < SIZE/4) {
                matrix[1][SIZE/4 + j] = j * 2;
            }
        }
    }
}

/* Function 5: Array section in target enter/exit data */
__attribute__((noinline, used))
void func_enter_exit(void) {
    int special_arr[SIZE];
    
    /* Target enter data with array section */
    #pragma omp target enter data map(to: special_arr[CHUNK:3*CHUNK])
    
    /* Modify data on device would go here */
    
    /* Target exit data with different section */
    #pragma omp target exit data map(from: special_arr[0:CHUNK])
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Allocate and initialize dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (use_all) {
        func_map();
        func_depend();
        func_update();
        func_complex_section();
        func_enter_exit();
    } else {
        /* This branch never executes but ensures functions aren't dead */
        func_map();
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (dynamic_arr) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dynamic_arr);
    
    return 0;
}
