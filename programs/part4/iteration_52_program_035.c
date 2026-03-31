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
void test_pointer_deref(struct Container* p, int n) {
    int* ptr = p->arr;
    
    #pragma omp target map((*ptr)[0:n])
    {
        for (int i = 0; i < n; i++)
            (*ptr)[i] = i;
    }
}

/* Function using array section with structure member access as base */
void test_struct_member(struct Container s, int start, int len) {
    #pragma omp task depend(inout: s.arr[start:len])
    {
        for (int i = 0; i < len; i++)
            s.arr[start + i] *= 2;
    }
}

/* Function using array section with function call as base */
void test_func_call_base(int n) {
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
    #pragma omp task depend(in: ((int*)buffer)[offset:count])
    {
        for (int i = 0; i < count; i++)
            ((int*)buffer)[offset + i] = 0;
    }
}

/* Function with complex lower bound and length expressions */
void test_complex_bounds(int* arr, int start, int end, int flag, int len) {
    /* Using arithmetic expressions for bounds */
    #pragma omp target map(arr[start+1:end-start-1])
    {
        for (int i = start+1; i < end; i++)
            arr[i] = arr[i] + 1;
    }
    
    /* Using function calls for bounds */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++)
            arr[i] = 0;
    }
    
    /* Using conditional expression for lower bound */
    #pragma omp target teams distribute parallel for \
        map(to: arr[flag ? 0 : 10: len])
    for (int i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++) {
        arr[i] = i;
    }
}

/* Function with multiple array sections in different clauses */
void test_multiple_sections(int* a, int* b, int* c, int n, int idx) {
    /* Multiple arrays in map clause */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Multiple depend clauses with array sections */
    #pragma omp task depend(in: a[0:1]) depend(out: b[1:n-1])
    {
        b[1] = a[0];
        for (int i = 2; i < n; i++)
            b[i] = b[i-1] + 1;
    }
    
    /* Linear clause with array section (GCC extension) */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < n; i++) {
        a[idx] += i;
        idx++;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(array_section_add : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(int arr[SIZE], int size) {
    /* Note: Array section reduction may require custom reduction */
    #pragma omp parallel for reduction(array_section_add: arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = 1;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* arr3 = (int*)malloc(N * sizeof(int));
    char* buffer = (char*)malloc(N * sizeof(int));
    
    struct Container s;
    struct Container* p = &s;
    s.ptr_arr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        ((int*)buffer)[i] = i;
    }
    
    /* Call test functions to generate various OMP_ARRAY_SECTION nodes */
    test_pointer_deref(p, 10);
    test_struct_member(s, 5, 15);
    test_func_call_base(20);
    test_cast_base(buffer, 3, 7);
    test_complex_bounds(arr1, 10, 50, 1, 30);
    test_multiple_sections(arr1, arr2, arr3, 25, 5);
    
    int arr_section[SIZE];
    test_reduction_array_section(arr_section, SIZE);
    
    /* Ensure all tasks complete */
    #pragma omp taskwait
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + ((int*)buffer)[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += arr_section[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    
    return 0;
}
