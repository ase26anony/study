/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent dead code elimination */
volatile int use_func = 1;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(int *arr, int n) {
    int offset = 10;
    int length = 100;
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(arr[offset:length])
    {
        /* Some computation inside the region */
        for (int i = offset; i < offset + length; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Another array section with variable bounds */
    #pragma omp target data map(arr[n:CHUNK])
    {
        for (int i = n; i < n + CHUNK; i++) {
            arr[i] += 1;
        }
    }
}

/* Function 2: Depend directive with array section */
__attribute__((noinline, used))
void func_depend(int *arr, int start, int len) {
    /* OMP_ARRAY_SECTION with arithmetic expression */
    #pragma omp task depend(inout: arr[start:len]) shared(arr)
    {
        for (int i = start; i < start + len; i++) {
            arr[i] = arr[i] * 3;
        }
    }
    
    /* Nested array section with different base */
    int *ptr = arr + 20;
    #pragma omp task depend(inout: ptr[0:30]) shared(ptr)
    {
        for (int i = 0; i < 30; i++) {
            ptr[i] += 5;
        }
    }
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(int *arr, int len) {
    /* OMP_ARRAY_SECTION with full range */
    #pragma omp target update from(arr[0:len])
    
    /* Multiple array sections in one directive */
    #pragma omp target update to(arr[5:len-10])
    
    /* Array section with complex subscript */
    int mid = len / 2;
    #pragma omp target update from(arr[mid-25:50])
}

/* Function 4: Combined construct with array section */
__attribute__((noinline, used))
void func_combined(void) {
    /* Array section on global array */
    #pragma omp target teams distribute parallel for map(global_arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
    }
    
    /* Array section with stride-like notation (will be OMP_ARRAY_SECTION) */
    int *section_start = &global_arr2[10];
    #pragma omp taskloop shared(global_arr2)
    for (int i = 0; i < 90; i++) {
        section_start[i] = i * 2;
    }
}

/* Function 5: Array section on multi-dimensional array */
__attribute__((noinline, used))
void func_multi_dim(int matrix[][SIZE], int rows, int cols) {
    /* Array section on 2D array row */
    #pragma omp target data map(matrix[2][10:cols-20])
    {
        for (int j = 10; j < cols - 10; j++) {
            matrix[2][j] = j * 4;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = i * 10;
    }
    
    int local_arr[SIZE];
    int local_arr2[SIZE];
    int matrix[5][SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
        local_arr2[i] = SIZE - i;
        for (int j = 0; j < 5; j++) {
            matrix[j][i] = i * j;
        }
    }
    
    /* Call functions based on volatile variable to prevent optimization */
    if (use_func > 0) {
        func_map(local_arr, 25);
        func_depend(local_arr2, 30, 40);
        func_update(local_arr, SIZE);
        func_combined();
        func_multi_dim(matrix, 5, SIZE);
    }
    
    /* Ensure tasks complete */
    #pragma omp taskwait
    
    /* Compute checksum to prevent elimination and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i] + local_arr2[i];
        for (int j = 0; j < 5; j++) {
            checksum += matrix[j][i];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
