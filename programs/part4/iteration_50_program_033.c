/* test_ddg_coverage.c
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler, specifically targeting create_ddg_edge()
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(float *farr, int *iarr, int n);
int test_loop_carried_deps(int *arr, int n);

/* 1. Test True Data Dependencies (RAW/flow dependencies) */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Distance 2 RAW with arithmetic */
        arr[i] += arr[i-2] * 3;
        
        /* Floating point RAW chain */
        float temp = (float)arr[i] / 2.0f;
        sum += (int)(temp * 1.5f);
    }
    
    /* Another loop with longer dependency chain */
    for (int i = 3; i < n-3; i++) {
        int t1 = arr[i] + arr[i-1];
        int t2 = t1 * arr[i-2];
        int t3 = t2 - arr[i-3];
        arr[i] = t3 + g_global_array[i % 1024];
        sum += arr[i];
    }
    
    return sum;
}

/* 2. Test Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* WAR (anti-dependency) pattern */
    for (int i = 1; i < n; i++) {
        int temp = arr[i];          /* Read arr[i] */
        arr[i] = temp + i;          /* Write arr[i] - creates WAR with next iteration's read */
        sum += arr[i-1];            /* Read arr[i-1] - creates WAR with previous iteration's write */
    }
    
    /* WAW (output-dependency) pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;             /* First write */
        arr[i] = arr[i] + g_volatile; /* Second write to same location - WAW */
        
        /* Multiple writes to same variable */
        int x = arr[i];
        x = x + 1;                  /* WAW on x */
        x = x * 2;                  /* Another WAW on x */
        arr[i] = x;
    }
    
    return sum;
}

/* 3. Test Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *p = arr1;
    int *q = arr2;
    
    /* Compiler can't tell if p and q alias */
    for (int i = 1; i < n; i++) {
        *p = *q + g_volatile;      /* Read from q, write to p */
        p++;
        q++;
        
        /* Array indexing with variable indices - creates ambiguous dependencies */
        int idx1 = (i * 7) % n;
        int idx2 = (i * 13) % n;
        arr1[idx1] = arr2[idx2] + i;
        
        sum += arr1[i % n];
    }
    
    /* Pointer arithmetic that could cause overlap */
    int *ptr1 = &arr1[10];
    int *ptr2 = &arr1[5];
    for (int i = 0; i < n/2; i++) {
        *ptr1 = *ptr2 * 2;         /* Potential aliasing */
        ptr1++;
        ptr2++;
        sum += *ptr1;
    }
    
    return sum;
}

/* 4. Test Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Loop with internal branching */
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        if (arr[i] > 0) {
            arr[i] = arr[i] * 2 + g_volatile;
        } else {
            arr[i] = arr[i] / 2 - g_volatile;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            sum += arr[i] * 3;
        } else if (i % 3 == 1) {
            sum += arr[i] * 5;
        } else {
            sum += arr[i] * 7;
        }
        
        /* Conditional with loop-carried dependency */
        if (i > 0 && arr[i-1] < 100) {
            arr[i] = arr[i] + arr[i-1];
        }
    }
    
    return sum;
}

/* 5. Test Mixed Data Types and Dependencies */
int test_mixed_dependencies(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    /* Mix float and int operations */
    for (int i = 1; i < n; i++) {
        /* Float RAW */
        farr[i] = farr[i-1] * 1.5f + (float)g_volatile;
        
        /* Int to float conversion dependencies */
        float temp = (float)iarr[i];
        farr[i] += temp / 2.0f;
        
        /* Float to int conversion */
        iarr[i] = (int)farr[i] + iarr[i-1];
        
        /* Mixed-type accumulation */
        fsum += farr[i];
        isum += iarr[i];
    }
    
    return isum + (int)fsum;
}

/* 6. Test Complex Loop-Carried Dependencies */
int test_loop_carried_deps(int *arr, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1, 2, and 3 dependencies */
        arr[i] = arr[i-1] + arr[i-2] + arr[i-3] + arr[i-4];
        
        /* Recurrence with computation */
        int t = arr[i] * 2;
        t = t - arr[i-2];          /* Distance 2 */
        arr[i] = t + g_global_array[i % 1024];
        
        /* Multi-statement recurrence chain */
        int a = arr[i-1] + 1;      /* Distance 1 */
        int b = a * arr[i-2];      /* Distance 2 */
        int c = b - arr[i-3];      /* Distance 3 */
        arr[i] = c + i;
        
        sum += arr[i];
    }
    
    /* Nested loop with dependencies */
    for (int i = 0; i < n/2; i++) {
        for (int j = 1; j < 8; j++) {
            arr[i] += arr[i] * j + g_volatile;
            sum += arr[i] % 256;
        }
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    const int N = 1000;
    int total_sum = 0;
    
    /* Initialize data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    float *farr = (float*)malloc(N * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        farr[i] = (float)(rand() % 100) / 3.0f;
        g_global_array[i % 1024] = rand() % 256;
    }
    
    /* Run all test patterns to trigger different DDG edge types */
    total_sum += test_raw_dependencies(arr1, N);
    total_sum += test_war_waw_dependencies(arr2, N);
    total_sum += test_memory_aliasing(arr1, arr2, N);
    total_sum += test_control_dependencies(arr1, N);
    total_sum += test_mixed_dependencies(farr, arr1, N);
    total_sum += test_loop_carried_deps(arr2, N);
    
    /* Add some volatile accesses to prevent dead code elimination */
    g_volatile = total_sum % 1000;
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += arr1[i] + arr2[i] + (int)farr[i];
    }
    final_result += total_sum;
    
    printf("Result: %d\n", final_result);
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return final_result != 0 ? 0 : 1;
}
