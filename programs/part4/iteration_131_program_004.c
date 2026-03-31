/* omp_array_section_test.c
 * Tests GCC's OMP_ARRAY_SECTION pretty-printing
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're not optimized away */
int global_arr[SIZE];
int global_arr2[SIZE];
int* global_ptr;

/* Prevent inlining to ensure code generation */
__attribute__((noinline, used))
void func_map_simple(void) {
    /* Simple array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i;
        }
    }
}

__attribute__((noinline, used))
void func_map_complex(void) {
    int local_arr[SIZE];
    int n = 5;
    
    /* Array section with variable bounds and arithmetic */
    #pragma omp target data map(local_arr[2*n:CHUNK])
    {
        for (int i = 2*n; i < 2*n + CHUNK; i++) {
            local_arr[i] = i * 2;
        }
    }
    
    /* Copy back to global for observable effect */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

__attribute__((noinline, used))
void func_depend(void) {
    int n = 20;
    int size = 30;
    
    /* Task with array section dependency */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] *= 2;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(void) {
    int local_arr3[SIZE];
    int len = SIZE/2;
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr3[i] = i * 3;
    }
    
    /* Target update with array section */
    #pragma omp target update from(local_arr3[0:len])
    
    /* Use the data to prevent elimination */
    for (int i = 0; i < len; i++) {
        global_arr[i] += local_arr3[i];
    }
}

__attribute__((noinline, used))
void func_nested_sections(void) {
    int matrix[SIZE][SIZE];
    int row = 5;
    int col_start = 10;
    int col_len = 20;
    
    /* Multiple array sections in different contexts */
    #pragma omp target data \
        map(matrix[row][col_start:col_len]) \
        map(matrix[row+1][0:SIZE])
    {
        for (int j = col_start; j < col_start + col_len; j++) {
            matrix[row][j] = row * 100 + j;
        }
        for (int j = 0; j < SIZE; j++) {
            matrix[row+1][j] = (row+1) * 100 + j;
        }
    }
    
    /* Extract to global */
    for (int j = 0; j < SIZE; j++) {
        global_arr[j] += matrix[row][j] + matrix[row+1][j];
    }
}

__attribute__((noinline, used))
void func_pointer_section(void) {
    int dynamic[SIZE];
    global_ptr = dynamic;
    int offset = 15;
    int length = 40;
    
    /* Array section on pointer */
    #pragma omp target data map(global_ptr[offset:length])
    {
        for (int i = offset; i < offset + length; i++) {
            global_ptr[i] = i * i;
        }
    }
    
    /* Copy to global */
    for (int i = 0; i < length; i++) {
        global_arr[offset + i] += global_ptr[offset + i];
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
    }
    
    /* Call different functions based on volatile variable
     * to ensure all are compiled but execution varies */
    if (mode & 1) func_map_simple();
    if (mode & 2) func_map_complex();
    if (mode & 4) func_depend();
    if (mode & 8) func_update();
    if (mode & 16) func_nested_sections();
    if (mode & 32) func_pointer_section();
    
    /* Compute checksum for observable side effect */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
