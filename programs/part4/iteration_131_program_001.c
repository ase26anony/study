/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to test different base types */
int global_arr[SIZE];
int *global_ptr;

/* Prevent dead code elimination */
volatile int use_all = 0;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Copy back to global for verification */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function 2: Depend directive with variable bounds */
__attribute__((noinline, used))
void func_depend(int n, int size) {
    int arr2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr2[i] = i * 3;
    }
    
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr2[i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    /* Accumulate results */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += arr2[i];
    }
}

/* Function 3: Update directive with pointer base */
__attribute__((noinline, used))
void func_update(int len) {
    int *arr3 = malloc(SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        arr3[i] = i * 4;
    }
    
    /* OMP_ARRAY_SECTION with pointer base */
    #pragma omp target update from(arr3[0:len])
    
    /* Another section with arithmetic expression */
    int start = 5;
    #pragma omp target update to(arr3[start:len/2])
    
    /* Accumulate results */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += arr3[i];
    }
    
    free(arr3);
}

/* Function 4: Complex array section with ARRAY_REF base */
__attribute__((noinline, used))
void func_complex(void) {
    int matrix[10][20];
    
    /* Initialize matrix */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 20 + j;
        }
    }
    
    /* OMP_ARRAY_SECTION on array subscript (ARRAY_REF base) */
    #pragma omp target data map(matrix[2][5:10])
    {
        for (int j = 5; j < 15; j++) {
            matrix[2][j] += 100;
        }
    }
    
    /* Accumulate results */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            global_arr[i * 20 + j] += matrix[i][j];
        }
    }
}

/* Function 5: Multiple array sections in one directive */
__attribute__((noinline, used))
void func_multiple(void) {
    int a[SIZE], b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 5;
        b[i] = i * 6;
    }
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(a[0:50], b[25:75])
    {
        for (int i = 0; i < 50; i++) a[i] += 2;
        for (int i = 25; i < 100; i++) b[i] += 3;
    }
    
    /* Accumulate results */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += a[i] + b[i];
    }
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
    }
    
    /* Force all functions to be compiled by using volatile variable */
    if (use_all) {
        func_map();
        func_depend(20, 30);
        func_update(40);
        func_complex();
        func_multiple();
    } else {
        /* Execute all functions to ensure code generation */
        func_map();
        func_depend(20, 30);
        func_update(40);
        func_complex();
        func_multiple();
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
