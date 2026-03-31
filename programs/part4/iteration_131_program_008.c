/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays - different base types */
int global_arr[SIZE];
double global_dbl_arr[SIZE];
static int static_arr[SIZE];

/* Prevent optimization */
volatile int use_func = 1;

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
        /* Do some work */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Another section with different base */
    #pragma omp target data map(global_arr[0:SIZE/2])
    {
        for (int i = 0; i < SIZE/2; i++) {
            global_arr[i] = local_arr[i];
        }
    }
}

/* Function 2: Depend directive with variable bounds */
__attribute__((noinline, used))
void func_depend(int start, int length) {
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_dbl_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_dbl_arr[i] = i * 3.14;
        }
    }
    
    /* Nested array section with arithmetic expression */
    int offset = 5;
    #pragma omp task depend(inout: static_arr[offset*2:CHUNK])
    {
        for (int i = offset*2; i < offset*2 + CHUNK; i++) {
            static_arr[i] = i * i;
        }
    }
}

/* Function 3: Update directive with pointer array section */
__attribute__((noinline, used))
void func_update(int *ptr, int len) {
    /* OMP_ARRAY_SECTION with pointer base */
    #pragma omp target update from(ptr[0:len])
    
    /* Multiple sections in same directive */
    #pragma omp target update \
        to(global_arr[20:30]) \
        from(global_dbl_arr[40:20])
}

/* Function 4: Complex array subscript as base */
__attribute__((noinline, used))
void func_complex_base(int idx) {
    int matrix[100][100];
    
    /* Base is ARRAY_REF, not simple VAR_DECL */
    #pragma omp target data map(matrix[idx][10:50])
    {
        for (int j = 10; j < 60; j++) {
            matrix[idx][j] = idx * 100 + j;
        }
    }
    
    /* Section with computed length */
    int section_len = idx > 10 ? 25 : 10;
    #pragma omp target update to(matrix[0][5:section_len])
}

/* Function 5: Target enter/exit data with sections */
__attribute__((noinline, used))
void func_target_data(void) {
    int dynamic_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i % 100;
    }
    
    /* Multiple array sections in map clause */
    #pragma omp target enter data map(to: \
        dynamic_arr[0:50], \
        dynamic_arr[100:50], \
        global_arr[150:25])
    
    /* Modify data */
    #pragma omp target
    for (int i = 0; i < 50; i++) {
        dynamic_arr[i] *= 2;
        dynamic_arr[100 + i] += 3;
        global_arr[150 + i] = i;
    }
    
    #pragma omp target exit data map(from: \
        dynamic_arr[0:50], \
        dynamic_arr[100:50])
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_dbl_arr[i] = 0.0;
        static_arr[i] = -1;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_func) {
        func_map();
    }
    
    if (use_func > 0) {
        func_depend(30, 40);
        #pragma omp taskwait
    }
    
    if (use_func != 0) {
        int *dynamic_ptr = malloc(SIZE * sizeof(int));
        for (int i = 0; i < SIZE; i++) {
            dynamic_ptr[i] = i * 2;
        }
        func_update(dynamic_ptr, 75);
        free(dynamic_ptr);
    }
    
    if (use_func == 1) {
        func_complex_base(3);
    }
    
    if (use_func >= 1) {
        func_target_data();
    }
    
    /* Compute checksum to ensure code executes */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i];
        checksum += (long long)global_dbl_arr[i];
        checksum += static_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
