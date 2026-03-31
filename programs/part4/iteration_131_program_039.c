/* omp_array_section_test.c
 * Generates OpenMP array sections to trigger OMP_ARRAY_SECTION
 * node pretty-printing in GCC's tree dumps.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays - different base arrays */
int global_arr[SIZE];
int global_arr2[SIZE];
static int static_arr[SIZE];

/* Prevent optimization */
volatile int use_all = 0;

/* Function declarations with attributes to prevent inlining/elimination */
void __attribute__((noinline,used)) func_map(int *arr, int start, int len);
void __attribute__((noinline,used)) func_depend(int *arr, int idx, int size);
void __attribute__((noinline,used)) func_update(int *arr, int offset, int length);
void __attribute__((noinline,used)) func_multi_section(int *a, int *b, int n);
void __attribute__((noinline,used)) func_complex_expr(int *arr, int m, int n);

/* Test 1: Simple map directive with array section */
void __attribute__((noinline,used)) func_map(int *arr, int start, int len) {
    /* Array section with variable bounds */
    #pragma omp target data map(arr[start:len])
    {
        /* Do some work inside the region */
        for (int i = 0; i < len; i++) {
            arr[start + i] += i;
        }
    }
}

/* Test 2: Task with depend clause using array section */
void __attribute__((noinline,used)) func_depend(int *arr, int idx, int size) {
    /* Array section in depend clause */
    #pragma omp task depend(inout: arr[idx:size])
    {
        for (int i = 0; i < size; i++) {
            arr[idx + i] *= 2;
        }
    }
    #pragma omp taskwait
}

/* Test 3: Target update with array section */
void __attribute__((noinline,used)) func_update(int *arr, int offset, int length) {
    /* Array section in target update */
    #pragma omp target update from(arr[offset:length])
    
    /* Also test 'to' direction */
    #pragma omp target update to(arr[offset + 10:length/2])
}

/* Test 4: Multiple array sections in same construct */
void __attribute__((noinline,used)) func_multi_section(int *a, int *b, int n) {
    /* Multiple array sections in map clause */
    #pragma omp target data map(a[0:n/2], b[n/4:3*n/4])
    {
        for (int i = 0; i < n/2; i++) {
            a[i] = b[n/4 + i] + 1;
        }
    }
}

/* Test 5: Complex subscript expressions */
void __attribute__((noinline,used)) func_complex_expr(int *arr, int m, int n) {
    int k = m * 2;
    /* Array section with arithmetic in bounds */
    #pragma omp target data map(arr[k + 1:n - m])
    {
        for (int i = 0; i < n - m; i++) {
            arr[k + 1 + i] = arr[k + 1 + i] * 3;
        }
    }
}

/* Test 6: Array section on pointer parameter (different tree code) */
void __attribute__((noinline,used)) func_pointer_section(int **ptr_arr, int start, int len) {
    /* Dereference and section - creates more complex tree */
    #pragma omp target data map(ptr_arr[0][start:len])
    {
        for (int i = 0; i < len; i++) {
            ptr_arr[0][start + i] += 5;
        }
    }
}

/* Test 7: Nested array sections (if supported) */
void __attribute__((noinline,used)) func_nested_access(int *arr, int idx) {
    /* Create ARRAY_REF as base for OMP_ARRAY_SECTION */
    int *slice = &arr[idx];
    #pragma omp target data map(slice[0:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            slice[i] = i * i;
        }
    }
}

int main(int argc, char **argv) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
        static_arr[i] = i * 2;
    }
    
    /* Pointer array for test 6 */
    int *ptr_array[1];
    ptr_array[0] = malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        ptr_array[0][i] = i * 3;
    }
    
    /* Use volatile to prevent dead code elimination */
    if (use_all) {
        /* Call all functions to ensure they're in compilation unit */
        func_map(global_arr, 10, 100);
        func_depend(global_arr2, 5, 75);
        func_update(static_arr, 0, 50);
        func_multi_section(global_arr, global_arr2, SIZE);
        func_complex_expr(global_arr, 10, 100);
        func_pointer_section(ptr_array, 20, 30);
        func_nested_access(static_arr, 40);
    } else {
        /* Normal execution path */
        printf("Testing OpenMP array sections...\n");
        
        /* Test 1: Simple map */
        func_map(global_arr, 10, 100);
        
        /* Test 2: Task depend */
        #pragma omp parallel
        #pragma omp single
        func_depend(global_arr2, 5, 75);
        
        /* Test 3: Target update */
        func_update(static_arr, 0, 50);
        
        /* Test 4: Multiple sections */
        func_multi_section(global_arr, global_arr2, SIZE);
        
        /* Test 5: Complex expressions */
        func_complex_expr(global_arr, 10, 100);
        
        /* Test 6: Pointer array section */
        func_pointer_section(ptr_array, 20, 30);
        
        /* Test 7: Nested access */
        func_nested_access(static_arr, 40);
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + static_arr[i];
    }
    if (ptr_array[0]) {
        for (int i = 0; i < SIZE; i++) {
            checksum += ptr_array[0][i];
        }
        free(ptr_array[0]);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
