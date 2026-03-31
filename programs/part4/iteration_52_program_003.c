/* test-omp-array-sections.c
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748) when compiled with
 * tree dump flags (e.g., -fdump-tree-original).
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
    int* ptr;
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
void test_map_complex_base1(void) {
    int data[N];
    int* ptr = data;
    /* Complex base: (*ptr) requires parentheses due to operator priority */
    #pragma omp target data map(tofrom: (*ptr)[0:N/2])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:N/2])
        for (int i = 0; i < N/2; i++)
            ptr[i] = i * 2;
    }
}

/* Function using structure member access as base */
void test_map_complex_base2(void) {
    struct Container s;
    s.ptr = s.arr;
    /* Base: s.arr (member access) */
    #pragma omp target map(tofrom: s.arr[1:N-1])
    {
        for (int i = 1; i < N-1; i++)
            s.arr[i] = i * 3;
    }
}

/* Function using pointer-to-member access as base */
void test_map_complex_base3(void) {
    struct Container s;
    struct Container* p = &s;
    /* Base: p->arr (pointer-to-member) */
    #pragma omp target teams distribute parallel for map(to: p->arr[2:N-2])
    for (int i = 2; i < N-2; i++)
        p->arr[i] = i * 4;
}

/* Function using function call as base */
void test_map_complex_base4(void) {
    /* Base: get_array() (function call returning pointer) */
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for map(tofrom: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++)
            get_array()[i] = i * 5;
    }
}

/* Function using cast expression as base */
void test_map_complex_base5(void) {
    char buffer[N * sizeof(int)];
    /* Base: (int *)buffer (cast expression) */
    #pragma omp target map(tofrom: ((int *)buffer)[10:N-10])
    {
        for (int i = 10; i < N-10; i++)
            ((int *)buffer)[i] = i * 6;
    }
}

/* Function with varied lower bound and length expressions */
void test_varied_bounds(void) {
    int arr[N];
    int i = 10, j = 30;
    int start = 5, end = 45;
    int flag = 1;
    
    /* Variable expressions */
    #pragma omp task depend(inout: arr[i:j])
    {
        for (int k = i; k < i+j; k++)
            arr[k] = k;
    }
    
    /* Arithmetic expressions */
    #pragma omp task depend(in: arr[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++)
            arr[k] = k * 2;
    }
    
    /* Function call expressions */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        for (int k = lower; k < lower + compute_length(); k++)
            arr[k] = k * 3;
    }
    
    /* Conditional (ternary) expression */
    #pragma omp task depend(inout: arr[flag ? 0 : 10 : N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + N/2; k++)
            arr[k] = k * 4;
    }
}

/* Function using array section in reduction clause (with custom reduction) */
#pragma omp declare reduction(array_plus : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    /* Array section in reduction clause */
    #pragma omp parallel for reduction(array_plus: arr[0:N])
    for (int i = 0; i < N; i++)
        arr[i] += 1;
}

/* Function using array section in linear clause */
void test_linear_array_section(void) {
    int arr[N];
    int idx = 0;
    
    /* Array section in linear clause */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function with multiple array sections in same directive */
void test_multiple_sections(void) {
    int a[N], b[N], c[N];
    int n = N;
    
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with nested array sections */
void test_nested_complex(void) {
    struct Container containers[5];
    int len = 10;
    
    /* Complex base with array indexing */
    #pragma omp target map(tofrom: containers[2].arr[0:len])
    {
        for (int i = 0; i < len; i++)
            containers[2].arr[i] = i * 7;
    }
    
    /* Even more complex: pointer dereference of array element */
    #pragma omp task depend(inout: (*containers[3].ptr)[5:15])
    {
        containers[3].ptr = containers[3].arr;
        for (int i = 5; i < 20; i++)
            containers[3].ptr[i] = i * 8;
    }
}

/* Main driver that calls all test functions */
int main(void) {
    /* Allocate and initialize data to prevent dead code elimination */
    test_map_simple();
    test_map_complex_base1();
    test_map_complex_base2();
    test_map_complex_base3();
    test_map_complex_base4();
    test_map_complex_base5();
    test_varied_bounds();
    test_reduction_array_section();
    test_linear_array_section();
    test_multiple_sections();
    test_nested_complex();
    
    printf("All OpenMP array section tests completed (compile with -fopenmp -fdump-tree-original)\n");
    return 0;
}
