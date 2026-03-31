/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
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

/* Function using array section in map clause with simple base */
void test_map_simple(void) {
    int arr[N];
    int n = N;
    
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (int i = 0; i < n; i++)
            arr[i] = i;
    }
}

/* Function using array section with pointer dereference as base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    int n = N/2;
    
    if (ptr) {
        #pragma omp target map(tofrom: (*ptr)[10:n])
        {
            for (int i = 0; i < n; i++)
                (*ptr)[10 + i] = i * 2;
        }
        free(ptr);
    }
}

/* Function using array section with structure member access as base */
void test_map_struct_member(void) {
    struct Container s;
    s.ptr_arr = malloc(N * sizeof(int));
    
    #pragma omp target map(tofrom: s.arr[1:5], s.ptr_arr[0:N])
    {
        for (int i = 0; i < 5; i++)
            s.arr[1 + i] = i * 3;
        for (int i = 0; i < N; i++)
            s.ptr_arr[i] = i * 4;
    }
    
    free(s.ptr_arr);
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    int len = SIZE/2;
    
    #pragma omp target map(to: get_array()[0:len])
    {
        int *arr = get_array();
        for (int i = 0; i < len; i++)
            arr[i] = i * 5;
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char *buffer = malloc(N * sizeof(int));
    int offset = 10;
    int count = 20;
    
    if (buffer) {
        #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
        {
            int *arr = (int *)buffer;
            for (int i = 0; i < count; i++)
                arr[offset + i] = i * 6;
        }
        free(buffer);
    }
}

/* Function using array section in depend clause with variable expressions */
void test_depend_variable_expr(void) {
    int arr[N];
    int start = 5, len = 30;
    
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = 0; i < len; i++)
            arr[start + i] += 1;
    }
    
    #pragma omp taskwait
}

/* Function using array section with arithmetic expressions for bounds */
void test_depend_arithmetic_expr(void) {
    int x[N], y[N];
    int size = N;
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < size; i++)
            y[i] = y[i-1] + 1;
    }
    
    #pragma omp taskwait
}

/* Function using array section with function calls for bounds */
int compute_lower(void) { return 15; }
int compute_length(void) { return 25; }

void test_depend_func_call_bounds(void) {
    int arr[N];
    
    #pragma omp task depend(inout: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = 0; i < length; i++)
            arr[lower + i] *= 2;
    }
    
    #pragma omp taskwait
}

/* Function using array section with ternary operator for lower bound */
void test_depend_ternary_bounds(void) {
    int arr[N];
    int flag = 1;
    int len = 40;
    
    #pragma omp task depend(inout: arr[flag ? 0 : 10: len])
    {
        int start = flag ? 0 : 10;
        for (int i = 0; i < len; i++)
            arr[start + i] = i * 7;
    }
    
    #pragma omp taskwait
}

/* Function using array section in target teams with multiple arrays */
void test_target_teams(void) {
    int a[N], b[N], c[N];
    int n = N;
    
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
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
#pragma omp declare reduction(array_plus : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(void) {
    int arr[N];
    
    #pragma omp parallel for reduction(array_plus: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
}

/* Main driver that calls all test functions */
int main(void) {
    /* Initialize arrays to prevent optimization */
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = 0;
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    
    test_depend_variable_expr();
    test_depend_arithmetic_expr();
    test_depend_func_call_bounds();
    test_depend_ternary_bounds();
    
    test_target_teams();
    test_linear_clause();
    test_reduction_array_section();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) sum += arr[i];
    printf("Result check: %d\n", sum);
    
    return 0;
}
