/* test-omp-array-sections.c
 * 
 * This program is designed to exercise GCC's pretty-printer for OpenMP array sections.
 * It contains various OpenMP directives with array section expressions that should
 * trigger the OMP_ARRAY_SECTION case in tree-pretty-print.cc (lines 2736-2748).
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex expressions */
int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }
int* get_array(void) { static int arr[SIZE]; return arr; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function with various OpenMP array sections */
void test_array_sections(int *arr, int n, struct Container *s, int **ptr, int flag) {
    int i, j, start, end, len, idx;
    int *buffer = arr;
    
    start = 10;
    end = 90;
    len = 30;
    idx = 0;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (i = 0; i < n; i++) {
            arr[i] = i;
        }
    }
    
    /* 2. Array section with complex base: structure member */
    #pragma omp target map(tofrom: s->arr[0:N])
    {
        for (i = 0; i < N; i++) {
            s->arr[i] = i * 2;
        }
    }
    
    /* 3. Array section with complex base: pointer dereference */
    #pragma omp task depend(inout: (*ptr)[0:len])
    {
        for (i = 0; i < len; i++) {
            (*ptr)[i] *= 2;
        }
    }
    
    /* 4. Array section with complex base: function call */
    #pragma omp target data map(tofrom: get_array()[0:SIZE])
    {
        #pragma omp target teams distribute parallel for map(to: get_array()[0:SIZE])
        for (i = 0; i < SIZE; i++) {
            get_array()[i] = i * 3;
        }
    }
    
    /* 5. Array section with complex base: cast expression */
    #pragma omp task depend(in: ((int *)buffer)[start:end-start])
    {
        for (i = start; i < end; i++) {
            ((int *)buffer)[i] = i;
        }
    }
    
    /* 6. Array section with variable lower bound and length */
    #pragma omp target teams distribute parallel for map(to: arr[i:j]) private(i, j)
    for (i = 0; i < 1; i++) {
        j = 10;
        arr[i] = 0;
    }
    
    /* 7. Array section with arithmetic expressions */
    #pragma omp task depend(out: arr[start+1:end-start-1])
    {
        for (i = start + 1; i < end - 1; i++) {
            arr[i] = i * 4;
        }
    }
    
    /* 8. Array section with function calls in bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        for (i = compute_lower(); i < compute_lower() + compute_length(); i++) {
            arr[i] = i * 5;
        }
    }
    
    /* 9. Array section with conditional (ternary) expression */
    #pragma omp task depend(inout: arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (i = lower; i < lower + len; i++) {
            arr[i] = i * 6;
        }
    }
    
    /* 10. Multiple array sections in same directive */
    #pragma omp target data map(to: arr[0:n], s->arr[0:N]) map(from: buffer[0:n])
    {
        #pragma omp target teams distribute parallel for
        for (i = 0; i < n; i++) {
            buffer[i] = arr[i] + s->arr[i % N];
        }
    }
    
    /* 11. Array section in linear clause (OpenMP 5.0+) */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (idx = 0; idx < n; idx++) {
        arr[idx] += idx;
    }
}

/* Custom reduction for array section (OpenMP 5.0+) */
#pragma omp declare reduction(array_sec_reduction : int : \
    omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

void test_reduction_with_section(int *arr, int size) {
    int i;
    
    /* 12. Array section in reduction clause with custom reduction */
    #pragma omp parallel for reduction(array_sec_reduction: arr[0:size])
    for (i = 0; i < size; i++) {
        arr[0] += arr[i];
    }
}

/* Main function to drive the tests */
int main(void) {
    int *arr1, *arr2;
    struct Container s;
    int *ptr_arr;
    int flag = 1;
    
    /* Allocate and initialize arrays */
    arr1 = (int *)malloc(N * sizeof(int));
    arr2 = (int *)malloc(N * sizeof(int));
    s.ptr_arr = (int *)malloc(N * sizeof(int));
    ptr_arr = arr2;
    
    if (!arr1 || !arr2 || !s.ptr_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        s.arr[i] = i * 3;
        s.ptr_arr[i] = i * 4;
    }
    
    /* Call functions with OpenMP array sections */
    test_array_sections(arr1, N, &s, &ptr_arr, flag);
    test_reduction_with_section(arr1, N);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + s.arr[i];
    }
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(s.ptr_arr);
    
    return 0;
}
