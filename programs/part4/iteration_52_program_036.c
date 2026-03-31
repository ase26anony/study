/* test-omp-array-sections.c
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
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
void test_pointer_deref(struct Container* p, int n) {
    int* ptr = p->arr;
    #pragma omp target map((*ptr)[0:n])
    {
        for (int i = 0; i < n; i++) {
            ptr[i] = i;
        }
    }
}

/* Function using array section with structure member access */
void test_struct_member(struct Container s, int start, int len) {
    #pragma omp task depend(inout: s.arr[start:len])
    {
        for (int i = 0; i < len; i++) {
            s.arr[start + i] *= 2;
        }
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
    #pragma omp target map(tofrom: ((int*)buffer)[offset:count])
    {
        for (int i = 0; i < count; i++) {
            ((int*)buffer)[offset + i] = i + 1;
        }
    }
}

/* Function with complex lower bound and length expressions */
void test_complex_bounds(int* arr, int start, int end, int flag, int len) {
    /* Using arithmetic expressions */
    #pragma omp task depend(in: arr[start+1:end-start-1])
    {
        for (int i = start + 1; i < end; i++) {
            arr[i] += 1;
        }
    }
    
    /* Using ternary operator in lower bound */
    #pragma omp task depend(out: arr[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int i = 0; i < len; i++) {
            arr[lower + i] = 0;
        }
    }
    
    /* Using function calls in bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        for (int i = compute_lower(); i < compute_lower() + compute_length(); i++) {
            arr[i] = i * 3;
        }
    }
}

/* Function with multiple array sections in different clauses */
void test_multiple_sections(int* a, int* b, int* c, int n) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function with linear clause using array section */
void test_linear_clause(int* arr, int n) {
    #pragma omp parallel for linear(arr[0:1]:1)
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_add : int [SIZE] : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void test_reduction_array_section(int arr[SIZE], int size) {
    #pragma omp parallel for reduction(arr_add: arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] += 1;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* arr3 = (int*)malloc(N * sizeof(int));
    char* buffer = (char*)malloc(N * sizeof(int));
    int arr4[SIZE] = {0};
    
    struct Container s;
    struct Container* p = &s;
    s.ptr_arr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    
    /* Test various array section patterns */
    test_pointer_deref(p, 10);
    test_struct_member(s, 5, 15);
    test_func_call_base(20);
    test_cast_base(buffer, 5, 10);
    test_complex_bounds(arr1, 10, 30, 1, 15);
    test_multiple_sections(arr1, arr2, arr3, N);
    test_linear_clause(arr1, N);
    test_reduction_array_section(arr4, SIZE);
    
    /* Ensure data is used to prevent optimization */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += arr4[i];
    }
    
    printf("Result check: %d\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    
    return 0;
}
