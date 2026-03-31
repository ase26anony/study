/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer
 * for OpenMP array sections (OMP_ARRAY_SECTION tree nodes).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test.c -o test.o
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
void test_map_struct_member(void) {
    struct Container s;
    struct Container *p = &s;
    
    #pragma omp target data map(tofrom: s.arr[1:5], p->arr[2:8])
    {
        #pragma omp target map(tofrom: s.arr[1:5])
        for (int i = 1; i < 6; i++)
            s.arr[i] = i * 3;
    }
}

/* Function using array section with function call as base */
void test_map_func_call_base(void) {
    #pragma omp target map(tofrom: get_array()[0:N])
    {
        int *arr = get_array();
        for (int i = 0; i < N; i++)
            arr[i] = i * 4;
    }
}

/* Function using array section with cast expression as base */
void test_map_cast_base(void) {
    char *buffer = malloc(N * sizeof(int));
    if (!buffer) return;
    
    #pragma omp target map(tofrom: ((int *)buffer)[0:N/2])
    {
        int *arr = (int *)buffer;
        for (int i = 0; i < N/2; i++)
            arr[i] = i * 5;
    }
    
    free(buffer);
}

/* Function using array section in depend clause with variable bounds */
void test_depend_variable_bounds(void) {
    int arr[N];
    int start = 10, len = 30;
    
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = start; i < start + len; i++)
            arr[i] = i * 6;
    }
    
    #pragma omp task depend(in: arr[0:1]) depend(out: arr[1:N-1])
    {
        arr[0] = 0;
        for (int i = 1; i < N; i++)
            arr[i] = arr[i-1] + 1;
    }
    
    #pragma omp taskwait
}

/* Function using array section with arithmetic expressions for bounds */
void test_arithmetic_bounds(void) {
    int arr[N];
    int start = 5, end = 45;
    
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int i = start+1; i < end; i++)
            arr[i] = i * 7;
    }
}

/* Function using array section with function calls for bounds */
void test_func_call_bounds(void) {
    int arr[N];
    
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = lower; i < lower + length; i++)
            arr[i] = i * 8;
    }
}

/* Function using array section with conditional expression for lower bound */
void test_conditional_bounds(void) {
    int arr[N];
    int flag = 1;
    int len = 25;
    
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + len; i++)
            arr[i] = i * 9;
    }
}

/* Function using array section in reduction-like pattern */
void test_reduction_pattern(void) {
    int arr[N];
    int sum = 0;
    
    /* Custom reduction-like pattern using array section */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        #pragma omp atomic
        sum += arr[i];
    }
    
    /* Linear clause with array section (if supported) */
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function with multiple array sections in complex directive */
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
    test_map_struct_member();
    test_map_func_call_base();
    test_map_cast_base();
    test_depend_variable_bounds();
    test_arithmetic_bounds();
    test_func_call_bounds();
    test_conditional_bounds();
    test_reduction_pattern();
    test_multiple_sections();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
