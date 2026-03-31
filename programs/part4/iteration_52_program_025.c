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

/* Helper functions to create complex expressions */
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
void test_pointer_deref(int (*ptr)[N]) {
    /* Base: (*ptr) - requires parentheses due to operator priority */
    #pragma omp target map((*ptr)[0:N])
    {
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i;
        }
    }
}

/* Function using array section with structure member access */
void test_struct_member(struct Container* s) {
    /* Base: s->arr - member access */
    #pragma omp task depend(inout: s->arr[10:30])
    {
        for (int i = 10; i < 40; i++) {
            s->arr[i] *= 2;
        }
    }
    
    /* Base: s.arr - direct member access */
    struct Container local_s = *s;
    #pragma omp target map(local_s.arr[5:25])
    {
        for (int i = 5; i < 30; i++) {
            local_s.arr[i] += 1;
        }
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    /* Base: get_array() - function call returning pointer */
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++) {
            get_array()[i] = i * 2;
        }
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(char* buffer) {
    /* Base: (int *)buffer - cast expression */
    int offset = 4;
    int count = 10;
    #pragma omp target map((int *)buffer)[offset:count]
    {
        int* int_buf = (int*)buffer;
        for (int i = offset; i < offset + count; i++) {
            int_buf[i] = i - offset;
        }
    }
}

/* Function with varied lower bound and length expressions */
void test_varied_bounds(int* arr, int n) {
    int i = 0, j = n/2;
    int start = 10, end = n;
    
    /* Integer constants */
    #pragma omp target map(arr[0:100])
    {
        for (int k = 0; k < 100; k++) arr[k] = k;
    }
    
    /* Variable expressions */
    #pragma omp task depend(in: arr[i:j])
    {
        for (int k = i; k < i + j; k++) arr[k] *= 3;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(arr[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++) arr[k] += 5;
    }
    
    /* Function calls in bounds */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++) arr[k] = 0;
    }
    
    /* Conditional (ternary) expression in lower bound */
    int flag = 1;
    int len = 20;
    #pragma omp target map(arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + len; k++) arr[k] = -1;
    }
}

/* Function with multiple OpenMP constructs */
void test_multiple_constructs(int* a, int* b, int* c, int n) {
    /* target data with map */
    #pragma omp target data map(tofrom: a[0:n])
    {
        /* target teams distribute parallel for with multiple maps */
        #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n])
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* task with multiple depend clauses */
    #pragma omp task depend(in: a[0:1]) depend(out: b[1:n-1])
    {
        b[1] = a[0];
        for (int i = 2; i < n; i++) b[i] = b[i-1] * 2;
    }
    
    /* linear clause with array section (GCC extension) */
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < n; i++) {
        idx = i;
        a[idx] = i;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_plus : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(int arr[SIZE]) {
    /* Note: Array section reduction may require custom reduction */
    #pragma omp parallel for reduction(array_plus: arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] += i;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* arr3 = (int*)malloc(N * sizeof(int));
    char* buffer = (char*)malloc(1024 * sizeof(char));
    int multi_dim_arr[N];
    int (*ptr_to_arr)[N] = &multi_dim_arr;
    
    struct Container s;
    s.arr[0] = 0;
    s.ptr_arr = arr2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        multi_dim_arr[i] = i;
    }
    
    /* Call test functions to generate various OMP_ARRAY_SECTION nodes */
    test_pointer_deref(ptr_to_arr);
    test_struct_member(&s);
    test_function_call_base();
    test_cast_base(buffer);
    test_varied_bounds(arr1, N);
    test_multiple_constructs(arr1, arr2, arr3, N);
    
    /* Test with static array for reduction */
    static int static_arr[SIZE];
    for (int i = 0; i < SIZE; i++) static_arr[i] = 1;
    test_reduction_array_section(static_arr);
    
    /* Ensure arrays are used to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + multi_dim_arr[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += static_arr[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    
    return 0;
}
