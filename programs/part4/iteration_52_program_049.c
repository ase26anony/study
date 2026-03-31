/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
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

/* Function using various OpenMP array section patterns */
void test_array_sections(void) {
    int arr1[N];
    int arr2[N];
    int arr3[N];
    int *ptr1 = arr1;
    struct Container s = {0};
    struct Container *p = &s;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (int i = 0; i < N; i++) arr1[i] = i;
    }
    
    /* 2. Array section with variable bounds */
    int start = 10, len = 30;
    #pragma omp target data map(tofrom: arr2[start:len])
    {
        #pragma omp target map(tofrom: arr2[start:len])
        for (int i = start; i < start + len; i++) arr2[i] = i * 2;
    }
    
    /* 3. Complex base expression: pointer dereference */
    #pragma omp task depend(inout: (*ptr1)[0:N]) shared(ptr1)
    {
        for (int i = 0; i < N; i++) (*ptr1)[i] += 1;
    }
    
    /* 4. Complex base expression: structure member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 1; i < 6; i++) s.arr[i] = i * 3;
    }
    
    /* 5. Complex base expression: pointer to structure member */
    #pragma omp task depend(out: p->arr[2:8]) shared(p)
    {
        for (int i = 2; i < 10; i++) p->arr[i] = i * 4;
    }
    
    /* 6. Complex base expression: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:SIZE])
    {
        int *tmp = get_array();
        for (int i = 0; i < SIZE; i++) tmp[i] = i * 5;
    }
    
    /* 7. Complex base expression: cast expression */
    char buffer[N * sizeof(int)];
    int offset = 4, count = 12;
    #pragma omp task depend(in: ((int *)buffer)[offset:count])
    {
        int *int_buf = (int *)buffer;
        for (int i = offset; i < offset + count; i++) int_buf[i] = i;
    }
    
    /* 8. Arithmetic expressions in bounds */
    int end = 40;
    #pragma omp target teams distribute parallel for map(to: arr3[start+1:end-start-1])
    for (int i = start + 1; i < end; i++) {
        arr3[i] = arr1[i] + arr2[i];
    }
    
    /* 9. Function calls in bounds */
    #pragma omp task depend(in: arr1[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = lower; i < lower + length; i++) arr1[i] *= 2;
    }
    
    /* 10. Conditional expression in lower bound */
    int flag = 1;
    #pragma omp task depend(out: arr2[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = lower; i < lower + len; i++) arr2[i] = -arr2[i];
    }
    
    /* 11. Multiple array sections in same directive */
    #pragma omp target teams distribute parallel for \
        map(to: arr1[0:n], arr2[0:n]) map(from: arr3[0:n]) \
        shared(arr1, arr2, arr3) private(n)
    for (int i = 0; i < n; i++) {
        arr3[i] = arr1[i] * arr2[i];
    }
    
    /* 12. Linear clause with array section (GCC extension) */
    int idx = 0;
    #pragma omp parallel for linear(arr1[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr1[idx] += i;
        idx++;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_plus : int [N] : \
    for (int i = 0; i < N; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_with_section(void) {
    int arr[N];
    for (int i = 0; i < N; i++) arr[i] = i;
    
    /* 13. Reduction with array section (requires custom reduction) */
    #pragma omp parallel for reduction(array_plus: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] += 1;
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays to prevent dead code elimination */
    test_array_sections();
    test_reduction_with_section();
    
    printf("OpenMP array section test completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
