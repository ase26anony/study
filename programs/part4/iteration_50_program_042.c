/* test_ddg_edges.c
 * Program to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>

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
int test_loop_carried_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW) */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Loop with multiple RAW dependencies */
    for (int i = 2; i < n; i++) {
        /* Flow dependency chain: i-2 -> i-1 -> i */
        arr[i] = arr[i-1] + arr[i-2] + g_volatile;
        
        /* Another flow dependency within same iteration */
        int temp = arr[i] * 3;
        sum += temp / 2;
        
        /* Cross-iteration dependency with distance > 0 */
        if (i > 3) {
            arr[i-2] = arr[i-3] + 1;  /* distance = 1 */
        }
    }
    
    /* Prevent dead code elimination */
    g_result ^= sum;
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *arr, int n) {
    int sum = 0;
    int temp_reg;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): read arr[i], then write to it */
        temp_reg = arr[i] + i;      /* Read arr[i] */
        arr[i] = temp_reg * 2;      /* Write arr[i] - anti-dependency */
        
        /* WAW (output-dependency): multiple writes to same location */
        arr[i] = temp_reg + 1;      /* First write */
        if (i % 3 == 0) {
            arr[i] = temp_reg + 2;  /* Second write to same location */
        }
        
        /* Register pressure to force spills and create more edges */
        int r1 = arr[i-1];
        int r2 = r1 * r1;
        int r3 = r2 + i;
        int r4 = r3 / 7;
        sum += r4;
    }
    
    g_result ^= sum;
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *p = arr1;
    int *q = arr2;
    
    /* Compiler doesn't know if p and q alias */
    for (int i = 0; i < n; i++) {
        *p = *q + 1;        /* Read from q, write to p */
        p++;
        q++;
        
        /* Array access with variable index - may alias */
        arr1[i % 10] = arr2[(i + 5) % 10] + g_volatile;
        
        sum += arr1[i];
    }
    
    g_result ^= sum;
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (arr[i] > 0) {
            /* Instructions dependent on the branch */
            int temp = arr[i] * 2;
            sum += temp;
            
            /* Nested control flow */
            if (temp > 100) {
                arr[i] = temp / 3;
            } else {
                arr[i] = temp + 7;
            }
        } else {
            /* Alternative path with different dependencies */
            arr[i] = -arr[i] + g_volatile;
            sum -= arr[i];
        }
        
        /* Loop with internal branch that affects scheduling */
        switch (i % 4) {
            case 0: arr[i] += 1; break;
            case 1: arr[i] += 2; break;
            case 2: arr[i] += 3; break;
            case 3: arr[i] += 4; break;
        }
    }
    
    g_result ^= sum;
    return sum;
}

/* Test 5: Mixed Data Types (int and float) */
int test_mixed_dependencies(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    
    for (int i = 1; i < n; i++) {
        /* Mixed int/float operations create different edge types */
        farr[i] = farr[i-1] * 1.5f + (float)iarr[i];
        
        /* Convert and create dependencies */
        int int_val = (int)farr[i];
        iarr[i] = int_val + iarr[i-1];
        
        /* Floating point control dependency */
        if (farr[i] > 100.0f) {
            farr[i] = farr[i] / 2.0f;
        }
        
        fsum += farr[i];
    }
    
    g_result ^= (int)fsum;
    return (int)fsum;
}

/* Test 6: Loop-Carried Dependencies with Distance > 0 */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 4; i < n; i++) {
        /* Distance = 1 */
        arr[i] = arr[i-1] + 1;
        
        /* Distance = 2 */
        int temp1 = arr[i-2] * 2;
        
        /* Distance = 3 */
        if (i > 6) {
            arr[i-3] = temp1 + arr[i-4];  /* distance = 3 */
        }
        
        /* Distance = 4 with anti-dependency */
        int temp2 = arr[i-4];             /* read arr[i-4] */
        arr[i] = arr[i] + temp2;          /* write arr[i] */
        if (i % 5 == 0) {
            arr[i-4] = temp2 + 1;         /* write arr[i-4] - WAR with distance 4 */
        }
        
        sum += arr[i];
    }
    
    g_result ^= sum;
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)malloc(n * sizeof(int) + 32);
    int *arr2 = (int*)malloc(n * sizeof(int) + 32);
    float *farr = (float*)malloc(n * sizeof(float) + 32);
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 113;
        farr[i] = (float)(i % 79) * 1.7f;
    }
    
    int total_sum = 0;
    
    /* Run all tests to create various DDG edge types */
    total_sum += test_raw_dependencies(arr1, n);
    total_sum += test_war_waw_dependencies(arr2, n);
    total_sum += test_memory_aliasing(arr1, arr2, n);
    total_sum += test_control_dependencies(arr1, n);
    total_sum += test_mixed_dependencies(farr, arr1, n);
    total_sum += test_loop_carried_dependencies(arr2, n);
    
    /* Use volatile to prevent optimization of final result */
    g_volatile = total_sum % 256;
    
    printf("Result checksum: %d (g_result: %d)\n", total_sum, g_result);
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return total_sum != 0 ? 0 : 1;
}
