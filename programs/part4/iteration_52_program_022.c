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
    
    #pragma omp target map((*ptr)[0:N/2])
    {
        for (int i = 0; i < N/2; i++)
            (*ptr)[i] = i;
    }
}

/* Function using array section with structure member access as base */
void test_struct_member(struct Container s) {
    #pragma omp task depend(inout: s.arr[10:20])
    {
        for (int i = 10; i < 30; i++)
            s.arr[i] *= 2;
    }
}

/* Function using array section with function call as base */
void test_function_call_base(void) {
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:SIZE])
        for (int i = 0; i < SIZE; i++) {
            get_array()[i] = i * 2;
        }
    }
}

/* Function using array section with cast expression as base */
void test_cast_base(char *buffer) {
    #pragma omp task depend(in: ((int *)buffer)[0:10]) \
                     depend(out: ((int *)buffer)[10:N-10])
    {
        for (int i = 0; i < N; i++)
            ((int *)buffer)[i] = i;
    }
}

/* Function with complex lower bound and length expressions */
void test_complex_bounds(int *arr, int start, int end, int flag) {
    int len = end - start;
    
    /* Array section with arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int i = start+1; i < end; i++)
            arr[i] += i;
    }
    
    /* Array section with function calls */
    #pragma omp task depend(in: arr[compute_lower():compute_length()])
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++)
            arr[i] = 0;
    }
    
    /* Array section with ternary operator in lower bound */
    #pragma omp target teams distribute parallel for \
        map(to: arr[flag ? 0 : 10: len])
    for (int i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++) {
        arr[i] = arr[i] * 3;
    }
}

/* Function using linear clause with array section */
void test_linear_clause(int *arr) {
    #pragma omp parallel for linear(arr[0:1]:1)
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
}

/* Function with multiple array sections in map clauses */
void test_multiple_sections(int *a, int *b, int *c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with nested array sections */
void test_nested_expressions(struct Container *p, int idx) {
    /* Complex base: pointer to struct member */
    #pragma omp task depend(inout: p->ptr_arr[idx:5])
    {
        for (int i = idx; i < idx + 5; i++)
            p->ptr_arr[i] = i * 2;
    }
}

/* Custom reduction for array section (requires declare reduction) */
#pragma omp declare reduction(array_sec_reduction : int : \
    omp_out[0:1] = omp_out[0:1] + omp_in[0:1]) \
    initializer(omp_priv[0:1] = 0)

void test_reduction_clause(int *arr, int size) {
    int sum = 0;
    
    /* Note: This may not create OMP_ARRAY_SECTION in all GCC versions,
       but included for completeness */
    #pragma omp parallel for reduction(array_sec_reduction: arr[0:1])
    for (int i = 0; i < size; i++) {
        arr[0] += i;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    char *buffer = (char *)malloc(N * sizeof(int));
    
    struct Container s;
    struct Container *p = &s;
    s.ptr_arr = (int *)malloc(N * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        s.arr[i] = i;
        s.ptr_arr[i] = i;
    }
    
    /* Call test functions to generate various OMP_ARRAY_SECTION nodes */
    test_pointer_deref(p);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            test_struct_member(s);
            test_function_call_base();
            test_cast_base(buffer);
        }
    }
    
    test_complex_bounds(arr1, 10, 90, 1);
    test_linear_clause(arr2);
    test_multiple_sections(arr1, arr2, arr3, 50);
    test_nested_expressions(p, 25);
    test_reduction_clause(arr1, N);
    
    /* Ensure arrays are used to prevent optimization */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + s.arr[i] + s.ptr_arr[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    free(s.ptr_arr);
    
    return 0;
}
