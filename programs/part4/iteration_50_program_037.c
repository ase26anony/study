/* test_ddg_coverage.c
 * Designed to trigger create_ddg_edge() logic in GCC's DDG module
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
float g_float_array[1024];
int* g_ptr1;
int* g_ptr2;

/* Function to prevent dead code elimination */
static int use_result(int x) {
    return g_volatile + x;
}

/* 1. Loop with true data dependencies (RAW/flow) and loop-carried dependencies */
int test_raw_dep(int* arr, int n, int stride) {
    int sum = 0;
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 flow dependency */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Distance 2 flow dependency */
        int temp = arr[i-2] * 3;
        
        /* Mixed integer/float operations for different data types */
        g_float_array[i] = g_float_array[i-1] * 1.5f + temp;
        
        /* Complex chain with multiple uses */
        sum += arr[i] + (int)g_float_array[i];
        
        /* Another flow dependency with different latency */
        g_array[i] = g_array[i-1] + arr[i] * 2;
    }
    return use_result(sum);
}

/* 2. Loop with anti-dependencies (WAR) and output dependencies (WAW) */
int test_war_waw_dep(float* farr, int* iarr, int n) {
    float acc = 0.0f;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): Read farr[i], then write to it */
        float old_val = farr[i];  /* Read */
        farr[i] = old_val * 2.0f + iarr[i];  /* Write - creates WAR */
        
        /* WAW (output dependency): Multiple writes to same location */
        iarr[i] = i * 3;          /* First write */
        iarr[i] = iarr[i] + 5;    /* Second write - creates WAW */
        
        /* Another WAR with different data type */
        int temp = iarr[i-1];     /* Read */
        iarr[i-1] = temp / 2;     /* Write */
        
        /* Complex WAW with memory */
        g_array[i] = old_val;
        g_array[i] = iarr[i] + (int)old_val;
        
        acc += farr[i] + iarr[i];
    }
    
    return use_result((int)acc);
}

/* 3. Loop with memory aliasing dependencies */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    int sum = 0;
    
    /* Create ambiguous pointer aliasing */
    int* p = arr1;
    int* q = arr2;
    
    /* Force compiler to assume possible aliasing */
    for (int i = 0; i < n; i++) {
        /* Memory writes that may alias */
        *p = i * 2;
        *q = *p + 1;  /* Could be RAW if p == q */
        
        /* Pointer arithmetic that creates uncertainty */
        p = &arr1[(i + 1) % n];
        q = &arr2[(i + g_volatile) % n];
        
        /* Array accesses with non-linear indices */
        arr1[i * 2 % n] = arr2[i * 3 % n] + 1;
        
        /* Volatile access creates hard memory dependency */
        arr2[i] = g_volatile;
        
        sum += arr1[i] + arr2[i];
    }
    
    return use_result(sum);
}

/* 4. Loop with control dependencies */
int test_control_dep(int* arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple conditional branches inside loop */
        if (arr[i] > threshold) {
            arr[i] = arr[i] * 2;
            count++;
        } else if (arr[i] < -threshold) {
            arr[i] = arr[i] / 2;
            count--;
        } else {
            arr[i] = 0;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            g_array[i] = arr[i] + 1;
            if (g_volatile > 0) {
                arr[i] = g_array[i] * 3;
            }
        }
        
        /* Conditional with loop-carried dependency */
        if (i > 1 && arr[i-2] > 0) {
            arr[i] = arr[i] + arr[i-2];
        }
    }
    
    return use_result(count);
}

/* 5. Complex nested loop with mixed dependencies */
int test_nested_mixed(float* farr, int* iarr, int n) {
    float total = 0.0f;
    
    for (int i = 1; i < n; i++) {
        float inner_acc = 0.0f;
        
        /* Inner loop with dependencies */
        for (int j = 0; j < 8; j++) {
            /* RAW in inner loop */
            farr[j] = farr[j] + iarr[j] * 0.5f;
            
            /* WAR in inner loop */
            int temp = iarr[j];
            iarr[j] = temp + j;
            
            /* Control dependency in inner loop */
            if (j % 2 == 0) {
                inner_acc += farr[j];
            } else {
                inner_acc -= farr[j];
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        g_float_array[i] = g_float_array[i-1] + inner_acc;
        
        /* Mixed-type operation */
        iarr[i] = (int)g_float_array[i] + iarr[i-1];
        
        total += inner_acc + iarr[i];
    }
    
    return use_result((int)total);
}

/* 6. Loop with function calls (act as memory clobbers) */
static int helper_func(int x, int y) {
    /* Access globals to create memory dependencies */
    g_volatile = x + y;
    return g_array[x % 1024] + y;
}

int test_func_call_dep(int* arr, int n) {
    int result = 0;
    
    for (int i = 1; i < n; i++) {
        /* Function call creates memory barrier effect */
        int val = helper_func(arr[i-1], i);
        
        /* Dependency through function return value */
        arr[i] = arr[i] + val;
        
        /* Another call with different args */
        val = helper_func(i, arr[i]);
        
        /* Use result to prevent elimination */
        result += val;
        
        /* Memory dependency through global */
        g_array[i] = g_volatile + arr[i];
    }
    
    return use_result(result);
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize data */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i % 100;
        arr2[i] = (i * 3) % 100;
        farr[i] = i * 0.5f;
        g_array[i % 1024] = i;
        g_float_array[i % 1024] = i * 0.25f;
    }
    
    g_ptr1 = arr1;
    g_ptr2 = arr2;
    
    /* Run all test patterns to create various DDG edges */
    int checksum = 0;
    
    checksum += test_raw_dep(arr1, n, 2);
    checksum += test_war_waw_dep(farr, arr2, n);
    checksum += test_memory_aliasing(arr1, arr2, n);
    checksum += test_control_dep(arr1, n, 50);
    checksum += test_nested_mixed(farr, arr2, n/2);
    checksum += test_func_call_dep(arr1, n);
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    /* Final volatile access */
    g_volatile = checksum;
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return checksum != 0 ? 0 : 1;
}
