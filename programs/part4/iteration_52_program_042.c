/* test-omp-array-sections.c
 * 
 * This test is designed to trigger the OMP_ARRAY_SECTION pretty-printing
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
int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }
int* get_array(void) { static int arr[N]; return arr; }

struct Container {
    int arr[N];
    int* ptr;
};

/* Function with various OpenMP array sections */
void test_array_sections(void) {
    int arr1[N], arr2[N], arr3[N];
    int *ptr1 = arr1;
    struct Container s = {0};
    struct Container *p = &s;
    
    int i = 0, j = 10;
    int start = 5, end = 95;
    int flag = 1;
    int idx = 0;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(arr1[0:N])
    {
        for (int k = 0; k < N; k++) arr1[k] = k;
    }
    
    /* 2. Complex base: pointer dereference */
    #pragma omp target map((*ptr1)[i:10])
    {
        for (int k = i; k < i+10; k++) arr1[k] *= 2;
    }
    
    /* 3. Complex base: structure member access */
    #pragma omp target map(s.arr[1:5])
    {
        for (int k = 1; k < 6; k++) s.arr[k] = k;
    }
    
    /* 4. Complex base: pointer to structure member */
    #pragma omp target map(p->arr[2:8])
    {
        for (int k = 2; k < 10; k++) p->arr[k] = k * 2;
    }
    
    /* 5. Complex base: function call returning pointer */
    #pragma omp target map(get_array()[0:N/2])
    {
        int *tmp = get_array();
        for (int k = 0; k < N/2; k++) tmp[k] = k;
    }
    
    /* 6. Complex base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(((int *)buffer)[10:20])
    {
        int *int_buf = (int *)buffer;
        for (int k = 10; k < 30; k++) int_buf[k] = k;
    }
    
    /* 7. Variable lower bound and length */
    #pragma omp target map(arr2[i:j])
    {
        for (int k = i; k < i+j; k++) arr2[k] = k * 3;
    }
    
    /* 8. Arithmetic expressions in bounds */
    #pragma omp target map(arr3[start+1:end-start-1])
    {
        for (int k = start+1; k < end; k++) arr3[k] = k * 4;
    }
    
    /* 9. Function calls in bounds */
    #pragma omp target map(arr1[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower+length; k++) arr1[k] += 1;
    }
    
    /* 10. Conditional (ternary) expression in lower bound */
    #pragma omp target map(arr2[flag ? 0 : 10: N/2])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower+N/2; k++) arr2[k] = k * 5;
    }
    
    /* 11. Array section in depend clause */
    #pragma omp task depend(inout: arr1[0:10])
    {
        for (int k = 0; k < 10; k++) arr1[k] += 100;
    }
    
    /* 12. Multiple array sections in map clause */
    #pragma omp target teams distribute parallel for \
            map(to: arr1[0:N], arr2[0:N]) map(from: arr3[0:N])
    for (int k = 0; k < N; k++) {
        arr3[k] = arr1[k] + arr2[k];
    }
    
    /* 13. Array sections in depend clause with multiple dependencies */
    #pragma omp task depend(in: arr1[0:1]) depend(out: arr2[1:N-1])
    {
        for (int k = 1; k < N; k++) arr2[k] = arr1[0] * k;
    }
    
    /* 14. Linear clause with array section (requires parentheses) */
    #pragma omp parallel for linear(arr1[idx:1]:1)
    for (idx = 0; idx < N; idx++) {
        arr1[idx] += idx;
    }
    
    /* 15. Target data with array section */
    #pragma omp target data map(tofrom: arr1[0:N])
    {
        #pragma omp target map(alloc: arr1[0:N])
        {
            for (int k = 0; k < N; k++) arr1[k] *= 2;
        }
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_reduction:int *: \
    for (int i = 0; i < N; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_with_array_section(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    /* 16. Reduction with array section (custom reduction) */
    #pragma omp parallel for reduction(arr_reduction: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

/* Main function to ensure code is not eliminated */
int main(void) {
    /* Allocate and initialize arrays */
    test_array_sections();
    test_reduction_with_array_section();
    
    printf("Test completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
