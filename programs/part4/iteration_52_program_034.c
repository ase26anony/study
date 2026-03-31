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
int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }
int* get_array(void) { static int arr[N]; return arr; }

struct Container {
    int arr[N];
    int* ptr;
};

/* Function with various OpenMP array sections */
void test_array_sections(int *arr, int n, int start, int end, int flag, int len, int idx) {
    int *ptr = arr;
    struct Container s;
    struct Container *p = &s;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (int i = 0; i < n; i++) arr[i] *= 2;
    }
    
    /* 2. Complex base: pointer dereference */
    #pragma omp target data map(tofrom: (*ptr)[0:n])
    {
        #pragma omp target map(tofrom: (*ptr)[0:n])
        for (int i = 0; i < n; i++) ptr[i] += i;
    }
    
    /* 3. Complex base: structure member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 1; i < 6; i++) s.arr[i] = i;
    }
    
    /* 4. Complex base: pointer to structure member */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 2; i < 10; i++) p->arr[i] = i * 2;
    }
    
    /* 5. Complex base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:n])
    {
        int *tmp = get_array();
        for (int i = 0; i < n; i++) tmp[i] = i;
    }
    
    /* 6. Complex base: cast expression */
    char *buffer = (char*)arr;
    #pragma omp target map(tofrom: ((int*)buffer)[0:n])
    {
        for (int i = 0; i < n; i++) ((int*)buffer)[i] = i * 3;
    }
    
    /* 7. Variable lower bound and length */
    #pragma omp target map(tofrom: arr[start:end-start])
    {
        for (int i = start; i < end; i++) arr[i] += 1;
    }
    
    /* 8. Arithmetic expressions in bounds */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int i = start+1; i < end-1; i++) arr[i] *= 2;
    }
    
    /* 9. Function calls in bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = lower; i < lower + length; i++) arr[i] = -arr[i];
    }
    
    /* 10. Conditional (ternary) expression in lower bound */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + len; i++) arr[i] = 0;
    }
    
    /* 11. Array section in depend clause */
    #pragma omp task depend(inout: arr[start:len])
    {
        for (int i = start; i < start + len; i++) arr[i] += 5;
    }
    
    /* 12. Multiple array sections in map clause */
    int a[N], b[N], c[N];
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* 13. Array sections in depend clauses with multiple dependencies */
    int x[N], y[N];
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:n-1])
    {
        y[1] = x[0];
        for (int i = 2; i < n; i++) y[i] = y[i-1] + 1;
    }
    
    /* 14. Linear clause with array section (requires valid syntax) */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < n; i++) {
        arr[idx] += i;
        idx++;
    }
}

/* Custom reduction for array section (if supported) */
#pragma omp declare reduction(arr_reduction:int[:] \
    : omp_out[:] = omp_in[:] + omp_out[:]) \
    initializer(omp_priv = omp_orig)

void test_reduction_with_section(int *arr, int size) {
    int sum = 0;
    /* 15. Reduction with array section - may require custom reduction */
    #pragma omp parallel for reduction(+:sum) /* reduction(+:arr[0:size]) */
    for (int i = 0; i < size; i++) {
        sum += arr[i];
        /* arr[i] += 1; */ /* Would need custom reduction for array section */
    }
}

/* Main driver to ensure code is not eliminated */
int main(void) {
    int *arr = (int*)malloc(N * sizeof(int));
    int start = 10, end = 50, flag = 1, len = 20, idx = 5;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) arr[i] = i;
    
    /* Call function with various OpenMP array sections */
    test_array_sections(arr, N, start, end, flag, len, idx);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) sum += arr[i];
    printf("Sum: %d\n", sum);
    
    free(arr);
    return 0;
}
