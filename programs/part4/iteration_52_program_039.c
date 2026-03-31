/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
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
        #pragma omp target map(tofrom: s.arr[1:5])
        {
            for (int i = 1; i < 6; i++) {
                s.arr[i] = i * 3;
            }
        }
    }
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    #pragma omp target map(tofrom: get_array()[0:N])
    {
        int *arr = get_array();
        for (int i = 0; i < N; i++) {
            arr[i] = i * 4;
        }
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target map(tofrom: ((int *)buffer)[0:N/2])
    {
        int *int_buf = (int *)buffer;
        for (int i = 0; i < N/2; i++) {
            int_buf[i] = i * 5;
        }
    }
}

/* Function using array section in depend clause with variable expressions */
void test_depend_variable_expr(void) {
    int arr[N];
    int i = 10, j = 20;
    
    #pragma omp task depend(inout: arr[i:j])
    {
        for (int k = i; k < i + j; k++) {
            arr[k] = k * 6;
        }
    }
    
    #pragma omp taskwait
}

/* Function using array section with arithmetic expressions for bounds */
void test_depend_arithmetic_expr(void) {
    int arr[N];
    int start = 5, end = 95;
    
    #pragma omp task depend(in: arr[start+1:end-start-1])
    {
        for (int i = start + 1; i < end; i++) {
            arr[i] = i * 7;
        }
    }
    
    #pragma omp taskwait
}

/* Function using array section with function calls for bounds */
void test_depend_func_call_bounds(void) {
    int arr[N];
    
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = lower; i < lower + length; i++) {
            arr[i] = i * 8;
        }
    }
    
    #pragma omp taskwait
}

/* Function using array section with ternary operator for lower bound */
void test_depend_ternary_expr(void) {
    int arr[N];
    int flag = 1;
    int len = 30;
    
    #pragma omp task depend(inout: arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + len; i++) {
            arr[i] = i * 9;
        }
    }
    
    #pragma omp taskwait
}

/* Function using array section in reduction clause (with custom reduction) */
#pragma omp declare reduction(array_plus : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < N; i++) \
            omp_priv[i] = 0)

void test_reduction_array_section(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    #pragma omp parallel for reduction(array_plus: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

/* Function using array section in linear clause */
void test_linear_array_section(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function with multiple array sections in a single directive */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with nested array section usage */
void test_nested_sections(void) {
    int matrix[N][N];
    
    #pragma omp target data map(tofrom: matrix[0:N][0:N])
    {
        #pragma omp target teams distribute parallel for collapse(2) \
            map(tofrom: matrix[0:N][0:N])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                matrix[i][j] = i * N + j;
            }
        }
    }
}

/* Main function that calls all test functions */
int main(void) {
    /* Initialize data */
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    
    test_depend_variable_expr();
    test_depend_arithmetic_expr();
    test_depend_func_call_bounds();
    test_depend_ternary_expr();
    
    test_reduction_array_section();
    test_linear_array_section();
    test_multiple_sections();
    test_nested_sections();
    
    printf("All OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
