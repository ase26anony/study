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

/* Helper functions for complex base expressions */
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

/* Function using various OpenMP array section patterns */
void test_array_sections(void) {
    int arr1[N], arr2[N], arr3[N];
    int *ptr1 = arr1;
    struct Container s = {0};
    struct Container *p = &s;
    
    int i = 0, j = 10;
    int start = 5, end = 95;
    int len = 30;
    int offset = 8, count = 12;
    int flag = 1;
    int idx = 0;
    int sum = 0;
    
    /* Initialize arrays */
    for (int k = 0; k < N; k++) {
        arr1[k] = k;
        arr2[k] = k * 2;
        arr3[k] = k * 3;
        s.arr[k] = k * 4;
    }
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (int k = 0; k < N; k++) arr1[k] *= 2;
    }
    
    /* 2. Array section with variable bounds */
    #pragma omp target data map(to: arr2[i:j]) map(from: arr3[start:end-start])
    {
        #pragma omp target map(tofrom: arr2[i:j])
        for (int k = i; k < i + j; k++) arr2[k] += 3;
    }
    
    /* 3. Complex base: pointer dereference */
    #pragma omp task depend(inout: (*((int (*)[N])ptr1))[0:len])
    {
        for (int k = 0; k < len; k++) (*((int (*)[N])ptr1))[k] += 5;
    }
    
    /* 4. Complex base: structure member access */
    #pragma omp target map(tofrom: s.arr[1:5]) map(tofrom: p->arr[2:8])
    {
        for (int k = 1; k < 6; k++) s.arr[k] *= 2;
        for (int k = 2; k < 10; k++) p->arr[k] += 7;
    }
    
    /* 5. Complex base: function call returning pointer */
    #pragma omp task depend(in: get_array()[0:N])
    {
        int *tmp = get_array();
        for (int k = 0; k < N; k++) tmp[k] = k;
    }
    
    /* 6. Complex base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
    {
        int *int_buf = (int *)buffer;
        for (int k = offset; k < offset + count; k++) int_buf[k] = k * 2;
    }
    
    /* 7. Array section with arithmetic expressions */
    #pragma omp target teams distribute parallel for map(to: arr1[start+1:end-start-1])
    for (int k = start + 1; k < end; k++) {
        arr1[k] = arr1[k] * arr1[k-1];
    }
    
    /* 8. Array section with function calls as bounds */
    #pragma omp task depend(out: arr2[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++) arr2[k] = 0;
    }
    
    /* 9. Array section with conditional (ternary) expression */
    #pragma omp target map(tofrom: arr3[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + len; k++) arr3[k] += 100;
    }
    
    /* 10. Array section in linear clause (requires valid iteration variable) */
    #pragma omp parallel for reduction(+:sum) linear(arr1[idx:1]:1)
    for (idx = 0; idx < N; idx++) {
        sum += arr1[idx];
        arr1[idx] = sum;
    }
    
    /* 11. Multiple array sections in same directive */
    #pragma omp target teams distribute parallel for \
        map(to: arr1[0:n], arr2[0:n]) map(from: arr3[0:n])
    for (int k = 0; k < N; k++) {
        arr3[k] = arr1[k] + arr2[k];
    }
    
    /* 12. Nested array sections in depend clauses */
    #pragma omp task depend(in: arr1[0:1]) depend(out: arr2[1:N-1])
    {
        arr2[1] = arr1[0];
        for (int k = 2; k < N; k++) arr2[k] = arr2[k-1] * 2;
    }
}

/* Additional test functions to increase variety */
void test_more_complex_bases(void) {
    int matrix[10][20];
    int (*row_ptr)[20] = &matrix[5];
    
    /* Array section with multidimensional base */
    #pragma omp target map(tofrom: row_ptr[0][3:10])
    {
        for (int k = 3; k < 13; k++) row_ptr[0][k] = k;
    }
    
    /* Array section with pointer arithmetic base */
    int *dynamic_arr = malloc(N * sizeof(int));
    #pragma omp target map(tofrom: dynamic_arr[5:15])
    {
        for (int k = 5; k < 20; k++) dynamic_arr[k] = k * 3;
    }
    free(dynamic_arr);
}

/* Main driver to ensure code is not eliminated */
int main(void) {
    test_array_sections();
    test_more_complex_bases();
    
    printf("OpenMP array section tests completed (compile with -fopenmp -fdump-tree-*)\n");
    return 0;
}
