/* omp_array_sections.c - Test program for OMP_ARRAY_SECTION node coverage */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP directives */
int global_arr[SIZE];
int global_arr2[SIZE];
int global_arr3[SIZE];

/* Volatile variable to prevent constant folding and dead code elimination */
volatile int selector = 0;

/* Function with array section in map clause */
__attribute__((noinline, used))
void func_map(int *arr, int n) {
    /* Use array section with variable lower bound and constant length */
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
void func_depend(int *arr, int start, int len) {
    /* Array section with both bounds as expressions */
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = start; i < start + len; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
    #pragma omp taskwait
}

/* Function with array section in target update */
__attribute__((noinline, used))
void func_update(int *arr, int offset, int length) {
    /* Array section with arithmetic in bounds */
    #pragma omp target update to(arr[offset:length/2])
    {
        /* Dummy operation */
        for (int i = offset; i < offset + length/2; i++) {
            arr[i] += 5;
        }
    }
}

/* Function with nested array reference as base */
__attribute__((noinline, used))
void func_nested_base(int **arr_ptr, int idx) {
    int *ptr = arr_ptr[idx];
    /* Base is ARRAY_REF (ptr[0]), not just VAR_DECL */
    #pragma omp target data map(ptr[10:20])
    {
        for (int i = 10; i < 30; i++) {
            ptr[i] = idx * 100 + i;
        }
    }
}

/* Function with multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(int *a, int *b, int *c) {
    /* Multiple array sections with different expressions */
    #pragma omp target data map(a[0:SIZE/4], b[SIZE/4:SIZE/2], c[SIZE/2:SIZE*3/4])
    {
        for (int i = 0; i < SIZE/4; i++) a[i] += 1;
        for (int i = SIZE/4; i < SIZE*3/4; i++) b[i] += 2;
        for (int i = SIZE/2; i < SIZE*3/4; i++) c[i] += 3;
    }
}

int main(void) {
    int local_arr[SIZE];
    int local_arr2[SIZE];
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    int *ptr_array[3];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
        global_arr3[i] = i * 3;
        local_arr[i] = i + 1;
        local_arr2[i] = i + 2;
        dynamic_arr[i] = i + 3;
    }
    
    ptr_array[0] = global_arr;
    ptr_array[1] = global_arr2;
    ptr_array[2] = global_arr3;
    
    /* Call functions with different array sections */
    /* Use selector to prevent constant folding but ensure all code is present */
    if (selector == 0) {
        /* Simple array section with constant bounds */
        func_map(global_arr, 10);
        
        /* Array section with variable bounds */
        func_depend(global_arr2, 20, 30);
        
        /* Array section with arithmetic in bounds */
        func_update(global_arr3, 5, 40);
        
        /* Nested base (array subscript as base) */
        func_nested_base(ptr_array, 1);
        
        /* Multiple sections in one directive */
        func_multiple_sections(local_arr, local_arr2, dynamic_arr);
    }
    
    /* Also test with local arrays */
    func_map(local_arr, 15);
    func_depend(local_arr2, 25, 35);
    
    /* Compute checksum to prevent optimization and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + global_arr3[i];
        checksum += local_arr[i] + local_arr2[i];
        if (i < SIZE) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
