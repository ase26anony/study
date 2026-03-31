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
void func_map(int *arr, int start, int length) {
    #pragma omp target data map(arr[start:length])
    {
        /* Simple computation inside region */
        for (int i = start; i < start + length; i++) {
            arr[i] *= 2;
        }
    }
}

/* Function 2: Depend clause with array section */
__attribute__((noinline, used))
void func_depend(int *arr, int n, int size) {
    #pragma omp task depend(inout: arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr[i] += i;
        }
    }
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(int *arr, int len) {
    #pragma omp target update from(arr[0:len])
    /* This directive doesn't need a structured block */
    ;
}

/* Function 4: Complex base expression (array subscript as base) */
__attribute__((noinline, used))
void func_complex_base(int **ptr_arr, int idx, int offset, int count) {
    #pragma omp target data map(ptr_arr[idx][offset:count])
    {
        for (int i = 0; i < count; i++) {
            ptr_arr[idx][offset + i] = idx * 100 + i;
        }
    }
}

/* Function 5: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(int *a, int *b, int start1, int len1, int start2, int len2) {
    #pragma omp target data map(a[start1:len1], b[start2:len2])
    {
        for (int i = 0; i < len1; i++) a[start1 + i] *= 3;
        for (int i = 0; i < len2; i++) b[start2 + i] -= 5;
    }
}

/* Function 6: Array section with arithmetic in bounds */
__attribute__((noinline, used))
void func_arithmetic_bounds(int *arr, int n) {
    int base = n * 2;
    int length = SIZE / 4;
    #pragma omp target data map(arr[base:length])
    {
        for (int i = 0; i < length; i++) {
            arr[base + i] = base + i;
        }
    }
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Dynamic allocation for pointer array test */
    int **ptr_arr = malloc(3 * sizeof(int *));
    for (int i = 0; i < 3; i++) {
        ptr_arr[i] = malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) {
            ptr_arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Local array */
    int local_arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Call functions based on volatile variable to prevent optimization */
    if (use_func > 0) {
        /* Test 1: Simple global array section */
        func_map(global_arr, 10, 100);
        
        /* Test 2: Depend clause with variable bounds */
        #pragma omp parallel
        #pragma omp single
        {
            func_depend(global_arr2, 5, 75);
            #pragma omp taskwait
        }
        
        /* Test 3: Update directive */
        func_update(local_arr, 50);
        
        /* Test 4: Complex base (array of pointers dereference) */
        func_complex_base(ptr_arr, 1, 25, 30);
        
        /* Test 5: Multiple sections */
        func_multiple_sections(global_arr, global_arr2, 0, 40, 30, 60);
        
        /* Test 6: Arithmetic in bounds */
        func_arithmetic_bounds(local_arr, 10);
    }
    
    /* Compute checksum to verify execution */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i];
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += ptr_arr[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        free(ptr_arr[i]);
    }
    free(ptr_arr);
    
    return 0;
}
