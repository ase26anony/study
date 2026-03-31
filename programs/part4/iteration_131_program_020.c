/* omp_array_sections.c
 * Generates various OpenMP array sections to exercise OMP_ARRAY_SECTION
 * pretty-printing in GCC's tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays - different base cases */
int global_arr[SIZE];
int global_arr2[SIZE];
int *global_ptr;

/* Prevent optimization */
volatile int use_global = 1;

/* Non-inlineable functions with different array section patterns */
__attribute__((noinline, used))
void func_map_simple(void) {
    /* Simple array section with constants */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
}

__attribute__((noinline, used))
void func_map_complex(void) {
    int local_arr[SIZE];
    int n = 5;
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Array section with variable expressions */
    #pragma omp target data map(local_arr[2*n:CHUNK])
    {
        for (int i = 2*n; i < 2*n + CHUNK; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Copy back to global for observable effect */
    if (use_global) {
        for (int i = 0; i < SIZE; i++) {
            global_arr[i] += local_arr[i];
        }
    }
}

__attribute__((noinline, used))
void func_depend(void) {
    int local = 20;
    int len = 30;
    
    /* Task with array section dependency */
    #pragma omp task depend(inout: global_arr2[local:len])
    {
        for (int i = local; i < local + len; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(void) {
    int update_arr[SIZE];
    int start = SIZE/4;
    int count = SIZE/2;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        update_arr[i] = i * 4;
    }
    
    /* Update with array section */
    #pragma omp target update to(update_arr[start:count])
    
    /* Simulate some device computation */
    #pragma omp target teams distribute parallel for map(from:update_arr[start:count])
    for (int i = start; i < start + count; i++) {
        update_arr[i] += 1000;
    }
    
    #pragma omp target update from(update_arr[start:count])
    
    /* Accumulate results */
    if (use_global) {
        for (int i = 0; i < SIZE; i++) {
            global_arr[i] += update_arr[i];
        }
    }
}

__attribute__((noinline, used))
void func_pointer_section(void) {
    int dyn_size = 150;
    int *dyn_arr = malloc(dyn_size * sizeof(int));
    
    if (!dyn_arr) return;
    
    /* Initialize dynamic array */
    for (int i = 0; i < dyn_size; i++) {
        dyn_arr[i] = i * 5;
    }
    
    /* Array section on pointer with arithmetic in bounds */
    int offset = dyn_size / 3;
    #pragma omp target data map(dyn_arr[offset:dyn_size/3])
    {
        for (int i = offset; i < offset + dyn_size/3; i++) {
            dyn_arr[i] += 500;
        }
    }
    
    /* Cleanup */
    free(dyn_arr);
}

__attribute__((noinline, used))
void func_nested_section(void) {
    int matrix[100][100];
    int row = 10;
    int col_start = 20;
    int col_len = 40;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Array section on multi-dimensional array */
    #pragma omp target data map(matrix[row][col_start:col_len])
    {
        for (int j = col_start; j < col_start + col_len; j++) {
            matrix[row][j] += 10000;
        }
    }
    
    /* Accumulate to global */
    if (use_global) {
        for (int j = 0; j < 100; j++) {
            global_arr[j % SIZE] += matrix[row][j];
        }
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
    }
    
    global_ptr = malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        global_ptr[i] = i * 3;
    }
    
    /* Force all functions to be compiled and called */
    #pragma omp parallel
    {
        #pragma omp single
        {
            func_map_simple();
            func_map_complex();
            func_depend();
            func_update();
            func_pointer_section();
            func_nested_section();
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (global_ptr) checksum += global_ptr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    if (global_ptr) free(global_ptr);
    return 0;
}
