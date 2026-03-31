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

/* Structure with array member */
struct with_array {
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

/* Function using array section with pointer dereference base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    #pragma omp target data map(tofrom: (*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(to: (*ptr)[0:N])
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i * 2;
        }
    }
    
    free(ptr);
}

/* Function using array section with structure member access base */
void test_map_struct_member(void) {
    struct with_array s;
    struct with_array *p = &s;
    
    /* Direct member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 0; i < 5; i++) {
            s.arr[i+1] = i;
        }
    }
    
    /* Pointer member access */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 0; i < 8; i++) {
            p->arr[i+2] = i * 3;
        }
    }
}

/* Function using array section with function call base */
void test_map_function_call_base(void) {
    #pragma omp target map(tofrom: get_array()[0:N])
    {
        int *arr = get_array();
        for (int i = 0; i < N; i++) {
            arr[i] = i * 4;
        }
    }
}

/* Function using array section with cast expression base */
void test_map_cast_base(void) {
    char *buffer = malloc(N * sizeof(int));
    if (!buffer) return;
    
    #pragma omp target map(tofrom: ((int *)buffer)[0:N/2])
    {
        int *int_buf = (int *)buffer;
        for (int i = 0; i < N/2; i++) {
            int_buf[i] = i * 5;
        }
    }
    
    free(buffer);
}

/* Function using array sections in depend clauses */
void test_task_depend(void) {
    int x[N], y[N];
    int i, j;
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        for (i = 1; i < N-1; i++) {
            y[i] = x[0] * i;
        }
    }
    
    #pragma omp task depend(inout: x[compute_lower():compute_length()])
    {
        for (i = compute_lower(); i < compute_lower() + compute_length(); i++) {
            x[i] += 1;
        }
    }
}

/* Function using array sections with complex bounds */
void test_complex_bounds(void) {
    int arr[N];
    int start = 10, end = 90;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[start:end-start])
    {
        for (int i = start; i < end; i++) {
            arr[i] = i - start;
        }
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int i = start+1; i < end-1; i++) {
            arr[i] = (i - start) * 2;
        }
    }
    
    /* Ternary expression in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10:SIZE])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + SIZE; i++) {
            arr[i] = i * 3;
        }
    }
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (idx = 0; idx < N; idx++) {
        arr[idx] = idx;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < SIZE; i++) \
            omp_priv[i] = 0)

void test_reduction_array_section(void) {
    int arr[SIZE];
    
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] = 1;
    }
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_map_simple();
    test_map_pointer_deref();
    test_map_struct_member();
    test_map_function_call_base();
    test_map_cast_base();
    
    #pragma omp parallel
    {
        test_task_depend();
    }
    
    test_complex_bounds();
    test_linear_clause();
    test_reduction_array_section();
    
    printf("All tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
