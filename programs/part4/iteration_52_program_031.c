/* test_omp_array_sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer
 * for OpenMP array section nodes (OMP_ARRAY_SECTION).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test_omp_array_sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions for complex base expressions */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int* ptr = arr;
    struct Container s;
    struct Container* p = &s;
    
    /* Base: array variable */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* Base: pointer dereference (needs parentheses) */
    #pragma omp target map((*ptr)[0:N/2])
    {
        ptr[0] = 2;
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

/* Function using array section in depend clause with varied bounds */
void test_depend_clause(void) {
    int x[N], y[N];
    int i = 0, j = 10;
    int start = 5, end = 20;
    int flag = 1;
    
    /* Simple variable bounds */
    #pragma omp task depend(inout: x[i:j])
    {
        x[i] = x[i] + 1;
    }
    
    /* Arithmetic expression bounds */
    #pragma omp task depend(in: x[start+1:end-start-1])
    {
        x[start+1] = 0;
    }
    
    /* Function call bounds */
    #pragma omp task depend(out: y[compute_lower():compute_length()])
    {
        y[compute_lower()] = 1;
    }
    
    /* Conditional (ternary) expression for lower bound */
    #pragma omp task depend(inout: x[flag ? 0 : 10: N/2])
    {
        x[0] = 2;
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
    
    /* Array section in target with multiple maps */
    int a[N], b[N], c[N];
    #pragma omp target teams distribute parallel for \
                map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    int offset = 10;
    int count = 20;
    
    /* Base: cast expression (will need parentheses) */
    #pragma omp target map(((int*)buffer)[offset:count])
    {
        ((int*)buffer)[offset] = 42;
    }
}

/* Function with multiple array sections in same directive */
void test_multiple_sections(void) {
    int arr1[N], arr2[N], arr3[N];
    int n = N/2;
    
    #pragma omp target data map(tofrom: arr1[0:N], arr2[0:n]) map(to: arr3[5:n])
    {
        #pragma omp target map(tofrom: arr1[0:N])
        {
            arr1[0] = arr2[0] + arr3[5];
        }
    }
}

/* Main driver that calls all test functions */
int main(void) {
    /* Allocate and initialize arrays to prevent dead code elimination */
    int* dynamic_arr = (int*)malloc(N * sizeof(int));
    if (!dynamic_arr) return 1;
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i;
    }
    
    /* Test with dynamic array */
    #pragma omp target map(dynamic_arr[0:N/4])
    {
        dynamic_arr[0] = 100;
    }
    
    /* Execute all test functions */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_cast_base();
    test_multiple_sections();
    
    free(dynamic_arr);
    
    printf("Tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
