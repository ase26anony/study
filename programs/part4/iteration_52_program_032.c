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

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(struct Container *s, int n) {
    int *ptr = s->arr;
    
    /* Complex base: structure member access */
    #pragma omp target map(tofrom: s->arr[0:n])
    {
        for (int i = 0; i < n; i++)
            s->arr[i] *= 2;
    }
    
    /* Complex base: pointer dereference */
    #pragma omp target map(tofrom: (*ptr)[0:n])
    {
        for (int i = 0; i < n; i++)
            ptr[i] += 1;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(int *arr, int start, int len) {
    /* Variable expressions for bounds */
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = 0; i < len; i++)
            arr[start + i] = i;
    }
    
    /* Arithmetic expressions for bounds */
    #pragma omp task depend(in: arr[0:1]) depend(out: arr[1:len-1])
    {
        for (int i = 1; i < len; i++)
            arr[i] = arr[i-1] + 1;
    }
}

/* Function using array section with function call as base */
void test_function_call_base(int offset, int count) {
    /* Complex base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[offset:count])
    {
        int *arr = get_array();
        for (int i = 0; i < count; i++)
            arr[offset + i] = i * 2;
    }
}

/* Function using array section with cast as base */
void test_cast_base(char *buffer, int offset, int count) {
    /* Complex base: cast expression */
    #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
    {
        int *arr = (int *)buffer;
        for (int i = 0; i < count; i++)
            arr[offset + i] = i * 3;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(int *arr, int size) {
    int sum = 0;
    
    /* Array section in linear clause */
    #pragma omp parallel for reduction(+:sum) linear(arr[0:size]:1)
    for (int i = 0; i < size; i++) {
        arr[i] = i;
        sum += arr[i];
    }
}

/* Function with conditional expressions in bounds */
void test_conditional_bounds(int *arr, int flag, int len) {
    /* Ternary operator in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: len])
    {
        int start = flag ? 0 : 10;
        for (int i = 0; i < len; i++)
            arr[start + i] = i * 4;
    }
}

/* Function with multiple array sections in single directive */
void test_multiple_sections(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with nested array accesses as base */
void test_nested_base(struct Container containers[], int idx, int m) {
    /* Complex base: array of structures with member access */
    #pragma omp target map(tofrom: containers[idx].arr[0:m])
    {
        for (int i = 0; i < m; i++)
            containers[idx].arr[i] = i * 5;
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    char *buffer = (char *)malloc(N * sizeof(int));
    
    struct Container s;
    s.ptr_arr = (int *)malloc(N * sizeof(int));
    
    struct Container containers[5];
    for (int i = 0; i < 5; i++) {
        containers[i].ptr_arr = (int *)malloc(N * sizeof(int));
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        s.arr[i] = i * 3;
        s.ptr_arr[i] = i * 4;
        ((int *)buffer)[i] = i * 5;
    }
    
    /* Call functions with various OpenMP array section patterns */
    test_map_clause(&s, 20);
    test_depend_clause(arr1, 10, 30);
    test_function_call_base(5, 15);
    test_cast_base(buffer, 0, 25);
    test_reduction_context(arr2, 40);
    test_conditional_bounds(arr3, 1, 20);
    test_multiple_sections(arr1, arr2, arr3, 30);
    test_nested_base(containers, 2, 15);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + s.arr[i] + s.ptr_arr[i];
        sum += ((int *)buffer)[i];
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < N; j++) {
            sum += containers[i].ptr_arr[j];
        }
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    free(s.ptr_arr);
    
    for (int i = 0; i < 5; i++) {
        free(containers[i].ptr_arr);
    }
    
    return 0;
}
