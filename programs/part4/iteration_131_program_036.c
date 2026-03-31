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
int *global_ptr;

/* Prevent inlining to ensure code generation */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Array section with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Array section with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(local_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Copy to global for observable effect */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

__attribute__((noinline, used))
void func_depend(void) {
    /* Array section in depend clause with expression */
    int n = 5;
    int size = SIZE / 4;
    
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Another depend with arithmetic expression */
    #pragma omp task depend(inout: global_arr2[2*n:size/2])
    {
        for (int i = 2*n; i < 2*n + size/2; i++) {
            global_arr2[i] += 7;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(void) {
    int local_arr3[SIZE];
    int len = SIZE - 10;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        local_arr3[i] = i * 5;
    }
    
    /* Array section in update directive */
    #pragma omp target update from(local_arr3[0:len])
    
    /* Multiple array sections in single directive */
    #pragma omp target update to(local_arr3[5:50], local_arr3[100:75])
    
    /* Complex expression in bounds */
    int offset = 30;
    #pragma omp target update from(local_arr3[offset:len-offset])
    
    /* Accumulate to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr3[i];
    }
}

__attribute__((noinline, used))
void func_pointer_section(void) {
    /* Dynamic array accessed through pointer */
    int *dyn_arr = malloc(SIZE * sizeof(int));
    if (!dyn_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        dyn_arr[i] = i * 7;
    }
    
    /* Array section on pointer with complex subscript */
    int idx = 15;
    #pragma omp target data map(dyn_arr[idx*2:CHUNK*3])
    {
        for (int i = idx*2; i < idx*2 + CHUNK*3; i++) {
            dyn_arr[i] -= 3;
        }
    }
    
    /* Nested array section - base is itself an array reference */
    int matrix[10][20];
    #pragma omp target data map(matrix[5][10:15])
    {
        for (int j = 10; j < 25; j++) {
            matrix[5][j] = j * 11;
        }
    }
    
    free(dyn_arr);
}

__attribute__((noinline, used))
void func_mixed_directives(void) {
    int arr4[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr4[i] = i;
    }
    
    /* Multiple array sections in map clause */
    #pragma omp target data map(arr4[0:50], arr4[100:50], arr4[150:30])
    {
        for (int i = 0; i < 50; i++) {
            arr4[i] += 1;
            arr4[100 + i] += 2;
            if (i < 30) arr4[150 + i] += 3;
        }
    }
    
    /* Array section in nowait depend */
    #pragma omp task depend(inout: arr4[25:75]) nowait
    {
        for (int i = 25; i < 100; i++) {
            arr4[i] *= 2;
        }
    }
    
    #pragma omp taskwait
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int checksum = 0;
    
    /* Initialize globals */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
    }
    
    global_ptr = malloc(SIZE * sizeof(int));
    if (global_ptr) {
        for (int i = 0; i < SIZE; i++) {
            global_ptr[i] = i * 13;
        }
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (mode & 1) func_map();
    if (mode & 2) func_depend();
    if (mode & 4) func_update();
    if (mode & 8) func_pointer_section();
    if (mode & 16) func_mixed_directives();
    
    /* Compute checksum for observable side effect */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (global_ptr) checksum += global_ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(global_ptr);
    return 0;
}
