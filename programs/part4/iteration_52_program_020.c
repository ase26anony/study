/* test_omp_array_sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * It contains various OpenMP constructs with array sections that have
 * complex base expressions, varied bounds, and appear in different clauses.
 *
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test_omp_array_sections.c
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

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int local_arr[N];
    int* ptr = local_arr;
    struct Container s;
    struct Container* p = &s;
    
    /* Base: array variable */
    #pragma omp target map(tofrom: local_arr[0:N])
    {
        for (int i = 0; i < N; i++)
            local_arr[i] = i;
    }
    
    /* Base: pointer dereference - triggers parentheses in pretty printer */
    #pragma omp target data map(tofrom: (*ptr)[0:N/2])
    {
        #pragma omp target map(tofrom: (*ptr)[0:N/2])
        for (int i = 0; i < N/2; i++)
            ptr[i] *= 2;
    }
    
    /* Base: structure member access */
    #pragma omp target map(tofrom: s.arr[1:5])
    {
        for (int i = 1; i < 6; i++)
            s.arr[i] = i * 2;
    }
    
    /* Base: pointer to structure member access */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 2; i < 10; i++)
            p->arr[i] = i * 3;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:N])
    {
        int* arr = get_array();
        for (int i = 0; i < N; i++)
            arr[i] = i * 4;
    }
}

/* Function using array section in depend clause with varied bounds */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    int i = 5, j = 15;
    
    /* Simple variable bounds */
    #pragma omp task depend(inout: x[start:len])
    {
        for (int k = start; k < start + len; k++)
            x[k] = k;
    }
    
    /* Arithmetic expression bounds */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:N-1])
    {
        y[1] = x[0];
        for (int k = 2; k < N; k++)
            y[k] = y[k-1] + 1;
    }
    
    /* Function call bounds */
    #pragma omp task depend(inout: x[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++)
            x[k] *= 2;
    }
    
    /* Ternary operator in lower bound */
    int flag = 1;
    #pragma omp task depend(inout: y[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (int k = lower; k < lower + len; k++)
            y[k] = k * 2;
    }
}

/* Function using array section with cast expression base */
void test_cast_base(void) {
    char buffer[N * sizeof(int)];
    int offset = 5, count = 10;
    
    /* Base: cast expression - triggers parentheses in pretty printer */
    #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
    {
        int* int_buf = (int*)buffer;
        for (int i = offset; i < offset + count; i++)
            int_buf[i] = i;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[SIZE];
    int sum = 0;
    int idx = 0;
    
    /* Initialize array */
    for (int i = 0; i < SIZE; i++)
        arr[i] = i + 1;
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[idx:1]:1)
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
        arr[idx]++;
    }
    
    /* Teams distribute with multiple array sections */
    #pragma omp target teams distribute parallel for \
        map(to: arr[0:SIZE/2]) map(from: arr[SIZE/2:SIZE/2])
    for (int i = 0; i < SIZE; i++) {
        if (i < SIZE/2)
            arr[i] = i;
        else
            arr[i] = arr[i - SIZE/2] * 2;
    }
}

/* Complex nested expression as base */
void test_nested_base(void) {
    struct Container containers[5];
    int index = 2;
    
    /* Base: array element access with structure member */
    #pragma omp target map(tofrom: containers[index].arr[0:10])
    {
        for (int i = 0; i < 10; i++)
            containers[index].arr[i] = i * index;
    }
    
    /* Base: pointer dereference with array subscript */
    int* ptr_array[5];
    for (int i = 0; i < 5; i++)
        ptr_array[i] = malloc(N * sizeof(int));
    
    #pragma omp target map(tofrom: (*ptr_array[3])[5:15])
    {
        for (int i = 5; i < 20; i++)
            ptr_array[3][i] = i * 3;
    }
    
    for (int i = 0; i < 5; i++)
        free(ptr_array[i]);
}

/* Main driver function */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_map_clause();
    test_depend_clause();
    test_cast_base();
    test_reduction_context();
    test_nested_base();
    
    printf("OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
