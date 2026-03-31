/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex base expressions */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(struct Container *p, int n) {
    int local_arr[N];
    
    /* Base: structure member access */
    #pragma omp target map(p->arr[0:n])
    {
        for (int i = 0; i < n; i++)
            p->arr[i] = i;
    }
    
    /* Base: pointer dereference */
    int (*ptr_to_arr)[N] = &local_arr;
    #pragma omp target map((*ptr_to_arr)[5:10])
    {
        for (int i = 0; i < 10; i++)
            (*ptr_to_arr)[5 + i] = i * 2;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(int *arr, int start, int len) {
    /* Base: simple array variable */
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = 0; i < len; i++)
            arr[start + i] *= 2;
    }
    
    /* Base: function call returning pointer */
    #pragma omp task depend(in: get_array()[0:10])
    {
        /* Do something with the array */
    }
}

/* Function using array section with complex bounds */
void test_complex_bounds(int *arr, int flag, int *bounds) {
    int lower = bounds[0];
    int upper = bounds[1];
    
    /* Complex lower bound: ternary operator */
    #pragma omp target map(arr[flag ? 0 : 10 : upper - lower])
    {
        for (int i = 0; i < (upper - lower); i++) {
            int idx = (flag ? 0 : 10) + i;
            if (idx < N) arr[idx] = i;
        }
    }
    
    /* Complex length: arithmetic expression */
    int offset = 5;
    #pragma omp target map(arr[offset: N - offset - 1])
    {
        for (int i = 0; i < N - offset - 1; i++)
            arr[offset + i] += 3;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(int *arr, int size) {
    int sum = 0;
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[0:size]:1)
    for (int i = 0; i < size; i++) {
        arr[i] = i;
        sum += arr[i];
    }
    
    /* Base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(((int *)buffer)[0:size])
    {
        for (int i = 0; i < size; i++)
            ((int *)buffer)[i] = i * 3;
    }
}

/* Function with multiple array sections in a single directive */
void test_multiple_sections(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Main driver that ensures all code is reachable */
int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    
    struct Container s;
    s.ptr_arr = (int *)malloc(N * sizeof(int));
    
    int bounds[2] = {10, 30};
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        s.arr[i] = i * 3;
        s.ptr_arr[i] = i * 4;
    }
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_map_clause(&s, 20);
    test_depend_clause(arr1, 5, 15);
    test_complex_bounds(arr2, 1, bounds);
    test_reduction_context(arr3, 25);
    test_multiple_sections(arr1, arr2, arr3, 40);
    
    /* Ensure results are used to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + s.arr[i] + s.ptr_arr[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(s.ptr_arr);
    
    return 0;
}
