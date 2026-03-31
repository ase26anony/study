/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define SECTION_SIZE 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int use_func = 1;

/* Function with array section in map clause */
__attribute__((noinline, used))
void func_map(int *arr, int start, int length) {
    #pragma omp target data map(arr[start:length])
    {
        /* Simple operation inside region */
        for (int i = start; i < start + length; i++) {
            arr[i] = arr[i] * 2;
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

/* Function with array section in target update */
__attribute__((noinline, used))
void func_update(int *arr, int start, int length) {
    #pragma omp target update to(arr[start:length])
    #pragma omp target update from(arr[start:length])
}

/* Function with complex subscript expression */
__attribute__((noinline, used))
void func_complex(int *arr, int n, int size) {
    int start = 2 * n;
    int length = size / 2;
    
    #pragma omp target data map(arr[start:length])
    {
        for (int i = 0; i < length; i++) {
            arr[start + i] = arr[start + i] * 3;
        }
    }
}

/* Function with array section on pointer */
__attribute__((noinline, used))
void func_pointer(int **ptr, int offset, int count) {
    int *arr = *ptr;
    
    #pragma omp target data map(arr[offset:count])
    {
        for (int i = 0; i < count; i++) {
            arr[offset + i] = arr[offset + i] - 1;
        }
    }
}

/* Function with multiple array sections */
__attribute__((noinline, used))
void func_multiple(int *arr1, int *arr2, int idx) {
    #pragma omp target data map(arr1[idx:10], arr2[20:30])
    {
        for (int i = 0; i < 10; i++) {
            arr1[idx + i] = arr1[idx + i] * 2;
        }
        for (int i = 0; i < 30; i++) {
            arr2[20 + i] = arr2[20 + i] + 3;
        }
    }
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Local array */
    int local_arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Dynamic array */
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_func) {
        /* Simple array section with constants */
        func_map(global_arr, 10, 100);
        
        /* Array section with variable bounds */
        int start = 5;
        int length = SECTION_SIZE;
        func_depend(global_arr2, start, length);
        
        /* Update with array section */
        func_update(local_arr, 0, SIZE/2);
        
        /* Complex subscript expression */
        func_complex(global_arr, 3, SIZE);
        
        /* Pointer-based array section */
        func_pointer(&dynamic_arr, 15, 25);
        
        /* Multiple array sections */
        func_multiple(global_arr, global_arr2, 30);
        
        /* Nested array section (array of pointers) */
        int *ptr_array[5];
        for (int i = 0; i < 5; i++) {
            ptr_array[i] = (int*)malloc(100 * sizeof(int));
            for (int j = 0; j < 100; j++) {
                ptr_array[i][j] = i * 100 + j;
            }
        }
        
        #pragma omp target data map(ptr_array[2][10:20])
        {
            for (int i = 0; i < 20; i++) {
                ptr_array[2][10 + i] *= 2;
            }
        }
        
        /* Clean up */
        for (int i = 0; i < 5; i++) {
            free(ptr_array[i]);
        }
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i];
        if (i < SIZE) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
