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
int test_nested_loop_dependencies(int *arr, int n, int m);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Distance 2 RAW dependency */
        arr[i] += arr[i-2] * 2;
        
        /* Floating point RAW dependency */
        float temp = (float)arr[i] / 3.14f;
        arr[i] = (int)(temp * 2.0f);
        
        sum += arr[i];
    }
    
    /* Additional RAW chain with pointer arithmetic */
    int *ptr = arr;
    for (int i = 0; i < n-1; i++) {
        *(ptr + 1) = *ptr + i;
        ptr++;
        sum += *ptr;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr, int n) {
    int sum = 0;
    int temp1, temp2;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read after write */
        temp1 = arr[i] + i;      /* Read arr[i] */
        arr[i] = temp1 * 2;      /* Write arr[i] - anti-dependent on previous read */
        
        /* WAW (output-dependency): write after write */
        arr[i] = temp1 + 1;      /* First write */
        arr[i] = arr[i] * 3;     /* Second write to same location - output dependency */
        
        /* More complex WAR/WAW mixing */
        temp2 = arr[i];
        arr[i] = (temp1 + temp2) / 2;
        arr[i] = arr[i] + g_volatile;  /* Another WAW */
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    /* Create potential aliasing */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Force compiler to assume aliasing */
    for (int i = 0; i < n; i++) {
        /* Access through different pointers that might alias */
        *ptr1 = *ptr2 + i;
        *ptr2 = *ptr1 * 2;
        
        /* Array access with variable indices - hard to analyze */
        int idx1 = i % (n/2);
        int idx2 = (i * 7) % (n/2);
        arr1[idx1] = arr2[idx2] + g_volatile;
        arr2[idx2] = arr1[idx1] - i;
        
        /* Pointer arithmetic that could cause overlap */
        if (i < n-10) {
            *(ptr1 + 5) = *(ptr2 + 3) + 1;
        }
        
        ptr1++;
        ptr2++;
        sum += *ptr1 + *ptr2;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > 0) {
            /* Branch 1 - complex operations */
            arr[i] = arr[i] * 2 + 1;
            sum += arr[i] / 3;
        } else {
            /* Branch 2 - different operations */
            arr[i] = arr[i] - 5;
            sum += arr[i] * 2;
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            arr[i] += g_volatile;
            if (arr[i] % 2 == 0) {
                arr[i] *= 2;
            } else {
                arr[i] /= 2;
            }
        }
        
        /* Switch-like control flow */
        switch (i % 4) {
            case 0: arr[i] += 1; break;
            case 1: arr[i] += 2; break;
            case 2: arr[i] += 3; break;
            case 3: arr[i] += 4; break;
        }
    }
    
    return sum;
}

/* Test 5: Mixed Dependencies */
int test_mixed_dependencies(int *arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 2; i < n; i++) {
        /* RAW with floating point */
        float f1 = (float)arr[i-1];
        float f2 = f1 * 3.14f;
        arr[i] = (int)f2;
        
        /* WAR with integer */
        int temp = arr[i];
        arr[i] = temp + i;
        temp = arr[i] * 2;  /* WAR: reading arr[i] after writing it */
        
        /* Memory dependency with volatile */
        arr[i] = g_volatile + temp;
        
        /* Control dependency */
        if (arr[i] % 2 == 0) {
            arr[i] += 1;
        } else {
            arr[i] -= 1;
        }
        
        /* Another RAW chain */
        arr[i] = arr[i-2] + arr[i-1];
        
        sum += arr[i];
        fsum += (float)arr[i];
    }
    
    return sum + (int)fsum;
}

/* Test 6: Nested Loops with Dependencies */
int test_nested_loop_dependencies(int *arr, int n, int m) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Outer loop carried dependency */
        arr[i] = arr[i-1] + 1;
        
        for (int j = 1; j < m; j++) {
            /* Inner loop dependencies */
            int idx = i * m + j;
            
            /* RAW in inner loop */
            g_array[idx] = g_array[idx-1] + arr[i];
            
            /* WAR in inner loop */
            int temp = g_array[idx];
            g_array[idx] = temp * j;
            temp = g_array[idx] + i;  /* WAR */
            
            /* Control dependency in inner loop */
            if (j % 3 == 0) {
                g_array[idx] += g_volatile;
            }
            
            sum += g_array[idx];
        }
        
        /* Cross-iteration dependency between inner and outer loops */
        arr[i] = g_array[i * m] + sum % 100;
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    const int N = 1000;
    const int M = 50;
    int total_sum = 0;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < 1024; i++) {
        g_array[i] = rand() % 100;
    }
    
    /* Dynamic allocation to prevent static analysis */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    
    if (!arr1 || !arr2) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
    
    /* Update volatile to prevent dead code elimination */
    g_volatile = rand() % 10;
    
    printf("Running DDG edge creation tests...\n");
    
    /* Run all tests to trigger different DDG edge types */
    total_sum += test_raw_dependencies(arr1, N);
    total_sum += test_war_waw_dependencies(arr2, N);
    total_sum += test_memory_aliasing(arr1, arr2, N);
    total_sum += test_control_dependencies(arr1, N);
    total_sum += test_mixed_dependencies(arr2, N);
    total_sum += test_nested_loop_dependencies(arr1, N/10, M);
    
    /* Use results to prevent optimization */
    g_result = total_sum;
    printf("Total checksum: %d\n", total_sum % 1000);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return g_result == 0 ? 0 : 0;  /* Always return 0, but compiler doesn't know */
}
