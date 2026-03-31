/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
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

/* Structure with array member */
struct with_array {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with simple base */
void test_map_simple(void) {
    int arr[N];
    
    #pragma omp target map(arr[0:N])
    {
        for (int i = 0; i < N; i++)
            arr[i] = i;
    }
}

/* Function using array section with pointer dereference as base */
void test_map_pointer_deref(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    #pragma omp target data map((*ptr)[0:N])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N])
        for (int i = 0; i < N; i++)
            (*ptr)[i] = i * 2;
    }
    
    free(ptr);
}

/* Function using structure member access as base */
void test_struct_member(void) {
    struct with_array s;
    struct with_array *p = &s;
    
    /* Direct member access */
    #pragma omp target map(s.arr[1:5])
    {
        for (int i = 0; i < 5; i++)
            s.arr[i+1] = i;
    }
    
    /* Pointer member access */
    #pragma omp target map(p->arr[2:8])
    {
        for (int i = 0; i < 8; i++)
            p->arr[i+2] = i * 3;
    }
}

/* Function using function call as base */
void test_function_call_base(void) {
    #pragma omp target data map(get_array()[0:N])
    {
        #pragma omp target map(get_array()[compute_lower():compute_length()])
        {
            int *arr = get_array();
            for (int i = 0; i < compute_length(); i++)
                arr[compute_lower() + i] = i * 4;
        }
    }
}

/* Function using cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target map(((int *)buffer)[0:N/2])
    {
        int *int_buf = (int *)buffer;
        for (int i = 0; i < N/2; i++)
            int_buf[i] = i * 5;
    }
}

/* Function with complex lower bound and length expressions */
void test_complex_bounds(void) {
    int arr[N];
    int start = 10, end = 90;
    int i = 5, j = 30;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(arr[i:j])
    {
        for (int k = 0; k < j; k++)
            arr[i + k] = k;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(arr[start+1:end-start-1])
    {
        for (int k = 0; k < end-start-1; k++)
            arr[start+1 + k] = k * 2;
    }
    
    /* Ternary conditional in lower bound */
    #pragma omp target map(arr[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = 0; k < N/2; k++)
            arr[lower + k] = k * 3;
    }
}

/* Function using array sections in depend clauses */
void test_depend_clauses(void) {
    int x[N], y[N];
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < N; i++)
            y[i] = y[i-1] + 1;
    }
    
    #pragma omp task depend(inout: x[10:20])
    {
        for (int i = 10; i < 30; i++)
            x[i] += 5;
    }
    
    #pragma omp taskwait
}

/* Function using array section in linear clause */
void test_linear_clause(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;  /* Linear clause increments idx */
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < SIZE; i++) \
            omp_priv[i] = 0)

void test_reduction_clause(void) {
    int arr[SIZE];
    
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
}

/* Function combining multiple array sections in one directive */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    
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
    test_map_pointer_deref();
    test_struct_member();
    test_function_call_base();
    test_cast_base();
    test_complex_bounds();
    test_depend_clauses();
    test_linear_clause();
    test_reduction_clause();
    test_multiple_sections();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
