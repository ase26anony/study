/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer for
 * OpenMP array section nodes (OMP_ARRAY_SECTION). It uses various forms
 * of array sections in different OpenMP clauses, with complex base
 * expressions and varied bounds, to ensure the uncovered lines in
 * tree-pretty-print.cc are executed during tree dumping passes.
 *
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex base expressions */
int *get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int *ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct Container s;
    struct Container *p = &s;
    
    /* Base: array variable */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* Base: pointer dereference - may need parentheses */
    #pragma omp target map(ptr[0:N])
    {
        ptr[0] = 2;
    }
    
    /* Base: structure member access */
    #pragma omp target map(s.arr[1:N-1])
    {
        s.arr[1] = 3;
    }
    
    /* Base: pointer to structure member access */
    #pragma omp target map(p->arr[2:N-2])
    {
        p->arr[2] = 4;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(get_array()[0:SIZE])
    {
        get_array()[0] = 5;
    }
    
    /* Base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(((int *)buffer)[0:N])
    {
        ((int *)buffer)[0] = 6;
    }
}

/* Function using array section in depend clause with varied bounds */
void test_depend_clause(void) {
    int x[N], y[N];
    int i = 10, j = 20;
    int start = 5, len = 30;
    int flag = 1;
    
    /* Simple constant bounds */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        y[1] = x[0];
    }
    
    /* Variable bounds */
    #pragma omp task depend(inout: x[i:j])
    {
        x[i] *= 2;
    }
    
    /* Arithmetic expression bounds */
    #pragma omp task depend(in: x[start+1:len-start-1])
    {
        x[start+1] = 0;
    }
    
    /* Function call bounds */
    #pragma omp task depend(out: y[compute_lower():compute_length()])
    {
        y[compute_lower()] = 7;
    }
    
    /* Conditional (ternary) lower bound */
    #pragma omp task depend(in: x[flag ? 0 : 10 : len])
    {
        x[flag ? 0 : 10] = 8;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    int idx = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        sum += arr[idx];
        idx++;
    }
    
    /* Target teams with multiple array sections */
    int a[N], b[N], c[N];
    #pragma omp target teams distribute parallel for \
                map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function using array section in target data construct */
void test_target_data(void) {
    int arr[N];
    
    #pragma omp target data map(tofrom: arr[0:N])
    {
        #pragma omp target
        for (int i = 0; i < N; i++) {
            arr[i] = i * 2;
        }
    }
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_target_data();
    
    printf("OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
