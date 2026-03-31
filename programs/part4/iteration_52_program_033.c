/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Function returning a pointer to use as array section base */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

/* Structure with array member */
struct with_array {
    int arr[N];
    int* ptr;
};

/* Function using array section with complex base expression */
void test_complex_base(struct with_array *s, int **ptr2d, int *buffer) {
    int len = 10;
    
    /* 1. Structure member access as base: s->arr[1:5] */
    #pragma omp target map(s->arr[1:5])
    {
        for (int i = 0; i < 5; i++) s->arr[1+i] = i;
    }
    
    /* 2. Pointer dereference as base: (*ptr2d)[2:8] */
    #pragma omp task depend(inout: (*ptr2d)[2:8])
    {
        for (int i = 0; i < 8; i++) (*ptr2d)[2+i] *= 2;
    }
    
    /* 3. Function call as base: get_array()[0:len] */
    #pragma omp target data map(tofrom: get_array()[0:len])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:len])
        for (int i = 0; i < len; i++) get_array()[i] = i;
    }
    
    /* 4. Cast expression as base: (int *)buffer)[offset:count] */
    int offset = 5, count = 15;
    #pragma omp target map((int *)buffer)[offset:count]
    {
        for (int i = 0; i < count; i++) ((int *)buffer)[offset + i] = i;
    }
}

/* Function using array sections with varied lower bound and length expressions */
void test_varied_bounds(int *arr, int n, int *bounds, int flag) {
    int start = bounds[0];
    int end = bounds[1];
    
    /* 1. Simple variable expressions: arr[i:j] */
    int i = 0, j = n/2;
    #pragma omp target map(arr[i:j])
    {
        for (int k = 0; k < j; k++) arr[i+k] = k;
    }
    
    /* 2. Arithmetic expressions: arr[start+1:end-start-1] */
    #pragma omp task depend(inout: arr[start+1:end-start-1])
    {
        int len = end - start - 1;
        for (int k = 0; k < len; k++) arr[start+1+k] += k;
    }
    
    /* 3. Function calls in bounds: arr[compute_start():compute_len()] */
    #pragma omp target data map(tofrom: arr[abs(start):abs(end)])
    {
        #pragma omp target teams distribute parallel for map(to: arr[abs(start):abs(end)])
        for (int k = 0; k < abs(end); k++) arr[abs(start)+k] *= 2;
    }
    
    /* 4. Conditional (ternary) expression: arr[flag ? 0 : 10: len] */
    int len = 20;
    #pragma omp target map(arr[flag ? 0 : 10: len])
    {
        int base = flag ? 0 : 10;
        for (int k = 0; k < len; k++) arr[base+k] = -k;
    }
}

/* Function with multiple OpenMP constructs using array sections */
void test_multiple_constructs(int *a, int *b, int *c, int *x, int *y, int size) {
    /* target data with multiple array sections */
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* task with multiple depend clauses using array sections */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        y[1] = x[0];
        for (int i = 2; i < size; i++) y[i] = y[i-1] * 2;
    }
    
    /* parallel for with linear clause using array section */
    int idx = 0;
    #pragma omp parallel for reduction(+:a[0:1]) linear(arr:1)
    for (idx = 0; idx < N; idx++) {
        a[0] += idx;
    }
}

/* Custom reduction for array section (requires OpenMP 5.0+) */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction(int arr[SIZE]) {
    /* reduction with array section - may require custom reduction */
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] += i;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int *x = (int *)malloc(N * sizeof(int));
    int *y = (int *)malloc(N * sizeof(int));
    int *buffer = (int *)malloc(N * sizeof(int));
    int **ptr2d = (int **)malloc(sizeof(int *));
    int bounds[2] = {10, 30};
    
    struct with_array s;
    s.ptr = (int *)malloc(N * sizeof(int));
    *ptr2d = (int *)malloc(N * sizeof(int));
    
    int array_for_reduction[SIZE];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        x[i] = i;
        y[i] = 0;
        buffer[i] = i;
        s.arr[i] = i;
        s.ptr[i] = i;
        (*ptr2d)[i] = i;
        if (i < SIZE) array_for_reduction[i] = 0;
    }
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_complex_base(&s, ptr2d, buffer);
    test_varied_bounds(arr1, N, bounds, 1);
    test_multiple_constructs(arr1, arr2, arr3, x, y, N);
    test_reduction(array_for_reduction);
    
    /* Ensure variables are used to prevent optimization */
    printf("Results: %d %d %d\n", arr1[0], arr2[0], arr3[0]);
    printf("Structure: %d %d\n", s.arr[0], s.ptr[0]);
    printf("Pointer2d: %d\n", (*ptr2d)[0]);
    printf("Buffer: %d\n", buffer[0]);
    printf("Reduction array: %d\n", array_for_reduction[0]);
    
    /* Cleanup */
    free(arr1); free(arr2); free(arr3);
    free(x); free(y); free(buffer);
    free(s.ptr); free(*ptr2d); free(ptr2d);
    
    return 0;
}
