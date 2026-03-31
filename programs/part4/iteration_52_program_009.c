/* test_omp_array_sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * It uses various OpenMP array section expressions with complex base
 * expressions, varied bounds, and multiple OpenMP constructs.
 *
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test_omp_array_sections.c
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

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

void target_region(int* arr, int n) {
    /* Simple array section in map clause */
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (int i = 0; i < n; i++) {
            arr[i] *= 2;
        }
    }
}

void task_with_depend(int* x, int* y, int start, int len) {
    /* Array sections in depend clauses */
    #pragma omp task depend(in: x[0:1]) depend(out: y[start:len])
    {
        for (int i = 0; i < len; i++) {
            y[start + i] = x[0] + i;
        }
    }
}

void complex_base_expressions(struct Container* s, int** ptr, int* buffer, 
                              int offset, int count, int flag) {
    /* Various complex base expressions for array sections */
    
    /* 1. Structure member access as base */
    #pragma omp target map(tofrom: s->arr[1:5])
    {
        for (int i = 0; i < 5; i++) {
            s->arr[i + 1] += i;
        }
    }
    
    /* 2. Pointer dereference as base (requires pointer to array) */
    int (*array_ptr)[N] = (int(*)[N])s->arr;
    #pragma omp target map(tofrom: (*array_ptr)[10:15])
    {
        for (int i = 0; i < 15; i++) {
            (*array_ptr)[10 + i] = i;
        }
    }
    
    /* 3. Function call returning pointer as base */
    #pragma omp target map(tofrom: get_array()[0:10])
    {
        int* arr = get_array();
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* 4. Cast expression as base */
    #pragma omp target map(tofrom: ((int*)buffer)[offset:count])
    {
        for (int i = 0; i < count; i++) {
            ((int*)buffer)[offset + i] = i * 3;
        }
    }
    
    /* 5. Conditional expression in lower bound */
    #pragma omp target map(tofrom: s->arr[flag ? 0 : 10: 20])
    {
        int start = flag ? 0 : 10;
        for (int i = 0; i < 20; i++) {
            s->arr[start + i] += 1;
        }
    }
}

void varied_bounds_expressions(int* arr, int i, int j, int start, int end) {
    /* Array sections with diverse bound expressions */
    
    /* 1. Variable expressions */
    #pragma omp target map(tofrom: arr[i:j])
    {
        for (int k = 0; k < j; k++) {
            arr[i + k] = k;
        }
    }
    
    /* 2. Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        int len = end - start - 1;
        for (int k = 0; k < len; k++) {
            arr[start + 1 + k] *= 2;
        }
    }
    
    /* 3. Function calls in bounds */
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = 0; k < length; k++) {
            arr[lower + k] = lower + k;
        }
    }
}

void multiple_omp_constructs(int* a, int* b, int* c, int n, int* sum, 
                             int* arr, int idx, int size) {
    /* Various OpenMP constructs with array sections */
    
    /* 1. target data with map */
    #pragma omp target data map(tofrom: a[0:n])
    {
        #pragma omp target map(to: b[0:n]) map(from: c[0:n])
        {
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* 2. target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
    
    /* 3. Linear clause with array section (requires pointer) */
    int* ptr = arr;
    #pragma omp parallel for reduction(+:sum[0]) linear(ptr[idx:1]:1)
    for (int i = 0; i < size; i++) {
        ptr[idx] += i;
        idx++;  /* Linear clause modifies idx */
    }
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_add : int* : \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void reduction_with_array_section(int* arr, int size) {
    /* Note: Custom reduction with array section - may not create 
       OMP_ARRAY_SECTION node but included for completeness */
    #pragma omp parallel for reduction(arr_add: arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] += i;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    int* arr3 = (int*)malloc(N * sizeof(int));
    int* buffer = (int*)malloc(N * sizeof(int));
    int sum[1] = {0};
    
    struct Container s;
    s.ptr_arr = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        buffer[i] = i;
        s.arr[i] = i * 3;
        s.ptr_arr[i] = i * 4;
    }
    
    /* Call functions with various OpenMP array sections */
    target_region(arr1, N);
    
    #pragma omp parallel
    #pragma omp single
    {
        task_with_depend(arr1, arr2, 10, 20);
    }
    
    int* ptr = s.arr;
    complex_base_expressions(&s, &ptr, buffer, 5, 10, 1);
    
    varied_bounds_expressions(arr3, 0, N/2, 10, 90);
    
    multiple_omp_constructs(arr1, arr2, arr3, N, sum, arr1, 0, N);
    
    /* Verify some results to prevent optimization */
    int check = 0;
    for (int i = 0; i < N; i++) {
        check += arr1[i] + arr2[i] + arr3[i] + s.arr[i];
    }
    printf("Check sum: %d\n", check);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(buffer);
    free(s.ptr_arr);
    
    return 0;
}
