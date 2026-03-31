/* test_omp_array_sections.c
 * Designed to exercise GCC's OMP_ARRAY_SECTION pretty-printing logic
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test_omp_array_sections.c
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Function returning a pointer for complex base expressions */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

/* Structure with array member */
struct WithArray {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with simple base */
void test_simple_map(void) {
    int arr[N];
    
    #pragma omp target map(tofrom: arr[0:N])
    {
        for (int i = 0; i < N; i++) {
            arr[i] = i;
        }
    }
}

/* Function using array section with pointer dereference as base */
void test_pointer_deref_base(void) {
    int (*ptr)[N] = malloc(sizeof(int[N]));
    if (!ptr) return;
    
    /* Complex base: (*ptr)[i:10] */
    int i = 5;
    #pragma omp target data map(tofrom: (*ptr)[i:10])
    {
        #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[i:10])
        for (int j = 0; j < 10; j++) {
            (*ptr)[i + j] = j * 2;
        }
    }
    
    free(ptr);
}

/* Function using structure member access as base */
void test_struct_member_base(void) {
    struct WithArray s;
    struct WithArray *p = &s;
    
    /* Multiple array sections with structure member bases */
    #pragma omp target data map(tofrom: s.arr[1:5], p->arr[2:8])
    {
        #pragma omp target teams distribute parallel for \
            map(to: s.arr[1:5]) map(from: p->arr[2:8])
        for (int i = 0; i < 5; i++) {
            s.arr[1 + i] = i * 3;
            p->arr[2 + i] = i * 4;
        }
    }
}

/* Function using function call as base */
void test_function_call_base(void) {
    /* Complex base: get_array()[0:n] */
    int n = 20;
    #pragma omp target map(tofrom: get_array()[0:n])
    {
        int* arr = get_array();
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = i * i;
        }
    }
}

/* Function using cast expression as base */
void test_cast_base(void) {
    char buffer[1024];
    int offset = 16;
    int count = 32;
    
    /* Complex base: (int *)buffer)[offset:count] */
    #pragma omp target data map(tofrom: ((int *)buffer)[offset:count])
    {
        int *int_buf = (int *)buffer;
        #pragma omp target teams distribute parallel for \
            map(tofrom: ((int *)buffer)[offset:count])
        for (int i = 0; i < count; i++) {
            int_buf[offset + i] = i + 100;
        }
    }
}

/* Function with varied lower bound and length expressions */
void test_varied_bounds(void) {
    int arr[1000];
    int start = 100;
    int end = 200;
    int i = 10, j = 20;
    int flag = 1;
    
    /* Integer constants */
    #pragma omp task depend(inout: arr[0:100])
    {
        for (int k = 0; k < 100; k++) arr[k] = k;
    }
    
    /* Variable expressions */
    #pragma omp task depend(in: arr[i:j]) depend(out: arr[j:i*2])
    {
        for (int k = 0; k < j; k++) arr[i + k] = k * 2;
    }
    
    /* Arithmetic expressions */
    #pragma omp target map(tofrom: arr[start+1:end-start-1])
    {
        for (int k = 0; k < end-start-1; k++) {
            arr[start + 1 + k] = k * 3;
        }
    }
    
    /* Conditional (ternary) expression */
    #pragma omp task depend(inout: arr[flag ? 0 : 10: 50])
    {
        for (int k = 0; k < 50; k++) {
            arr[(flag ? 0 : 10) + k] = k * 4;
        }
    }
    
    /* Function calls in bounds */
    int compute_lower(void) { return 300; }
    int compute_length(void) { return 40; }
    
    #pragma omp target map(tofrom: arr[compute_lower():compute_length()])
    {
        for (int k = 0; k < compute_length(); k++) {
            arr[compute_lower() + k] = k * 5;
        }
    }
}

/* Function with reduction clause using array section */
void test_reduction_array_section(void) {
    int arr[N];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
    }
    
    /* Custom reduction for array section */
    #pragma omp declare reduction(array_sum:int:omp_out += omp_in) \
        initializer(omp_priv = 0)
    
    #pragma omp parallel for reduction(array_sum:sum) \
        map(to: arr[0:N])
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
}

/* Function with linear clause using array section */
void test_linear_array_section(void) {
    int arr[N];
    int idx = 0;
    
    #pragma omp parallel for linear(arr[idx:1]:1) \
        map(tofrom: arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[idx] = i;
        idx++;  /* Linear clause handles idx increment */
    }
}

/* Function with multiple OpenMP constructs */
void test_multiple_constructs(void) {
    int a[N], b[N], c[N];
    int x[10], y[100];
    int size = 50;
    
    /* target data with multiple array sections */
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
            map(to: a[0:N], b[0:N]) map(from: c[0:N])
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* task with depend clauses using array sections */
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:size-1])
    {
        y[1] = x[0] * 2;
        for (int i = 2; i < size; i++) {
            y[i] = y[i-1] + 1;
        }
    }
    
    /* Nested array sections in complex expressions */
    struct WithArray struct_arr[5];
    #pragma omp parallel for
    for (int i = 0; i < 5; i++) {
        #pragma omp target map(tofrom: struct_arr[i].arr[0:N/5])
        {
            for (int j = 0; j < N/5; j++) {
                struct_arr[i].arr[j] = i * 100 + j;
            }
        }
    }
}

/* Main function to drive all tests */
int main(void) {
    /* Call all test functions to ensure code generation */
    test_simple_map();
    test_pointer_deref_base();
    test_struct_member_base();
    test_function_call_base();
    test_cast_base();
    test_varied_bounds();
    test_reduction_array_section();
    test_linear_array_section();
    test_multiple_constructs();
    
    printf("All OpenMP array section tests completed (compile-time coverage).\n");
    printf("Check generated .original dump file for OMP_ARRAY_SECTION pretty-printing.\n");
    
    return 0;
}
