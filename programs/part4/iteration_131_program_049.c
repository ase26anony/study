/* omp_array_section_coverage.c
 * Generates OMP_ARRAY_SECTION nodes for GCC pretty-printer coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays for different base cases */
int global_arr[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure compilation */
volatile int select = 1;

/* Function 1: Array section with simple VAR_DECL base */
__attribute__((noinline, used))
void func_map_simple(void) {
    int local_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple computation to prevent elimination */
        local_arr[50] = local_arr[10] + 1;
    }
}

/* Function 2: Array section with ARRAY_REF base (nested subscript) */
__attribute__((noinline, used))
void func_map_nested(void) {
    int matrix[SIZE][SIZE];
    
    /* OMP_ARRAY_SECTION on nested array reference */
    #pragma omp target data map(matrix[5][10:50])
    {
        matrix[5][25] = 42;
    }
}

/* Function 3: Array section with variable bounds */
__attribute__((noinline, used))
void func_depend_variable(void) {
    int n = 25;
    int size = 75;
    
    /* OMP_ARRAY_SECTION with variable expressions */
    #pragma omp task depend(inout: global_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr[i] *= 2;
        }
    }
}

/* Function 4: Array section with pointer base */
__attribute__((noinline, used))
void func_update_pointer(void) {
    int len = CHUNK * 2;
    
    /* OMP_ARRAY_SECTION on pointer with arithmetic in bounds */
    #pragma omp target update from(dynamic_arr[CHUNK:len - CHUNK])
    {
        /* Empty but ensures directive is processed */
    }
}

/* Function 5: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int arr1[SIZE], arr2[SIZE];
    int start = 5;
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(arr1[start:30], arr2[0:SIZE/2])
    {
        arr1[start + 10] = arr2[SIZE/4];
    }
}

/* Function 6: Complex expression in bounds */
__attribute__((noinline, used))
void func_complex_bounds(void) {
    int arr[SIZE];
    int n = 10;
    
    /* OMP_ARRAY_SECTION with arithmetic in bounds */
    #pragma omp target data map(arr[2*n:sizeof(arr)/sizeof(int) - n*3])
    {
        arr[20] = 100;
    }
}

/* Function 7: Array section in target region */
__attribute__((noinline, used))
void func_target_region(void) {
    int arr[SIZE];
    
    /* OMP_ARRAY_SECTION inside target region */
    #pragma omp target map(arr[0:SIZE])
    {
        for (int i = 0; i < SIZE; i++) {
            arr[i] = i * 2;
        }
    }
}

/* Function 8: Array section with depend clause */
__attribute__((noinline, used))
void func_task_depend(void) {
    int arr[SIZE];
    int idx = 30;
    
    /* OMP_ARRAY_SECTION in depend clause */
    #pragma omp task depend(inout: arr[idx:20])
    {
        for (int i = idx; i < idx + 20; i++) {
            arr[i] += 1;
        }
    }
}

/* Main function that calls all test cases */
int main(void) {
    int result = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i * 3;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (select & 1) func_map_simple();
    if (select & 2) func_map_nested();
    if (select & 4) func_depend_variable();
    if (select & 8) func_update_pointer();
    if (select & 16) func_multiple_sections();
    if (select & 32) func_complex_bounds();
    if (select & 64) func_target_region();
    if (select & 128) func_task_depend();
    
    /* Ensure all tasks complete */
    #pragma omp taskwait
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        result += global_arr[i] + dynamic_arr[i % SIZE];
    }
    
    printf("Result checksum: %d\n", result);
    
    free(dynamic_arr);
    return 0;
}
