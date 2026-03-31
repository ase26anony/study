/* test-omp-array-sections.c
 * 
 * This program is designed to exercise GCC's pretty-printer for OpenMP array sections.
 * It contains various OpenMP directives with array section expressions to trigger
 * the OMP_ARRAY_SECTION case in tree-pretty-print.cc (lines 2736-2748).
 *
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
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

/* Function using array section in map clause with simple base */
void test_map_simple(void) {
    int arr[N];
    
    #pragma omp target map(tofrom: arr[0:N])
    {
        for (int i = 0; i < N; i++) {
            arr[i] = i;
        }
    }
}

/* Function using array section with pointer dereference as base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N])
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i * 2;
        }
    }
    
    free(ptr);
}

/* Function using array section with structure member access as base */
void test_map_struct_member(void) {
    struct Container s;
    struct Container *p = &s;
    
    #pragma omp target data map(tofrom: s.arr[1:5], p->arr[2:8])
    {
        #pragma omp target teams distribute parallel for \
            map(to: s.arr[1:5]) map(from: p->arr[2:8])
        for (int i = 0; i < 5; i++) {
            s.arr[i+1] = i;
            p->arr[i+2] = i * 2;
        }
    }
}

/* Function using array section with function call as base */
void test_map_function_call_base(void) {
    #pragma omp target data map(tofrom: get_array()[0:N])
    {
        #pragma omp target map(tofrom: get_array()[compute_lower():compute_length()])
        {
            for (int i = 0; i < compute_length(); i++) {
                get_array()[i + compute_lower()] = i * 3;
            }
        }
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target data map(tofrom: ((int *)buffer)[0:N/2])
    {
        #pragma omp target map(tofrom: ((int *)buffer)[10:20])
        {
            for (int i = 0; i < 20; i++) {
                ((int *)buffer)[i + 10] = i * 4;
            }
        }
    }
}

/* Function using array section in depend clause */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        for (int i = 1; i < N-1; i++) {
            y[i] = x[0] * i;
        }
    }
    
    #pragma omp task depend(inout: x[start:len])
    {
        for (int i = start; i < start + len; i++) {
            x[i] += y[i];
        }
    }
    
    #pragma omp taskwait
}

/* Function using array section with complex lower bound and length expressions */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 20;
    int start = 10, end = 90;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = i; k < i + j; k++) {
            arr[k] = k;
        }
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++) {
            arr[k] = arr[k] * 2;
        }
    }
    
    /* Conditional (ternary) expression */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++) {
            arr[k] = arr[k] + 1;
        }
    }
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;  // Linear clause increments the array section
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_add : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < N; i++) \
            omp_priv[i] = 0)

void test_reduction_clause(void) {
    int arr[N];
    
    #pragma omp parallel for reduction(arr_add: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
}

/* Main driver function */
int main(void) {
    /* Initialize and call all test functions */
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_function_call_base();
    test_map_cast_base();
    test_depend_clause();
    test_complex_bounds();
    test_linear_clause();
    test_reduction_clause();
    
    printf("All OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printer output)\n");
    return 0;
}
