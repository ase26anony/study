/* test-omp-array-sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * It contains various OpenMP constructs with array sections that have
 * complex base expressions, varied lower bounds, and lengths.
 *
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions for complex expressions */
int *get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int *ptr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct Container s;
    struct Container *p = &s;
    
    /* Base: pointer dereference */
    #pragma omp target map((*ptr)[0:N])
    {
        (*ptr)[0] = 1;
    }
    
    /* Base: structure member access */
    #pragma omp target map(s.arr[1:N-1])
    {
        s.arr[1] = 2;
    }
    
    /* Base: pointer to structure member */
    #pragma omp target map(p->arr[2:N-2])
    {
        p->arr[2] = 3;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(get_array()[0:SIZE])
    {
        get_array()[0] = 4;
    }
    
    /* Base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(((int *)buffer)[0:N])
    {
        ((int *)buffer)[0] = 5;
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
    #pragma omp task depend(in: y[0:1]) depend(out: y[1:N-1])
    {
        y[1] = y[0];
    }
    
    /* Function calls for bounds */
    #pragma omp task depend(inout: x[compute_lower():compute_length()])
    {
        x[compute_lower()] = 0;
    }
    
    /* Ternary operator in lower bound */
    int flag = 1;
    #pragma omp task depend(inout: y[flag ? 0 : 10: len])
    {
        y[0] = 1;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[0:N]:1)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        sum += arr[i];
    }
    
    /* Array section in target data */
    #pragma omp target data map(tofrom: arr[0:N])
    {
        #pragma omp target teams distribute parallel for map(to: arr[0:N])
        for (int i = 0; i < N; i++) {
            arr[i] *= 2;
        }
    }
}

/* Complex nested expressions as base */
void test_nested_base(void) {
    struct Container containers[5];
    int *ptr_array[5];
    
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = containers[i].arr;
    }
    
    /* Base: array subscript with pointer dereference */
    #pragma omp target map(ptr_array[2][0:N])
    {
        ptr_array[2][0] = 10;
    }
    
    /* Base: nested structure access */
    #pragma omp target map(containers[1].arr[0:N/2])
    {
        containers[1].arr[0] = 20;
    }
}

/* Multiple array sections in same directive */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    int n = N;
    
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Mixed base types in same clause */
    struct Container s1, s2;
    #pragma omp target map(s1.arr[0:10], s2.arr[10:20])
    {
        s1.arr[0] = s2.arr[10];
    }
}

/* Main driver function */
int main(void) {
    /* Initialize data */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_nested_base();
    test_multiple_sections();
    
    printf("OpenMP array section test completed (compile with -fdump-tree-* for coverage)\n");
    return 0;
}
