/* omp_array_section_coverage.c
 * Designed to trigger OMP_ARRAY_SECTION node pretty-printing in GCC */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible across scopes */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent dead code elimination and inlining */
__attribute__((noinline, used))
void func_map_simple(void) {
    /* Simple array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Dummy operation to prevent empty region optimization */
        global_arr[10] = 1;
    }
}

__attribute__((noinline, used))
void func_map_complex(int offset, int length) {
    /* Array section with variable bounds */
    #pragma omp target data map(global_arr2[offset:length])
    {
        global_arr2[offset] = offset;
    }
}

__attribute__((noinline, used))
void func_depend_section(int n, int size) {
    /* Array section in depend clause */
    #pragma omp task depend(inout: dynamic_arr[n:size])
    {
        for (int i = 0; i < size; i++) {
            dynamic_arr[n + i] = i;
        }
    }
}

__attribute__((noinline, used))
void func_update_section(int start, int len) {
    /* Array section in target update */
    #pragma omp target update from(global_arr[start:len])
    {
        /* No body needed for update directive */
    }
}

__attribute__((noinline, used))
void func_multiple_sections(void) {
    /* Multiple array sections in same directive */
    int local_arr[SIZE];
    
    #pragma omp target data \
        map(local_arr[0:SIZE]) \
        map(global_arr[SIZE/4:SIZE/2])
    {
        for (int i = 0; i < SIZE/2; i++) {
            local_arr[i] = global_arr[SIZE/4 + i];
        }
    }
}

__attribute__((noinline, used))
void func_array_ref_section(int idx) {
    /* Array section where base is an ARRAY_REF (more complex op0) */
    int matrix[10][SIZE];
    
    #pragma omp target data map(matrix[idx][0:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            matrix[idx][i] = idx * 100 + i;
        }
    }
}

__attribute__((noinline, used))
void func_pointer_section(int *ptr, int start, int count) {
    /* Array section on pointer parameter */
    #pragma omp target data map(ptr[start:count])
    {
        for (int i = 0; i < count; i++) {
            ptr[start + i] = start + i;
        }
    }
}

int main(void) {
    volatile int mode = 0; /* Prevent constant folding */
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = 0;
    }
    
    /* Call different functions based on volatile variable
     * Ensures all code paths are present in compilation unit */
    if (mode == 0) {
        func_map_simple();
    }
    
    if (mode < 1) {
        func_map_complex(20, 75);
    }
    
    if (mode < 2) {
        #pragma omp parallel
        #pragma omp single
        {
            func_depend_section(5, 30);
        }
    }
    
    if (mode < 3) {
        func_update_section(40, 60);
    }
    
    if (mode < 4) {
        func_multiple_sections();
    }
    
    if (mode < 5) {
        func_array_ref_section(3);
    }
    
    if (mode < 6) {
        int local_array[SIZE];
        func_pointer_section(local_array, 10, 40);
    }
    
    /* Compute checksum to ensure arrays are used */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < SIZE && dynamic_arr) {
            checksum += dynamic_arr[i];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
