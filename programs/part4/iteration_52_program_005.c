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

/* Function returning a pointer to use as base expression */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

/* Function to compute lower bound */
int compute_lower(void) {
    return 5;
}

/* Function to compute length */
int compute_length(void) {
    return 20;
}

/* Structure with array member */
struct WithArray {
    int arr[N];
    int* ptr_arr;
};

/* Function containing various OpenMP array section expressions */
void test_array_sections(void) {
    int arr1[N];
    int arr2[N];
    int arr3[N];
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    int start = 10, end = 90;
    int i = 0, j = 50;
    int flag = 1;
    int len = 30;
    int offset = 5, count = 25;
    int idx = 0;
    int sum = 0;
    
    struct WithArray s;
    struct WithArray *p = &s;
    s.ptr_arr = arr3;
    
    /* 1. Simple array section in map clause - base is array variable */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (int k = 0; k < N; k++) {
            arr1[k] = k;
        }
    }
    
    /* 2. Array section with variable expressions */
    #pragma omp target data map(to: arr2[i:j])
    {
        #pragma omp target map(from: arr2[i:j])
        {
            for (int k = i; k < i + j; k++) {
                arr2[k] = k * 2;
            }
        }
    }
    
    /* 3. Array section with arithmetic expressions in bounds */
    #pragma omp target teams distribute parallel for map(to: arr1[start+1:end-start-1])
    for (int k = start + 1; k < end; k++) {
        arr1[k] = arr1[k] * 3;
    }
    
    /* 4. Array section with pointer dereference as base - tests op_prio logic */
    #pragma omp task depend(inout: (*ptr1)[0:10])
    {
        for (int k = 0; k < 10; k++) {
            (*ptr1)[k] += 1;
        }
    }
    
    /* 5. Array section with structure member access as base */
    #pragma omp task depend(in: s.arr[1:5]) depend(out: p->arr[2:8])
    {
        for (int k = 1; k < 6; k++) s.arr[k] = k;
        for (int k = 2; k < 10; k++) p->arr[k] = k * 2;
    }
    
    /* 6. Array section with function call as base */
    #pragma omp target map(tofrom: get_array()[0:N])
    {
        int *tmp = get_array();
        for (int k = 0; k < N; k++) {
            tmp[k] = k * 3;
        }
    }
    
    /* 7. Array section with cast expression as base */
    #pragma omp target data map(tofrom: ((int *)ptr2)[offset:count])
    {
        #pragma omp target map(tofrom: ((int *)ptr2)[offset:count])
        {
            for (int k = offset; k < offset + count; k++) {
                ((int *)ptr2)[k] = k * 4;
            }
        }
    }
    
    /* 8. Array section with function calls in bounds */
    #pragma omp task depend(inout: arr1[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int k = lower; k < lower + length; k++) {
            arr1[k] += 5;
        }
    }
    
    /* 9. Array section with conditional (ternary) expression in lower bound */
    #pragma omp target teams distribute parallel for map(to: arr1[flag ? 0 : 10: len])
    for (int k = (flag ? 0 : 10); k < (flag ? 0 : 10) + len; k++) {
        arr1[k] = arr1[k] * 2;
    }
    
    /* 10. Multiple array sections in same directive */
    #pragma omp target map(to: arr1[0:n], arr2[0:n]) map(from: arr3[0:n])
    {
        int n = 25;
        for (int k = 0; k < n; k++) {
            arr3[k] = arr1[k] + arr2[k];
        }
    }
    
    /* 11. Array section in linear clause */
    #pragma omp parallel for reduction(+:sum) linear(arr1[idx:1]:1)
    for (idx = 0; idx < N; idx++) {
        sum += arr1[idx];
        arr1[idx] = sum;
    }
    
    /* 12. Complex nested base expression */
    #pragma omp task depend(in: (*(p->ptr_arr))[0:10])
    {
        for (int k = 0; k < 10; k++) {
            (*(p->ptr_arr))[k] = k * 7;
        }
    }
}

/* Another function with different array section patterns */
void more_tests(void) {
    int buffer[1000];
    int *dynamic_arr = (int*)malloc(500 * sizeof(int));
    int chunk = 100;
    
    /* Array section with dynamic memory */
    #pragma omp target data map(tofrom: dynamic_arr[0:chunk])
    {
        #pragma omp target map(tofrom: dynamic_arr[0:chunk])
        {
            for (int k = 0; k < chunk; k++) {
                dynamic_arr[k] = k * 11;
            }
        }
    }
    
    /* Array section with more complex base */
    int (*arr_ptr)[100] = (int (*)[100])buffer;
    #pragma omp task depend(inout: (*arr_ptr)[10:40])
    {
        for (int k = 10; k < 50; k++) {
            (*arr_ptr)[k] = k * 13;
        }
    }
    
    free(dynamic_arr);
}

int main(void) {
    /* Initialize data */
    printf("Testing OpenMP array sections for GCC pretty-printer coverage\n");
    
    /* Call functions with OpenMP array sections */
    test_array_sections();
    more_tests();
    
    printf("Test completed successfully\n");
    return 0;
}
