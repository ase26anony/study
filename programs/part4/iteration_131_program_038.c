/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define SECTION_SIZE 50

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Volatile variable to prevent optimization */
volatile int use_all = 1;

/* Function with array section in map clause */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Array section with constant bounds */
    #pragma omp target data map(local_arr[10:SECTION_SIZE])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 10 + SECTION_SIZE; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Copy back to global for verification */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function with array section in depend clause */
__attribute__((noinline, used))
void func_depend(void) {
    int n = 5;
    int size = 30;
    
    /* Array section with variable bounds */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Another task with different array section */
    #pragma omp task depend(inout: global_arr2[2*n:size/2])
    {
        for (int i = 2*n; i < 2*n + size/2; i++) {
            global_arr2[i] += 1;
        }
    }
    
    #pragma omp taskwait
}

/* Function with array section in update clause */
__attribute__((noinline, used))
void func_update(void) {
    int len = 40;
    
    /* Initialize dynamic array */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 4;
    }
    
    /* Array section on pointer with arithmetic in bounds */
    #pragma omp target update to(dynamic_arr[2*len:SIZE/4])
    
    /* Another update with different section */
    int start = 10;
    #pragma omp target update from(dynamic_arr[start:len-start])
}

/* Function with nested array reference as base */
__attribute__((noinline, used))
void func_nested_base(void) {
    int matrix[SIZE][SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    /* Array section on a subscripted array reference */
    #pragma omp target data map(matrix[5][10:SECTION_SIZE])
    {
        for (int j = 10; j < 10 + SECTION_SIZE; j++) {
            matrix[5][j] *= 2;
        }
    }
    
    /* Copy to global for verification */
    for (int j = 0; j < SIZE; j++) {
        global_arr[j] += matrix[5][j];
    }
}

/* Function with multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Multiple array sections in single map clause */
    #pragma omp target data map(arr1[0:20], arr2[10:30], arr3[5*2:40])
    {
        for (int i = 0; i < 20; i++) arr1[i] += 1;
        for (int i = 10; i < 40; i++) arr2[i] += 2;
        for (int i = 10; i < 50; i++) arr3[i] += 3;
    }
    
    /* Update global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += arr1[i] + arr2[i] + arr3[i];
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
    }
    
    /* Allocate dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    if (!dynamic_arr) return 1;
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_all) {
        func_map();
        
        #pragma omp parallel
        {
            #pragma omp single
            func_depend();
        }
        
        func_update();
        func_nested_base();
        func_multiple_sections();
    }
    
    /* Compute checksum to verify execution */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (dynamic_arr) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
