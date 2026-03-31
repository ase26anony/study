/* test_ddg_edges.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(int *arr, int n);
int test_loop_carried_dependencies(int *arr, int n);
int test_complex_nested_loop(int *arr, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    volatile int barrier = g_volatile;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        arr[i] = arr[i-1] + i;
        
        /* Distance 2 RAW dependency */
        arr[i] += arr[i-2] * 2;
        
        /* Floating point RAW dependency */
        float temp = (float)arr[i] / 3.14f;
        arr[i] = (int)(temp * 2.0f);
        
        sum += arr[i];
    }
    
    /* Prevent dead code elimination */
    return sum + barrier;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr, int n) {
    int temp1, temp2;
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): read after write */
        temp1 = arr[i];          /* Read arr[i] */
        arr[i] = i * 2;          /* Write arr[i] - creates WAR with next iteration */
        
        /* WAW (output-dependency): write after write */
        arr[i] = temp1 + 1;      /* Second write to arr[i] */
        
        /* More complex WAR/WAW pattern */
        temp2 = arr[i-1];
        arr[i-1] = temp1 * 3;
        arr[i] = temp2 + arr[i-1];
        
        sum += arr[i] + arr[i-1];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    volatile int *volatile_ptr = &g_volatile;
    
    /* Create ambiguous pointer aliasing */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Force compiler to assume aliasing */
    for (int i = 0; i < n; i++) {
        /* May-alias accesses */
        *ptr1 = i * 3;
        *ptr2 = *ptr1 + 1;
        
        /* Pointer arithmetic that could cause overlap */
        ptr1 = &arr1[(i + 1) % n];
        ptr2 = &arr2[(i + 2) % n];
        
        /* Volatile access creates hard dependency */
        sum += *volatile_ptr;
        
        /* Array access with variable index - hard to analyze */
        arr1[i % n] = arr2[(i + 1) % n] + sum;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    volatile int cond = g_volatile;
    
    for (int i = 0; i < n; i++) {
        /* Multiple conditional branches inside loop */
        if (i % 3 == 0) {
            arr[i] = i * 2;
            if (cond > 0) {
                arr[i] += 5;
            }
        } else if (i % 3 == 1) {
            arr[i] = i * 3;
            /* Nested condition */
            if (arr[i] > 100) {
                arr[i] = 100;
            }
        } else {
            arr[i] = i;
        }
        
        /* Another condition with computation */
        int temp = (arr[i] > 50) ? arr[i] / 2 : arr[i] * 2;
        
        /* Switch-like control flow */
        switch (i % 4) {
            case 0: temp += 1; break;
            case 1: temp += 2; break;
            case 2: temp += 3; break;
            case 3: temp += 4; break;
        }
        
        sum += temp;
    }
    
    return sum;
}

/* Test 5: Mixed Dependencies */
int test_mixed_dependencies(int *arr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 2; i < n; i++) {
        /* RAW with floating point */
        float f1 = (float)arr[i-1];
        float f2 = f1 * 1.5f;
        
        /* WAR with integer */
        int temp = arr[i];
        arr[i] = (int)f2;
        
        /* WAW */
        arr[i] = temp + arr[i];
        
        /* Control dependency */
        if (arr[i] % 2 == 0) {
            f2 += 3.14f;
        }
        
        /* Memory dependency with global */
        g_array[i % 1024] = arr[i];
        
        /* Loop-carried dependency with distance 3 */
        if (i >= 3) {
            arr[i] += arr[i-3];
        }
        
        fsum += f2;
        isum += arr[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Loop-Carried Dependencies with Different Distances */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies */
    for (int i = 4; i < n; i++) {
        /* Distance 1 */
        arr[i] = arr[i-1] + 1;
        
        /* Distance 2 */
        arr[i] += arr[i-2] * 2;
        
        /* Distance 3 with floating point */
        float ftemp = (float)arr[i-3];
        arr[i] += (int)(ftemp * 1.5f);
        
        /* Distance 4 through memory */
        g_array[i % 1024] = arr[i-4] + i;
        arr[i] += g_array[(i-1) % 1024];
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 7: Complex Nested Loop Structure */
int test_complex_nested_loop(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Outer loop computation */
        int outer_val = arr[i] * 2;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < 8; j++) {
            /* RAW in inner loop */
            int inner_temp = outer_val + j;
            
            /* WAR in inner loop */
            outer_val = inner_temp * 3;
            
            /* Memory access with potential aliasing */
            g_array[(i + j) % 1024] = outer_val;
            
            /* Control in inner loop */
            if (j % 2 == 0) {
                inner_temp += g_array[(i + j - 1) % 1024];
            }
            
            sum += inner_temp;
        }
        
        /* Loop-carried dependency in outer loop */
        if (i > 0) {
            arr[i] = arr[i-1] + sum % 100;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int n = 1000;
    int result = 0;
    
    /* Initialize with random data to prevent compile-time computation */
    srand(time(NULL));
    for (int i = 0; i < 1024; i++) {
        g_array[i] = rand() % 100;
    }
    
    /* Dynamic iteration count prevents unrolling */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with dynamic size */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
    
    printf("Testing DDG edge creation with n = %d\n", n);
    
    /* Run all tests to create various DDG edge types */
    result += test_raw_dependencies(arr1, n);
    result += test_war_waw_dependencies(arr2, n);
    result += test_memory_aliasing(arr1, arr2, n);
    result += test_control_dependencies(arr1, n);
    result += test_mixed_dependencies(arr2, n);
    result += test_loop_carried_dependencies(arr1, n);
    result += test_complex_nested_loop(arr2, n);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Store to global to ensure side effects */
    g_result = result;
    
    free(arr1);
    free(arr2);
    
    return result != 0 ? 0 : 1;
}
