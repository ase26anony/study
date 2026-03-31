/* omp_array_sections.c - Test program for OMP_ARRAY_SECTION node coverage */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000
#define CHUNK 100

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int global_arr3[SIZE];

/* Volatile variable to prevent optimization */
volatile int use_all = 1;

/* Function with target data map containing array section */
__attribute__((noinline, used))
void func_map(int *arr, int start, int length) {
    /* Use array section in map clause */
    #pragma omp target data map(arr[start:length])
    {
        /* Simple computation inside region */
        for (int i = start; i < start + length; i++) {
            arr[i] = arr[i] * 2;
        }
    }
}

/* Function with task depend containing array section */
__attribute__((noinline, used))
void func_depend(int *arr, int n, int size) {
    /* Array section in depend clause */
    #pragma omp task depend(inout: arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr[i] = arr[i] + i;
        }
    }
    
    /* Ensure task is executed */
    #pragma omp taskwait
}

/* Function with target update containing array section */
__attribute__((noinline, used))
void func_update(int *arr, int len) {
    /* Initialize data on device */
    #pragma omp target enter data map(to: arr[0:len])
    
    /* Array section in update clause */
    #pragma omp target update from(arr[0:len])
    
    #pragma omp target exit data map(delete: arr[0:len])
}

/* Function with complex array subscript as base */
__attribute__((noinline, used))
void func_complex_base(int *arr, int idx, int offset, int length) {
    /* Base is arr[idx] which becomes ARRAY_REF, testing op_prio logic */
    int *ptr = &arr[idx];
    
    #pragma omp target data map(ptr[offset:length])
    {
        for (int i = 0; i < length; i++) {
            ptr[offset + i] = ptr[offset + i] * 3;
        }
    }
}

/* Function with multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(int *a, int *b, int start1, int len1, int start2, int len2) {
    #pragma omp target data map(a[start1:len1], b[start2:len2])
    {
        for (int i = 0; i < len1; i++) {
            a[start1 + i] = a[start1 + i] - 1;
        }
        for (int i = 0; i < len2; i++) {
            b[start2 + i] = b[start2 + i] + 1;
        }
    }
}

/* Function with arithmetic in array section bounds */
__attribute__((noinline, used))
void func_arithmetic_bounds(int *arr, int n, int m) {
    /* Complex expressions in bounds */
    #pragma omp target data map(arr[2*n: m/2])
    {
        for (int i = 0; i < m/2; i++) {
            arr[2*n + i] = arr[2*n + i] * arr[2*n + i];
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
        global_arr3[i] = i * 2;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (use_all) {
        /* Test 1: Simple array section with constants */
        func_map(global_arr, 10, CHUNK);
        
        /* Test 2: Array section with variables */
        int n = 50;
        int size = 200;
        func_depend(global_arr2, n, size);
        
        /* Test 3: Full array section */
        func_update(global_arr3, SIZE);
        
        /* Test 4: Complex base (array subscript) */
        func_complex_base(global_arr, 100, 10, 50);
        
        /* Test 5: Multiple sections */
        func_multiple_sections(global_arr, global_arr2, 0, 100, 200, 100);
        
        /* Test 6: Arithmetic in bounds */
        func_arithmetic_bounds(global_arr3, 10, 100);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + global_arr3[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
