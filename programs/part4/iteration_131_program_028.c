/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int trigger = 1;

/* Function with array section in map clause */
__attribute__((noinline, used))
void func_map(int *arr, int n) {
    #pragma omp target data map(arr[n:CHUNK])
    {
        /* Simple computation inside region */
        for (int i = n; i < n + CHUNK; i++) {
            arr[i] = i * 2;
        }
    }
}

/* Function with array section in depend clause */
__attribute__((noinline, used))
void func_depend(int *arr, int start, int length) {
    #pragma omp task depend(inout: arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            arr[i] = arr[i] + 1;
        }
    }
}

/* Function with array section in to/from clauses */
__attribute__((noinline, used))
void func_update(int *arr, int offset, int len) {
    #pragma omp target update to(arr[offset:len])
    
    /* Simulate some target computation */
    #pragma omp target map(from: arr[offset:len])
    {
        for (int i = offset; i < offset + len; i++) {
            arr[i] = arr[i] * 3;
        }
    }
    
    #pragma omp target update from(arr[offset:len])
}

/* Function with nested array reference as base */
__attribute__((noinline, used))
void func_nested_base(int **ptr_arr, int idx, int len) {
    int *ptr = ptr_arr[idx];
    #pragma omp target data map(ptr[10:len])
    {
        for (int i = 10; i < 10 + len; i++) {
            ptr[i] = idx * 100 + i;
        }
    }
}

/* Function with complex subscript expressions */
__attribute__((noinline, used))
void func_complex_expr(int *arr, int n) {
    int start = n * 2;
    int length = SIZE / (n + 1);
    
    #pragma omp task depend(inout: arr[start:length])
    {
        for (int i = start; i < start + length && i < SIZE; i++) {
            arr[i] = arr[i] - n;
        }
    }
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 10;
    }
    
    int local_arr[SIZE];
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    int *ptr_array[3];
    
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 5;
        dynamic_arr[i] = i * 7;
    }
    
    ptr_array[0] = global_arr;
    ptr_array[1] = local_arr;
    ptr_array[2] = dynamic_arr;
    
    /* Call functions with different array sections */
    if (trigger) {
        /* Simple array section with constant bounds */
        func_map(global_arr, 10);
        
        /* Array section with variable bounds */
        func_depend(global_arr2, 20, 30);
        
        /* Array section with offset and length */
        func_update(local_arr, 0, 40);
        
        /* Nested base (ARRAY_REF -> INDIRECT_REF) */
        func_nested_base(ptr_array, 1, 25);
        
        /* Complex subscript expressions */
        func_complex_expr(dynamic_arr, 3);
    }
    
    /* Ensure tasks complete */
    #pragma omp taskwait
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i];
        if (i < SIZE && dynamic_arr) {
            checksum += dynamic_arr[i];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
