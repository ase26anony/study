/* omp_array_section_coverage.c
 * Designed to trigger OMP_ARRAY_SECTION node pretty-printing in GCC
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are present */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section with map clause on global array */
NOINLINE_USED
void func_map_global(void) {
    /* Map a section of global array */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple operation inside region */
        #pragma omp target teams distribute parallel for simd
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
}

/* Function 2: Array section with map clause on local array */
NOINLINE_USED
void func_map_local(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Map a section with variable bounds */
    int start = 5;
    int length = 75;
    #pragma omp target data map(local_arr[start:length])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = start; i < start + length; i++) {
            local_arr[i] *= 2;
        }
    }
}

/* Function 3: Array section with depend clause */
NOINLINE_USED
void func_depend(void) {
    /* Task with array section dependency */
    #pragma omp task depend(inout: global_arr2[0:SIZE/2])
    {
        for (int i = 0; i < SIZE/2; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    #pragma omp task depend(inout: global_arr2[SIZE/2:SIZE/2])
    {
        for (int i = SIZE/2; i < SIZE; i++) {
            global_arr2[i] = i * 4;
        }
    }
    
    #pragma omp taskwait
}

/* Function 4: Array section with update directive */
NOINLINE_USED
void func_update(void) {
    int update_arr[SIZE];
    
    /* Initialize on host */
    for (int i = 0; i < SIZE; i++) {
        update_arr[i] = i;
    }
    
    /* Update to device */
    #pragma omp target update to(update_arr[20:80])
    
    /* Modify on device */
    #pragma omp target teams distribute parallel for simd map(tofrom: update_arr[20:80])
    for (int i = 20; i < 100; i++) {
        update_arr[i] += 1000;
    }
    
    /* Update back to host */
    #pragma omp target update from(update_arr[20:80])
}

/* Function 5: Complex array section expressions */
NOINLINE_USED
void func_complex_expr(void) {
    int complex_arr[SIZE];
    int n = 10;
    int m = 5;
    
    /* Array section with arithmetic in bounds */
    #pragma omp target data map(complex_arr[2*n:CHUNK*m])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 2*n; i < 2*n + CHUNK*m; i++) {
            complex_arr[i] = i * i;
        }
    }
}

/* Function 6: Pointer-based array section */
NOINLINE_USED
void func_pointer_section(void) {
    int *ptr_arr = malloc(SIZE * sizeof(int));
    if (!ptr_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        ptr_arr[i] = i;
    }
    
    /* Array section on pointer */
    #pragma omp target data map(ptr_arr[30:70])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 30; i < 100; i++) {
            ptr_arr[i] += 500;
        }
    }
    
    free(ptr_arr);
}

/* Function 7: Nested array reference as base */
NOINLINE_USED
void func_nested_base(void) {
    int matrix[10][SIZE];
    
    /* Array section on a subscripted base (matrix[2]) */
    #pragma omp target data map(matrix[2][10:50])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 10; i < 60; i++) {
            matrix[2][i] = i * 10;
        }
    }
}

/* Function 8: Multiple array sections in one directive */
NOINLINE_USED
void func_multiple_sections(void) {
    int multi1[SIZE], multi2[SIZE];
    
    #pragma omp target data map(multi1[0:40], multi2[60:40])
    {
        #pragma omp target teams distribute parallel for simd
        for (int i = 0; i < 40; i++) {
            multi1[i] = i;
            multi2[60 + i] = i * 2;
        }
    }
}

/* Main function with volatile control to prevent dead code elimination */
int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (mode > 0) {
        func_map_global();
        func_map_local();
    }
    
    if (mode > 0) {
        #pragma omp parallel
        #pragma omp single
        {
            func_depend();
        }
    }
    
    if (mode > 0) {
        func_update();
        func_complex_expr();
    }
    
    if (mode > 0) {
        func_pointer_section();
        func_nested_base();
        func_multiple_sections();
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
