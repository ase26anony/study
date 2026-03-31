/* test-omp-array-sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * It contains various OpenMP constructs with array sections that have
 * complex base expressions, varied bounds, and appear in different clauses.
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
    static int arr[N];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in target data directive */
void test_target_data(struct Container* s, int* ptr, int n) {
    /* Base: structure member access */
    #pragma omp target data map(tofrom: s->arr[0:n])
    {
        /* Base: pointer dereference */
        #pragma omp target map(to: (*ptr)[0:n])
        {
            for (int i = 0; i < n; i++) {
                s->arr[i] = i;
            }
        }
    }
}

/* Function using array section in depend clauses */
void test_task_depend(int* x, int* y, int start, int len, int size) {
    /* Base: simple array, lower bound: variable, length: arithmetic expression */
    #pragma omp task depend(in: x[start:len]) \
                     depend(out: y[1:size-1])
    {
        for (int i = 0; i < len; i++) {
            y[i+1] = x[start + i] * 2;
        }
    }
    
    /* Base: function call returning pointer */
    #pragma omp task depend(inout: get_array()[compute_lower():compute_length()])
    {
        int* arr = get_array();
        for (int i = 0; i < compute_length(); i++) {
            arr[compute_lower() + i] += 1;
        }
    }
}

/* Function using array section in map clause with teams/distribute */
void test_teams_distribute(int* a, int* b, int* c, int n) {
    /* Multiple array sections in map clauses */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function using array section in linear clause */
void test_linear_clause(int* arr, int idx, int m) {
    /* Base: array with index, length: constant */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < m; i++) {
        arr[idx] += i;
        idx++;
    }
}

/* Function with cast expression as base */
void test_cast_base(char* buffer, int offset, int count) {
    /* Base: cast expression */
    #pragma omp target map(tofrom: ((int*)buffer)[offset:count])
    {
        int* int_buf = (int*)buffer;
        for (int i = 0; i < count; i++) {
            int_buf[offset + i] = i * 10;
        }
    }
}

/* Function with ternary operator in lower bound */
void test_ternary_bound(int* arr, int flag, int len) {
    /* Lower bound: ternary conditional expression */
    #pragma omp target map(tofrom: arr[flag ? 0 : 10: len])
    {
        int start = flag ? 0 : 10;
        for (int i = 0; i < len; i++) {
            arr[start + i] = arr[start + i] * 3;
        }
    }
}

/* Custom reduction for array section (requires declaration) */
#pragma omp declare reduction(array_sec_reduction : int : \
    omp_out += omp_in) initializer(omp_priv = 0)

void test_reduction_array_section(int* arr, int size) {
    int sum = 0;
    /* Array section in reduction clause - may require custom reduction */
    #pragma omp parallel for reduction(array_sec_reduction: arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays */
    int* dynamic_arr = (int*)malloc(N * sizeof(int));
    int static_arr[N];
    int a[N], b[N], c[N];
    char buffer[N * sizeof(int)];
    struct Container s;
    s.ptr_arr = (int*)malloc(N * sizeof(int));
    
    int* ptr_to_array = static_arr;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i;
        static_arr[i] = i * 2;
        a[i] = i;
        b[i] = N - i;
        s.arr[i] = 0;
        s.ptr_arr[i] = i * 3;
    }
    
    /* Call functions with various OpenMP array sections */
    test_target_data(&s, &ptr_to_array, 50);
    
    #pragma omp parallel
    #pragma omp single
    {
        test_task_depend(static_arr, dynamic_arr, 10, 20, N);
    }
    
    test_teams_distribute(a, b, c, N);
    test_linear_clause(dynamic_arr, 5, 20);
    test_cast_base(buffer, 0, 25);
    test_ternary_bound(static_arr, 1, 30);
    test_reduction_array_section(s.arr, SIZE);
    
    /* Ensure arrays are used to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dynamic_arr[i] + static_arr[i] + a[i] + b[i] + c[i] + s.arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dynamic_arr);
    free(s.ptr_arr);
    
    return 0;
}
