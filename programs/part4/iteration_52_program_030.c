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

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section with pointer dereference as base */
void test_pointer_deref(struct Container *p) {
    int *ptr = p->arr;
    
    /* Base: (*ptr) - pointer dereference */
    #pragma omp target map((*ptr)[0:N])
    {
        for (int i = 0; i < N; i++)
            (*ptr)[i] = i;
    }
}

/* Function using array section with structure member access as base */
void test_struct_member(struct Container *s) {
    /* Base: s->arr - structure member access */
    #pragma omp task depend(inout: s->arr[10:30])
    {
        for (int i = 10; i < 40; i++)
            s->arr[i] *= 2;
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    /* Base: get_array() - function call returning pointer */
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++)
            get_array()[i] = i * 2;
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(char *buffer) {
    /* Base: (int *)buffer - cast expression */
    int offset = 16;
    int count = 32;
    
    #pragma omp target map((int *)buffer)[offset:count]
    {
        int *int_buf = (int *)buffer;
        for (int i = offset; i < offset + count; i++)
            int_buf[i] = i - offset;
    }
}

/* Function with complex lower bound and length expressions */
void test_complex_bounds(int *arr, int start, int end, int flag) {
    int len = end - start;
    
    /* Variable expressions for bounds */
    #pragma omp task depend(in: arr[start:len])
    {
        for (int i = start; i < end; i++)
            arr[i] = arr[i] * 3;
    }
    
    /* Arithmetic expressions for bounds */
    #pragma omp target map(arr[start+1:end-start-1])
    {
        for (int i = start + 1; i < end - 1; i++)
            arr[i] = arr[i] + 1;
    }
    
    /* Function calls for bounds */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++)
            arr[i] = 0;
    }
    
    /* Conditional (ternary) expression for lower bound */
    #pragma omp target map(arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + len; i++)
            arr[i] = -arr[i];
    }
}

/* Function with multiple OpenMP constructs using array sections */
void test_multiple_constructs(int *a, int *b, int *c, int n, int *x, int *y, int size) {
    /* target data with map clause */
    #pragma omp target data map(tofrom: a[0:n])
    {
        #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n])
        for (int i = 0; i < n; i++)
            c[i] = a[i] + b[i];
    }
    
    /* task with multiple depend clauses */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < size; i++)
            y[i] = y[i-1] + 1;
    }
    
    /* linear clause with array section (requires valid iteration variable) */
    int idx = 0;
    #pragma omp parallel for reduction(+:a[0:n]) linear(idx:1)
    for (int i = 0; i < n; i++) {
        a[i] += i;
        idx++;
    }
}

/* Custom reduction for array section (simplified example) */
#pragma omp declare reduction(arr_add : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer( \
        for (int i = 0; i < SIZE; i++) \
            omp_priv[i] = 0)

void test_reduction_with_section(int arr[SIZE]) {
    #pragma omp parallel for reduction(arr_add: arr[0:SIZE])
    for (int i = 0; i < 1000; i++) {
        int idx = i % SIZE;
        arr[idx] += 1;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    struct Container container;
    container.ptr_arr = (int*)malloc(N * sizeof(int));
    
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    
    int *x = (int*)malloc(SIZE * sizeof(int));
    int *y = (int*)malloc(SIZE * sizeof(int));
    
    char *buffer = (char*)malloc(1024 * sizeof(char));
    
    int flag = 1;
    int start = 10, end = 90;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        container.arr[i] = i;
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        if (i < SIZE) {
            x[i] = i;
            y[i] = 0;
        }
    }
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_pointer_deref(&container);
    test_struct_member(&container);
    test_function_call_base();
    test_cast_base(buffer);
    test_complex_bounds(arr1, start, end, flag);
    test_multiple_constructs(arr1, arr2, arr3, 50, x, y, SIZE);
    
    int reduction_arr[SIZE] = {0};
    test_reduction_with_section(reduction_arr);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += container.arr[i] + arr1[i] + arr2[i] + arr3[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += x[i] + y[i] + reduction_arr[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(container.ptr_arr);
    free(arr1);
    free(arr2);
    free(arr3);
    free(x);
    free(y);
    free(buffer);
    
    return 0;
}
