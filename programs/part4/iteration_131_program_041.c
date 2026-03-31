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
            arr[i] = arr[i] * 2;
        }
    }
}

/* Function 2: Depend directive with array section using variables */
__attribute__((noinline, used))
void func_depend(int *arr, int n, int size) {
    #pragma omp task depend(inout: arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr[i] = arr[i] + i;
        }
    }
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(int *arr, int len) {
    #pragma omp target update from(arr[0:len])
    /* Dummy update - in real code would have target computation */
}

/* Function 4: Complex array section with ARRAY_REF base */
__attribute__((noinline, used))
void func_complex(int *arr, int idx) {
    int *ptr = arr + 10;
    #pragma omp target data map(ptr[idx*2:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            ptr[idx*2 + i] = idx + i;
        }
    }
}

/* Function 5: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multi_sections(int *a, int *b, int start, int len) {
    #pragma omp target data map(a[start:len], b[start*2:len/2])
    {
        for (int i = 0; i < len; i++) {
            a[start + i] = b[start*2 + i % (len/2)];
        }
    }
}

/* Function 6: Array section with arithmetic in bounds */
__attribute__((noinline, used))
void func_arithmetic_bounds(int *arr, int n) {
    int lower = n * 3;
    int length = SIZE / (n + 1);
    #pragma omp target update to(arr[lower:length])
    /* Dummy - would normally have target computation */
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    int local_arr[SIZE];
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
        dynamic_arr[i] = i * 3;
    }
    
    /* Call functions based on volatile variable to prevent optimization */
    if (use_func > 0) {
        /* Test 1: Simple array section with constant bounds */
        func_map(global_arr, 10, 100);
        
        /* Test 2: Array section with variable bounds */
        func_depend(global_arr2, 5, 75);
        
        /* Test 3: Update with full array section */
        func_update(local_arr, SIZE);
        
        /* Test 4: Complex base (pointer arithmetic result) */
        func_complex(dynamic_arr, 3);
        
        /* Test 5: Multiple sections */
        func_multi_sections(global_arr, global_arr2, 20, 60);
        
        /* Test 6: Arithmetic in bounds */
        func_arithmetic_bounds(local_arr, 4);
    }
    
    /* Compute checksum to prevent elimination and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i];
        if (i < SIZE) checksum += dynamic_arr[i]; /* Avoid out-of-bounds */
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
