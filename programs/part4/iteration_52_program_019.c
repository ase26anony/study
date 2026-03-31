/* test-omp-array-sections.c
 * 
 * This test program is designed to trigger the OMP_ARRAY_SECTION
 * pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Function returning a pointer to an array */
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
    int* ptr;
};

/* Test function with various OpenMP array section expressions */
void test_array_sections(void) {
    int i, j, start, end, len, flag, idx;
    int arr1[N], arr2[N], arr3[N];
    int *ptr1 = arr1;
    int buffer[2*N];
    struct WithArray s;
    struct WithArray *p = &s;
    
    start = 10;
    end = 90;
    len = 30;
    flag = 1;
    idx = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        s.arr[i] = i * 3;
    }
    
    /* 1. Simple array section in map clause - base is simple array variable */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (i = 0; i < N; i++) {
            arr1[i] *= 2;
        }
    }
    
    /* 2. Array section with variable bounds */
    #pragma omp target data map(to: arr2[start:len]) map(from: arr3[start:len])
    {
        #pragma omp target teams distribute parallel for
        for (i = start; i < start + len; i++) {
            arr3[i] = arr2[i] + 1;
        }
    }
    
    /* 3. Array section with complex base: structure member access */
    #pragma omp task depend(inout: s.arr[1:5])
    {
        for (i = 1; i < 6; i++) {
            s.arr[i] += 10;
        }
    }
    
    /* 4. Array section with complex base: pointer dereference */
    #pragma omp task depend(in: (*ptr1)[i:10])
    {
        for (j = i; j < i + 10; j++) {
            ptr1[j] = j;
        }
    }
    
    /* 5. Array section with complex base: pointer to structure member */
    #pragma omp task depend(out: p->arr[2:8])
    {
        for (i = 2; i < 10; i++) {
            p->arr[i] = i * 5;
        }
    }
    
    /* 6. Array section with complex base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:n])
    {
        int *tmp = get_array();
        for (i = 0; i < 10; i++) {
            tmp[i] = i * 3;
        }
    }
    
    /* 7. Array section with complex base: cast expression */
    #pragma omp target map(tofrom: ((int *)buffer)[offset:count])
    {
        int offset = 5;
        int count = 15;
        for (i = offset; i < offset + count; i++) {
            ((int *)buffer)[i] = i * 7;
        }
    }
    
    /* 8. Array section with arithmetic expressions for bounds */
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    {
        int a[N], b[N], c[N];
        int n = N;
        for (i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* 9. Array section with function calls for bounds */
    #pragma omp task depend(in: arr1[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (i = lower; i < lower + length; i++) {
            arr1[i] += 100;
        }
    }
    
    /* 10. Array section with conditional (ternary) expression for lower bound */
    #pragma omp task depend(out: arr2[flag ? 0 : 10: len])
    {
        int lower = flag ? 0 : 10;
        for (i = lower; i < lower + len; i++) {
            arr2[i] = i * 11;
        }
    }
    
    /* 11. Multiple array sections in same directive */
    #pragma omp target data map(tofrom: arr1[0:N/2], arr2[N/2:N/2])
    {
        #pragma omp target
        for (i = 0; i < N; i++) {
            if (i < N/2) arr1[i] *= 3;
            else arr2[i] *= 3;
        }
    }
    
    /* 12. Array section in linear clause (though not all compilers support this) */
    #pragma omp parallel for linear(arr1[idx:1]:1)
    for (idx = 0; idx < N; idx++) {
        arr1[idx] += idx;
    }
    
    /* 13. Complex expression with parentheses needed due to priority */
    /* This should trigger op_prio check for parentheses */
    int *complex_ptr = arr1;
    #pragma omp target map(tofrom: (*complex_ptr)[start+1:end-start-1])
    {
        for (i = start + 1; i < end; i++) {
            complex_ptr[i] = i * 13;
        }
    }
}

/* Main function to drive the test */
int main(void) {
    printf("Testing OpenMP array sections for GCC pretty-printer coverage\n");
    
    test_array_sections();
    
    printf("Test completed (coverage is measured at compile time with -fdump-tree-* flags)\n");
    return 0;
}
