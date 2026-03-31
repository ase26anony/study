/* omp_array_section_test.c
 * Generates various OpenMP array sections to exercise OMP_ARRAY_SECTION
 * pretty-printing in tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays - different base types */
int global_arr[SIZE];
double global_dbl_arr[SIZE];
static int static_arr[SIZE];

/* Prevent optimization and ensure functions are compiled */
#define KEEP __attribute__((used, noinline))

/* Function 1: Map directive with array section */
void KEEP func_map(int n, int len) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Multiple array sections with different bases and expressions */
    #pragma omp target data map(global_arr[n:len]) \
                            map(local_arr[0:SIZE]) \
                            map(static_arr[10:CHUNK])
    {
        /* Nested array section in update */
        #pragma omp target update to(global_dbl_arr[n*2:len/2])
        
        /* Simple computation to prevent dead code elimination */
        global_arr[n] = local_arr[0] + static_arr[10];
    }
}

/* Function 2: Task depend with array sections */
void KEEP func_depend(int start, int count) {
    int* dynamic_arr = malloc(SIZE * sizeof(int));
    
    if (!dynamic_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i;
    }
    
    /* Array section on pointer with variable bounds */
    #pragma omp task depend(inout: dynamic_arr[start:count]) \
                     depend(in: global_arr[0:start])
    {
        for (int i = start; i < start + count; i++) {
            dynamic_arr[i] *= 2;
        }
    }
    
    /* Wait for task completion */
    #pragma omp taskwait
    
    free(dynamic_arr);
}

/* Function 3: Target update with complex expressions */
void KEEP func_update(int base, int offset) {
    int matrix[SIZE][SIZE];
    
    /* Initialize matrix */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    
    /* Array section with arithmetic in bounds */
    #pragma omp target update from(matrix[base][offset:CHUNK*2]) \
                             to(global_dbl_arr[base+offset:SIZE-base])
    
    /* Multiple sections in one directive */
    #pragma omp target data map(matrix[base:CHUNK][offset:CHUNK])
    {
        /* Nested array ref as base for array section */
        int* row = matrix[base];
        #pragma omp target update to(row[offset:CHUNK])
    }
}

/* Function 4: Combined construct with array sections */
void KEEP func_combined(int idx) {
    struct {
        int data[SIZE];
        int count;
    } wrapper;
    
    wrapper.count = SIZE;
    for (int i = 0; i < SIZE; i++) {
        wrapper.data[i] = i * 3;
    }
    
    /* Array section on struct member */
    #pragma omp target enter data map(to: wrapper.data[idx:wrapper.count-idx])
    
    /* Task with depend on array section */
    #pragma omp task depend(inout: wrapper.data[idx*2:20])
    {
        wrapper.data[idx*2] += 100;
    }
    
    #pragma omp taskwait
    #pragma omp target exit data map(from: wrapper.data[idx:wrapper.count-idx])
}

/* Main function with volatile control to prevent dead code elimination */
int main() {
    volatile int mode = 0;  /* Prevent constant folding */
    int result = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_dbl_arr[i] = i * 1.5;
        static_arr[i] = i * 2;
    }
    
    /* Call different functions based on volatile variable
     * This ensures all functions are compiled into the binary */
    if (mode == 0) {
        func_map(10, 100);
        func_depend(5, 30);
        func_update(2, 15);
        func_combined(3);
    } else {
        /* Alternative path to ensure all code is reachable in analysis */
        func_map(20, 50);
        func_depend(0, SIZE);
        func_update(1, 10);
        func_combined(1);
    }
    
    /* Compute checksum to verify execution */
    for (int i = 0; i < SIZE; i++) {
        result += global_arr[i] + (int)global_dbl_arr[i] + static_arr[i];
    }
    
    printf("Result checksum: %d\n", result);
    
    return 0;
}
