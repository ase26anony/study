/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer
 * for OpenMP array section nodes (OMP_ARRAY_SECTION).
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
    int *ptr;
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
void test_struct_member(void) {
    struct Container s;
    struct Container *p = &s;
    
    /* Direct member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 0; i < 5; i++)
            s.arr[i+1] = i;
    }
    
    /* Pointer member access */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 0; i < 8; i++)
            p->arr[i+2] = i * 3;
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    #pragma omp target data map(tofrom: get_array()[0:N])
    {
        #pragma omp target map(tofrom: get_array()[0:N])
        {
            int *arr = get_array();
            for (int i = 0; i < N; i++)
                arr[i] = i * 4;
        }
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    
    #pragma omp target map(tofrom: ((int *)buffer)[0:N/2])
    {
        int *int_buf = (int *)buffer;
        for (int i = 0; i < N/2; i++)
            int_buf[i] = i * 5;
    }
}

/* Function using array sections in depend clauses */
void test_depend_clauses(void) {
    int x[10], y[20];
    int start = 2, len = 5;
    
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        for (int i = 0; i < 10; i++) x[i] = i;
        for (int i = 0; i < 20; i++) y[i] = i * 2;
    }
    
    #pragma omp task depend(inout: x[start:len])
    {
        for (int i = start; i < start + len; i++)
            x[i] += 10;
    }
    
    #pragma omp taskwait
}

/* Function using array sections with complex lower bound/length expressions */
void test_complex_bounds(void) {
    int arr[N];
    int i = 10, j = 30;
    int start = 5, end = 45;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = i; k < i + j; k++)
            arr[k] = k;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(to: arr[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++)
            arr[k] = k * 2;
    }
    
    /* Function call expressions */
    #pragma omp target map(from: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++)
            arr[k] = k * 3;
    }
    
    /* Conditional (ternary) expression */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++)
            arr[k] = k * 4;
    }
}

/* Function using array section in reduction-like pattern */
void test_reduction_pattern(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) arr[i] = 1;
    
    /* Manual reduction using array section */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    /* Array section in linear clause */
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function with multiple array sections in single directive */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
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
    test_depend_clauses();
    test_complex_bounds();
    test_reduction_pattern();
    test_multiple_sections();
    
    printf("All OpenMP array section tests completed (compile-time coverage).\n");
    return 0;
}
