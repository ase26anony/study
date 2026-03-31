/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to test different base types */
int global_arr[SIZE];
int *global_ptr;

/* Prevent optimization and ensure functions are compiled */
#define KEEP __attribute__((used, noinline))

/* Function 1: Map directive with array section on global array */
void KEEP func_map(void) {
    /* Use array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple operation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i * 2;
        }
    }
}

/* Function 2: Depend directive with array section on local array */
void KEEP func_depend(int n, int size) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Use array section with variable bounds */
    #pragma omp task depend(inout: local_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            local_arr[i] *= 3;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section on pointer */
void KEEP func_update(int len) {
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    
    if (!dynamic_arr) return;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * i;
    }
    
    /* Use array section with arithmetic expression */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Another with different expression */
    int offset = 5;
    #pragma omp target update to(dynamic_arr[offset*2:len/2])
    
    free(dynamic_arr);
}

/* Function 4: Complex array section within target region */
void KEEP func_complex(void) {
    int matrix[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    /* Array section on 2D array access */
    #pragma omp target map(matrix[10:20][30:40])
    {
        for (int i = 10; i < 30; i++) {
            for (int j = 30; j < 70; j++) {
                matrix[i][j] += 1;
            }
        }
    }
}

/* Function 5: Multiple array sections in same directive */
void KEEP func_multi_sections(void) {
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Multiple array sections with different expressions */
    #pragma omp target data \
        map(arr1[0:SIZE/2]) \
        map(arr2[10:SIZE-20]) \
        map(arr3[2*CHUNK:3*CHUNK])
    {
        /* Do some work */
        for (int i = 0; i < SIZE/2; i++) {
            arr1[i] += arr2[i+10] + arr3[i+2*CHUNK];
        }
    }
}

/* Function 6: Array section with ARRAY_REF as base */
void KEEP func_array_ref_base(void) {
    int nested[5][SIZE];
    
    /* Initialize */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < SIZE; j++) {
            nested[i][j] = i * 100 + j;
        }
    }
    
    /* Base is an ARRAY_REF (nested[2]), section on second dimension */
    #pragma omp target update to(nested[2][10:50])
    
    /* Another with different index */
    int idx = 3;
    #pragma omp target update from(nested[idx][20:30])
}

/* Main function with volatile control to prevent dead code elimination */
int main(void) {
    volatile int mode = 1; /* Volatile to prevent constant folding */
    int checksum = 0;
    
    /* Initialize global arrays */
    global_ptr = malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        if (global_ptr) global_ptr[i] = i * 2;
    }
    
    /* Call different functions based on volatile variable */
    /* This ensures all functions are present in the compilation unit */
    if (mode > 0) {
        func_map();
    }
    
    if (mode > 1) {
        func_depend(5, 20);
    } else {
        func_update(30);
    }
    
    if (mode < 10) {
        func_complex();
    }
    
    func_multi_sections();
    func_array_ref_base();
    
    /* Compute checksum to prevent optimization and verify execution */
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i];
        if (global_ptr) checksum += global_ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    if (global_ptr) free(global_ptr);
    return 0;
}
