/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex expressions */
int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }
int* get_array(void) { 
    static int arr[SIZE];
    return arr; 
}

/* Structure with array member */
struct WithArray {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct WithArray s;
    struct WithArray *p = &s;
    
    /* Base: simple array variable */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* Base: pointer dereference (complex expression) */
    #pragma omp target map((*ptr)[10:20])
    {
        ptr[10] = 2;
    }
    
    /* Base: structure member access */
    #pragma omp target map(s.arr[1:5])
    {
        s.arr[1] = 3;
    }
    
    /* Base: pointer to structure member access */
    #pragma omp target map(p->arr[2:8])
    {
        p->arr[2] = 4;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(get_array()[0:10])
    {
        get_array()[0] = 5;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    
    /* Variable expressions for bounds */
    #pragma omp task depend(inout: x[start:len])
    {
        x[start] = x[start] + 1;
    }
    
    /* Arithmetic expressions for bounds */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        y[1] = x[0];
    }
    
    /* Function calls for bounds */
    #pragma omp task depend(in: x[compute_lower():compute_length()])
    {
        x[compute_lower()] = 0;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    int idx = 0;
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        sum += i;
        idx++;
    }
    
    /* Array section in target teams */
    #pragma omp target teams distribute parallel for \
                map(to: arr[0:N/2]) map(from: arr[N/2:N/2])
    for (int i = 0; i < N; i++) {
        if (i < N/2)
            arr[N/2 + i] = arr[i] * 2;
    }
}

/* Function with cast expression as base */
void test_cast_base(void) {
    char buffer[4 * N];
    int offset = 10, count = 20;
    
    /* Base: cast expression */
    #pragma omp target map(((int *)buffer)[offset:count])
    {
        ((int *)buffer)[offset] = 42;
    }
}

/* Function with ternary operator in bounds */
void test_ternary_bounds(void) {
    int arr[N];
    int flag = 1;
    int len = N;
    
    /* Ternary expression in lower bound */
    #pragma omp target map(arr[flag ? 0 : 10: len])
    {
        arr[flag ? 0 : 10] = 99;
    }
    
    /* Complex expression in length */
    #pragma omp target map(arr[0: flag ? N/2 : N/4])
    {
        arr[0] = 100;
    }
}

/* Function with multiple array sections in same clause */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    int n = N;
    
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

/* Main driver function */
int main(void) {
    /* Initialize data */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_cast_base();
    test_ternary_bounds();
    test_multiple_sections();
    
    printf("OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
