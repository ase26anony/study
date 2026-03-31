/* test-omp-array-sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc.
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
    
    #pragma omp target data map(tofrom: s.arr[0:N], p->arr[10:N-10])
    {
        #pragma omp target teams distribute parallel for \
            map(to: s.arr[0:N]) map(from: p->arr[10:N-10])
        for (int i = 0; i < N; i++) {
            s.arr[i] = i;
            if (i >= 10 && i < N-10)
                p->arr[i] = i * 3;
        }
    }
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    #pragma omp target map(tofrom: get_array()[0:SIZE])
    {
        int *arr = get_array();
        for (int i = 0; i < SIZE; i++)
            arr[i] = i * 4;
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target map(tofrom: ((int *)buffer)[0:N])
    {
        int *int_buf = (int *)buffer;
        for (int i = 0; i < N; i++)
            int_buf[i] = i * 5;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = N-20;
    
    #pragma omp task depend(inout: x[0:N]) shared(x)
    {
        for (int i = 0; i < N; i++) x[i] = i;
    }
    
    #pragma omp task depend(in: x[start:len]) depend(out: y[1:N-1]) shared(x, y)
    {
        for (int i = 1; i < N-1; i++) y[i] = x[i-1] + x[i] + x[i+1];
    }
    
    #pragma omp taskwait
}

/* Function using array section with complex lower bound and length */
void test_complex_bounds(void) {
    int arr[N];
    int i = 5, j = 30;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = i; k < i + j; k++)
            arr[k] = k;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[i+1:j-i-1])
    {
        for (int k = i+1; k < j; k++)
            arr[k] = arr[k] * 2;
    }
    
    /* Function calls in bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++)
            arr[k] = arr[k] + 1;
    }
    
    /* Conditional (ternary) expression in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10 : N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++)
            arr[k] = arr[k] - 1;
    }
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_add : int [N] : \
    for (int i = 0; i < N; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_clause(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma omp parallel for reduction(arr_add: arr[0:N])
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
    
    printf("All OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
