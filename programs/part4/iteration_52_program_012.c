/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * It uses various OpenMP array section expressions with complex base
 * expressions to exercise the priority-checking parentheses logic.
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
        for (int i = 0; i < N; i++)
            arr[i] = i;
    }
}

/* Function using array section with pointer dereference as base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N])
        for (int i = 0; i < N; i++)
            (*ptr)[i] = i * 2;
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
            map(tofrom: s.arr[1:5]) map(tofrom: p->arr[2:8])
        for (int i = 0; i < 8; i++) {
            if (i < 5) s.arr[i+1] = i;
            if (i < 6) p->arr[i+2] = i * 2;
        }
    }
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for \
            map(tofrom: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++)
            get_array()[i] = i * 3;
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target data map(tofrom: ((int *)buffer)[0:N])
    {
        #pragma omp target teams distribute parallel for \
            map(tofrom: ((int *)buffer)[0:N])
        for (int i = 0; i < N; i++)
            ((int *)buffer)[i] = i * 4;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 20;
    
    #pragma omp task depend(inout: x[0:1]) \
                     depend(out: y[start:len]) shared(x, y)
    {
        x[0] = 1;
        for (int i = start; i < start + len; i++)
            y[i] = x[0] * i;
    }
    
    #pragma omp task depend(in: y[1:N-1]) \
                     depend(inout: x[compute_lower():compute_length()]) shared(x, y)
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++)
            x[i] += y[i+1];
    }
    
    #pragma omp taskwait
}

/* Function using array section with complex lower bound and length */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 10;
    int start = 2, end = 40;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = i; k < i + j; k++)
            arr[k] = k;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++)
            arr[k] = arr[k] * 2;
    }
    
    /* Ternary conditional expression */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: N/2])
    {
        for (int k = (flag ? 0 : 10); k < (flag ? 0 : 10) + N/2; k++)
            arr[k] = arr[k] + 1;
    }
    
    /* Function calls as bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        for (int k = compute_lower(); k < compute_lower() + compute_length(); k++)
            arr[k] = arr[k] - 1;
    }
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1) linear(idx:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_plus : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_clause(void) {
    int arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++)
        arr[i] = i;
    
    /* Array section in reduction clause with custom reduction */
    #pragma omp parallel for reduction(array_plus: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    test_depend_clause();
    test_complex_bounds();
    test_linear_clause();
    test_reduction_clause();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
