/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dumps: -fdump-tree-all, -fdump-tree-gimple
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

/* Function using array section with pointer dereference as base */
void test_pointer_deref_base(int (*ptr)[N]) {
    #pragma omp target map((*ptr)[0:N])
    {
        for (int i = 0; i < N; i++) {
            (*ptr)[i] = i;
        }
    }
}

/* Function using array section with structure member access as base */
void test_struct_member_base(struct Container* p) {
    int start = 10, len = 30;
    #pragma omp task depend(inout: p->arr[start:len])
    {
        for (int i = start; i < start + len; i++) {
            p->arr[i] *= 2;
        }
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    int n = 15;
    #pragma omp target data map(tofrom: get_array()[0:n])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:n])
        for (int i = 0; i < n; i++) {
            get_array()[i] = i * 2;
        }
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(char* buffer, int offset, int count) {
    #pragma omp target map(((int*)buffer)[offset:count])
    {
        for (int i = 0; i < count; i++) {
            ((int*)buffer)[offset + i] = i + 1;
        }
    }
}

/* Function with varied lower bound and length expressions */
void test_varied_bounds(int* arr, int size) {
    int i = 5, j = 10;
    int start = 2, end = 40;
    int flag = 1;
    
    /* Integer constants */
    #pragma omp target map(arr[0:100])
    {
        arr[0] = 1;
    }
    
    /* Variable expressions */
    #pragma omp task depend(in: arr[i:j])
    {
        for (int k = i; k < i + j; k++) {
            arr[k] = k;
        }
    }
    
    /* Arithmetic expressions */
    #pragma omp target teams distribute parallel for \
        map(to: arr[start+1:end-start-1])
    for (int k = start + 1; k < end; k++) {
        arr[k] = arr[k] * 3;
    }
    
    /* Function calls */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++) {
            arr[k] = -k;
        }
    }
    
    /* Conditional (ternary) expression */
    #pragma omp target map(arr[flag ? 0 : 10: size/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + size/2; k++) {
            arr[k] = arr[k] + 5;
        }
    }
}

/* Function with linear clause using array section */
void test_linear_clause(int* arr, int n) {
    #pragma omp parallel for linear(arr[0:1]:1)
    for (int idx = 0; idx < n; idx++) {
        arr[idx] = idx * idx;
    }
}

/* Function with reduction-like pattern using array section */
void test_reduction_pattern(int* arr, int size) {
    int sum = 0;
    
    /* Note: Standard OpenMP reduction doesn't directly support array sections,
     * but we can use a manual reduction pattern */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Array section in depend clause */
    int temp[10];
    #pragma omp task depend(in: arr[0:1]) depend(out: temp[1:9])
    {
        for (int i = 1; i < 10; i++) {
            temp[i] = arr[0] * i;
        }
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int matrix[N][N];
    char* buffer = (char*)malloc(SIZE * sizeof(int));
    struct Container s;
    struct Container* p = &s;
    
    s.ptr_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        s.arr[i] = i * 2;
        s.ptr_arr[i] = i * 3;
    }
    
    for (int i = 0; i < SIZE; i++) {
        ((int*)buffer)[i] = 0;
    }
    
    /* Call test functions to generate OMP_ARRAY_SECTION nodes */
    test_pointer_deref_base(&matrix);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            test_struct_member_base(p);
        }
    }
    
    test_function_call_base();
    test_cast_base(buffer, 5, 10);
    test_varied_bounds(arr1, N);
    test_linear_clause(arr2, N);
    test_reduction_pattern(s.arr, N);
    
    /* Ensure all tasks complete */
    #pragma omp taskwait
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(buffer);
    free(s.ptr_arr);
    
    printf("Test completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
