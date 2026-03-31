/* test-omp-array-sections.c
 * 
 * This program is designed to exercise GCC's pretty-printer for OpenMP array sections.
 * It contains various OpenMP constructs with array sections that should trigger
 * the OMP_ARRAY_SECTION case in tree-pretty-print.cc lines 2736-2748.
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex expressions */
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

/* Function using array section in map clause with simple base */
void test_map_simple(void) {
    int arr[N];
    
    /* Simple array section in map clause */
    #pragma omp target map(tofrom: arr[0:N])
    {
        for (int i = 0; i < N; i++) {
            arr[i] = i;
        }
    }
}

/* Function using array section with pointer dereference as base */
void test_map_complex_base(void) {
    int (*ptr)[N];
    int buffer[N];
    ptr = &buffer;
    
    /* Array section with dereferenced pointer as base */
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(to: (*ptr)[0:N])
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i * 2;
        }
    }
}

/* Function using array section with structure member access */
void test_struct_member(void) {
    struct Container s;
    struct Container *p = &s;
    
    /* Array section with direct member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 1; i < 6; i++) {
            s.arr[i] = i * 3;
        }
    }
    
    /* Array section with pointer member access */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 2; i < 10; i++) {
            p->arr[i] = i * 4;
        }
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    /* Array section with function call returning pointer as base */
    #pragma omp target map(tofrom: get_array()[0:10])
    {
        int *arr = get_array();
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 5;
        }
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    /* Array section with cast expression as base */
    #pragma omp target map(tofrom: ((int *)buffer)[10:20])
    {
        int *arr = (int *)buffer;
        for (int i = 10; i < 30; i++) {
            arr[i] = i * 6;
        }
    }
}

/* Function using array sections in depend clauses */
void test_depend_clauses(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    
    /* Array sections in depend clauses */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < N; i++) {
            y[i] = y[i-1] + 1;
        }
    }
    
    #pragma omp task depend(inout: x[start:len])
    {
        for (int i = start; i < start + len; i++) {
            x[i] = x[i] * 3;
        }
    }
    
    #pragma omp taskwait
}

/* Function using array sections with complex bounds */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 20;
    int start = 10, end = 40;
    int flag = 1;
    
    /* Array section with variable bounds */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = i; k < i + j; k++) {
            arr[k] = k * 7;
        }
    }
    
    /* Array section with arithmetic expression bounds */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int k = start + 1; k < end; k++) {
            arr[k] = arr[k] * 2;
        }
    }
    
    /* Array section with function call bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++) {
            arr[k] = k * 8;
        }
    }
    
    /* Array section with conditional (ternary) expression bounds */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++) {
            arr[k] = k * 9;
        }
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
    }
    
    /* Using array section in parallel region */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    /* Using linear clause with array section (though not standard reduction) */
    #pragma omp parallel for
    for (int idx = 0; idx < N; idx++) {
        /* This creates array sections in the internal representation */
        int val = arr[idx];
        arr[idx] = val * 2;
    }
}

/* Function using multiple array sections in complex OpenMP construct */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    
    /* Multiple array sections in map clauses */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N], b[0:N]) map(from: c[0:N])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_map_simple();
    test_map_complex_base();
    test_struct_member();
    test_function_call_base();
    test_cast_base();
    test_depend_clauses();
    test_complex_bounds();
    test_reduction_context();
    test_multiple_sections();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
