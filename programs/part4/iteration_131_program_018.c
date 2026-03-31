/* omp_array_section_test.c
 * Designed to trigger OMP_ARRAY_SECTION node pretty-printing in GCC
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible across scopes */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are present in IR */
__attribute__((noinline, used))
void func_map_simple(void) {
    /* Simple array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
}

__attribute__((noinline, used))
void func_map_complex(int offset, int length) {
    /* Array section with variable bounds */
    int local_arr[SIZE];
    
    #pragma omp target data map(local_arr[offset:length])
    {
        for (int i = offset; i < offset + length; i++) {
            local_arr[i] = i * 2;
        }
    }
    
    /* Use the result to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += local_arr[i];
    }
}

__attribute__((noinline, used))
void func_depend(int n, int size) {
    /* Array section in depend clause */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] *= 3;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(int start, int len) {
    /* Array section in target update */
    #pragma omp target update from(global_arr[start:len])
    
    /* Also test 'to' direction */
    #pragma omp target update to(global_arr2[start:len/2])
}

__attribute__((noinline, used))
void func_pointer_arithmetic(int *ptr, int idx) {
    /* Array section on pointer with arithmetic in subscript */
    #pragma omp target data map(ptr[idx*2:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            ptr[idx*2 + i] = idx + i;
        }
    }
}

__attribute__((noinline, used))
void func_nested_sections(void) {
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[0:SIZE/2]) \
        map(global_arr2[SIZE/4:SIZE/2])
    {
        for (int i = 0; i < SIZE/2; i++) {
            global_arr[i] += global_arr2[i + SIZE/4];
        }
    }
}

__attribute__((noinline, used))
void func_mixed_expressions(int m, int n) {
    /* Array sections with complex bound expressions */
    int temp[SIZE];
    
    #pragma omp target data map(temp[m+n:SIZE-m-n])
    {
        for (int i = m+n; i < SIZE; i++) {
            temp[i] = i / (m+1);
        }
    }
    
    /* Another with division in length */
    #pragma omp target update to(temp[0:SIZE/(n+1)])
}

int main(void) {
    volatile int selector = 0; /* Prevent constant folding */
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (selector == 0) {
        func_map_simple();
        func_map_complex(5, 75);
        func_depend(20, 30);
        func_update(0, 100);
        func_pointer_arithmetic(dynamic_arr, 10);
        func_nested_sections();
        func_mixed_expressions(3, 7);
    }
    
    /* Compute checksum to ensure code executes */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < SIZE) checksum += dynamic_arr[i]; /* Conditional to prevent opt */
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
