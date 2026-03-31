/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to test different base types */
int global_arr[SIZE];
int *global_ptr;

/* Prevent optimization of test functions */
__attribute__((noinline, used))
void test_map_sections(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Test 1: Simple array section with constants */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Empty region - just for node creation */
    }
    
    /* Test 2: Array section with variable bounds */
    int start = 5;
    int length = CHUNK;
    #pragma omp target data map(local_arr[start:length])
    {
        /* Empty region */
    }
    
    /* Test 3: Array section on global array */
    #pragma omp target data map(global_arr[0:SIZE])
    {
        /* Empty region */
    }
}

__attribute__((noinline, used))
void test_depend_sections(void) {
    int arr1[SIZE], arr2[SIZE];
    volatile int n = 10; /* volatile to prevent constant folding */
    
    /* Test 4: Array section in depend clause with expression */
    #pragma omp task depend(inout: arr1[n:CHUNK*2])
    {
        arr1[n] = 1;
    }
    
    /* Test 5: More complex subscript expression */
    #pragma omp task depend(inout: arr2[2*n:SIZE/4])
    {
        arr2[2*n] = 2;
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void test_update_sections(void) {
    static int static_arr[SIZE];
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    
    if (!dynamic_arr) return;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        static_arr[i] = i * 3;
        dynamic_arr[i] = i * 4;
    }
    
    /* Test 6: Array section on static array */
    #pragma omp target update to(static_arr[20:80])
    
    /* Test 7: Array section on pointer/dynamic array */
    int offset = 30;
    #pragma omp target update from(dynamic_arr[offset:SIZE-offset])
    
    free(dynamic_arr);
}

__attribute__((noinline, used))
void test_mixed_sections(void) {
    int multi_arr[3][SIZE];
    int *ptr_arr[SIZE];
    
    /* Test 8: Array section with ARRAY_REF as base (multi-dimensional) */
    #pragma omp target data map(multi_arr[1][5:CHUNK])
    {
        multi_arr[1][5] = 99;
    }
    
    /* Test 9: Array section on pointer array element */
    if (global_ptr) {
        #pragma omp target update to(global_ptr[15:25])
    }
}

__attribute__((noinline, used))
void test_complex_expressions(void) {
    int complex_arr[SIZE * 2];
    int idx = 5;
    
    /* Test 10: Complex expressions in bounds */
    #pragma omp target data map(complex_arr[idx*3:(SIZE/CHUNK)*10])
    {
        complex_arr[idx*3] = 100;
    }
    
    /* Test 11: Function call in bounds (though may be simplified) */
    int get_len(void) { return CHUNK; }
    #pragma omp target data map(complex_arr[0:get_len()])
    {
        complex_arr[0] = 200;
    }
}

int main(void) {
    volatile int selector = 0; /* Force all code paths to exist */
    
    /* Initialize global arrays */
    global_ptr = malloc(SIZE * sizeof(int));
    if (!global_ptr) return 1;
    
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_ptr[i] = i * 5;
    }
    
    /* Call test functions based on volatile variable 
       to prevent dead code elimination */
    if (selector == 0) {
        test_map_sections();
    }
    if (selector != 1) {
        test_depend_sections();
    }
    if (selector == 0 || selector == 2) {
        test_update_sections();
    }
    test_mixed_sections();
    test_complex_expressions();
    
    /* Compute checksum to ensure arrays are used */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(global_ptr);
    return 0;
}
