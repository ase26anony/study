/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748) when compiled with
 * tree dump flags (e.g., -fdump-tree-original).
 *
 * It contains various OpenMP constructs with array sections, using complex
 * base expressions and varied lower-bound/length expressions to exercise
 * all paths in the uncovered code block.
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex expressions */
int *get_array(void) {
    static int arr[N];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int *ptr;
};

/* Function using array section with pointer dereference as base */
void test_pointer_deref(struct Container *cont, int n) {
    int *ptr = cont->arr;
    
    /* Base: (*ptr) where ptr is int* */
    #pragma omp target map((*ptr)[0:n])
    {
        for (int i = 0; i < n; i++)
            (*ptr)[i] = i;
    }
}

/* Function using array section with structure member access */
void test_struct_member(struct Container s, int start, int len) {
    /* Base: s.arr (structure member access) */
    #pragma omp task depend(inout: s.arr[start:len])
    {
        for (int i = 0; i < len; i++)
            s.arr[start + i] *= 2;
    }
}

/* Function using array section with pointer-to-member access */
void test_pointer_member(struct Container *p, int offset, int count) {
    /* Base: p->arr (pointer-to-member access) */
    #pragma omp target data map(tofrom: p->arr[offset:count])
    {
        #pragma omp target teams distribute parallel for map(to: p->arr[offset:count])
        for (int i = 0; i < count; i++)
            p->arr[offset + i] += i;
    }
}

/* Function using array section with function call as base */
void test_function_call_base(int n) {
    /* Base: get_array() (function call returning pointer) */
    #pragma omp target map(get_array()[0:n])
    {
        for (int i = 0; i < n; i++)
            get_array()[i] = i * 2;
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(char *buffer, int offset, int count) {
    /* Base: (int *)buffer (cast expression) */
    #pragma omp task depend(in: ((int *)buffer)[offset:count])
    {
        int *arr = (int *)buffer;
        for (int i = 0; i < count; i++)
            arr[offset + i] = 0;
    }
}

/* Function with varied lower-bound and length expressions */
void test_varied_expressions(int *arr, int size, int flag) {
    int i = 10, j = 20;
    int start = 5, end = 45;
    
    /* 1. Integer constants */
    #pragma omp target map(arr[0:100])
    {
        for (int k = 0; k < 100; k++) arr[k] = k;
    }
    
    /* 2. Variable expressions */
    #pragma omp task depend(inout: arr[i:j])
    {
        for (int k = 0; k < j; k++) arr[i + k] += k;
    }
    
    /* 3. Arithmetic expressions */
    #pragma omp target data map(tofrom: arr[start+1:end-start-1])
    {
        int len = end - start - 1;
        #pragma omp target teams distribute parallel for map(to: arr[start+1:len])
        for (int k = 0; k < len; k++)
            arr[start + 1 + k] *= 3;
    }
    
    /* 4. Function calls in bounds */
    #pragma omp task depend(in: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = 0; k < length; k++)
            arr[lower + k] = -1;
    }
    
    /* 5. Conditional (ternary) expression in lower bound */
    #pragma omp target map(arr[flag ? 0 : 10: size/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = 0; k < size/2; k++)
            arr[lower + k] = k * 4;
    }
}

/* Function with linear clause using array section */
void test_linear_clause(int *arr, int n) {
    /* Linear clause with array section */
    #pragma omp parallel for linear(arr[0:1]:1)
    for (int idx = 0; idx < n; idx++) {
        arr[idx] = idx;
    }
}

/* Function with reduction clause using array section (requires custom reduction) */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < SIZE; i++) \
            omp_priv[i] = 0)

void test_reduction_clause(int arr[SIZE], int n) {
    int i;
    
    /* Custom reduction with array section */
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (i = 0; i < n; i++) {
        arr[i % SIZE] += i;
    }
}

/* Function with multiple array sections in same directive */
void test_multiple_sections(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with complex nested base expression */
void test_nested_base(int ***ppp, int m, int n) {
    /* Base: (**ppp) - double pointer dereference */
    #pragma omp target map((**ppp)[0:m*n])
    {
        for (int i = 0; i < m*n; i++)
            (**ppp)[i] = i;
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int arr4[SIZE];
    
    char *buffer = (char *)malloc(N * sizeof(int));
    struct Container s, *p;
    int ***ppp;
    
    /* Initialize data structures */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    
    for (int i = 0; i < SIZE; i++) {
        arr4[i] = 0;
    }
    
    s.ptr = arr1;
    for (int i = 0; i < N; i++) s.arr[i] = i;
    
    p = &s;
    
    /* Triple pointer setup */
    ppp = (int ***)malloc(sizeof(int **));
    *ppp = (int **)malloc(sizeof(int *));
    **ppp = arr1;
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_pointer_deref(&s, 20);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            test_struct_member(s, 5, 10);
            test_pointer_member(p, 2, 8);
            test_function_call_base(15);
            test_cast_base(buffer, 3, 7);
        }
    }
    
    test_varied_expressions(arr1, N, 1);
    test_linear_clause(arr2, N);
    test_reduction_clause(arr4, N);
    test_multiple_sections(arr1, arr2, arr3, N);
    test_nested_base(ppp, 5, 5);
    
    /* Ensure arrays are used to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += arr4[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    free(**ppp);
    free(*ppp);
    free(ppp);
    
    return 0;
}
