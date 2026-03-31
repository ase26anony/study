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
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

struct Container {
    int arr[N];
    int* ptr_arr;
};

void init_data(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
}

/* Function using various OpenMP array sections */
void test_array_sections(void) {
    int arr1[N];
    int arr2[N];
    int arr3[N];
    int* ptr = arr1;
    struct Container s;
    struct Container* p = &s;
    
    s.ptr_arr = malloc(N * sizeof(int));
    p->ptr_arr = s.ptr_arr;
    
    init_data(arr1, N);
    init_data(arr2, N);
    init_data(arr3, N);
    init_data(s.arr, N);
    init_data(s.ptr_arr, N);
    
    int start = 10;
    int len = 20;
    int offset = 5;
    int count = 15;
    int i = 0, j = 30;
    int flag = 1;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (int k = 0; k < N; k++) {
            arr1[k] *= 2;
        }
    }
    
    /* 2. Array section with variable expressions */
    #pragma omp target data map(to: arr2[start:len]) map(from: arr3[start:len])
    {
        #pragma omp target teams distribute parallel for
        for (int k = start; k < start + len; k++) {
            arr3[k] = arr2[k] + 1;
        }
    }
    
    /* 3. Complex base: pointer dereference */
    #pragma omp task depend(inout: (*ptr)[i:10])
    {
        for (int k = i; k < i + 10; k++) {
            ptr[k] = k;
        }
    }
    
    /* 4. Complex base: structure member access */
    #pragma omp task depend(in: s.arr[1:5]) depend(out: p->arr[2:8])
    {
        for (int k = 2; k < 10; k++) {
            p->arr[k] = s.arr[k-1];
        }
    }
    
    /* 5. Complex base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:N/2])
    {
        int* local_arr = get_array();
        for (int k = 0; k < N/2; k++) {
            local_arr[k] += 10;
        }
    }
    
    /* 6. Complex base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp task depend(inout: ((int *)buffer)[offset:count])
    {
        int* int_buf = (int*)buffer;
        for (int k = offset; k < offset + count; k++) {
            int_buf[k] = k * 2;
        }
    }
    
    /* 7. Array section with arithmetic expressions */
    #pragma omp target teams distribute parallel for \
                map(to: arr1[start+1:len-start-1])
    for (int k = start + 1; k < len; k++) {
        arr1[k] = arr1[k] * 3;
    }
    
    /* 8. Array section with function calls as bounds */
    int compute_lower(void) { return 5; }
    int compute_length(void) { return 25; }
    #pragma omp target map(tofrom: arr2[compute_lower():compute_length()])
    {
        for (int k = compute_lower(); k < compute_lower() + compute_length(); k++) {
            arr2[k] = arr2[k] / 2;
        }
    }
    
    /* 9. Array section with conditional (ternary) expression */
    #pragma omp task depend(in: arr3[flag ? 0 : 10: len])
    {
        for (int k = (flag ? 0 : 10); k < (flag ? 0 : 10) + len; k++) {
            arr3[k] = arr3[k] - 5;
        }
    }
    
    /* 10. Array section in linear clause (requires valid iteration variable) */
    int idx = 0;
    #pragma omp parallel for linear(arr1[idx:1]:1)
    for (idx = 0; idx < N-1; idx++) {
        arr1[idx+1] += arr1[idx];
    }
    
    /* 11. Multiple array sections in same directive */
    #pragma omp target teams distribute parallel for \
                map(to: arr1[0:n], arr2[0:n]) map(from: arr3[0:n])
    for (int k = 0; k < N; k++) {
        arr3[k] = arr1[k] + arr2[k];
    }
    
    free(s.ptr_arr);
}

/* Custom reduction for array section */
#pragma omp declare reduction(arr_reduction:int*: \
    for (int i = 0; i < SIZE; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = get_array())

void test_reduction_with_array_section(void) {
    int* arr = get_array();
    
    /* 12. Array section in reduction clause with custom reduction */
    #pragma omp parallel for reduction(arr_reduction:arr[0:SIZE])
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr[j] += i;
        }
    }
}

int main(void) {
    printf("Testing OpenMP array sections for GCC pretty-printer coverage\n");
    
    test_array_sections();
    test_reduction_with_array_section();
    
    printf("Test completed successfully\n");
    return 0;
}
