/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int use_func = 1;

/* Function 1: Target data with array section */
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
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Copy to global for observable effect */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function 2: Task with depend clause using array section */
__attribute__((noinline, used))
void func_depend(void) {
    static int shared_arr[SIZE];
    int n = 5;
    int length = SIZE / 2;
    
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: shared_arr[n:length])
    {
        for (int i = n; i < n + length; i++) {
            shared_arr[i] = i * 3;
        }
    }
    
    #pragma omp taskwait
    
    /* Copy to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr2[i] = shared_arr[i];
    }
}

/* Function 3: Target update with complex array section expression */
__attribute__((noinline, used))
void func_update(void) {
    int dynamic_arr[SIZE];
    int start = 20;
    int len = SIZE - 40;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with arithmetic in bounds */
    #pragma omp target update to(dynamic_arr[start*2:len/2])
    
    /* Simulate device computation */
    #pragma omp target teams distribute parallel for
    for (int i = start*2; i < start*2 + len/2; i++) {
        dynamic_arr[i] *= 2;
    }
    
    #pragma omp target update from(dynamic_arr[0:SIZE])
    
    /* Accumulate to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += dynamic_arr[i];
    }
}

/* Function 4: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    int idx = 15;
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(arr1[idx:30], arr2[0:SIZE], arr3[SIZE/4:SIZE/2])
    {
        /* Nested array section access */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < 30; i++) {
            arr1[idx + i] += arr2[i * 2];
        }
    }
    
    /* Add to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr2[i] += arr1[i] + arr3[i];
    }
}

/* Function 5: Array section with pointer base */
__attribute__((noinline, used))
void func_pointer_section(void) {
    int *ptr_arr = (int*)malloc(SIZE * sizeof(int));
    int offset = 25;
    
    if (!ptr_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        ptr_arr[i] = i * 4;
    }
    
    /* OMP_ARRAY_SECTION with pointer base */
    #pragma omp target data map(ptr_arr[offset:SIZE-offset])
    {
        #pragma omp target teams distribute parallel for
        for (int i = offset; i < SIZE; i++) {
            ptr_arr[i] += 5;
        }
    }
    
    /* Copy and free */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += ptr_arr[i];
    }
    
    free(ptr_arr);
}

int main(void) {
    /* Initialize globals */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_func & 1) func_map();
    if (use_func & 2) func_depend();
    if (use_func & 4) func_update();
    if (use_func & 8) func_multiple_sections();
    if (use_func & 16) func_pointer_section();
    
    /* Compute checksum for observable output */
    long long checksum1 = 0, checksum2 = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum1 += global_arr[i];
        checksum2 += global_arr2[i];
    }
    
    printf("Checksum 1: %lld\n", checksum1);
    printf("Checksum 2: %lld\n", checksum2);
    
    return 0;
}
