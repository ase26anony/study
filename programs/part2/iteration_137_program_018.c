/* Compile with: gcc -O2 -fmodulo-sched -fno-tree-vectorize -fno-unroll-loops -fdump-rtl-sms -fdump-rtl-sms-details modulo_test.c -o modulo_test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency - critical for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling problem */
        for (i = 0; i < 32; i++) {
            /* Complex arithmetic with true data dependency chain */
            /* sum depends on previous iteration's sum value */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum += (sum & 0xFF);      /* Use bitwise AND */
            sum ^= (i & 1);           /* XOR with loop index */
        }
        
        /* Modify input slightly for outer loop variation */
        /* Prevents complete optimization of outer loop */
        b[0] += sum & 0xF;
        a[1] ^= sum & 0xFF;
    }
    
    return sum;
}

/* Another function with different dependency pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int compute_loop2(int *arr, int n) {
    int acc = arr[0];
    int i, j;
    
    for (j = 0; j < 5; j++) {
        /* Loop with multiple dependency chains */
        for (i = 1; i < 64; i++) {
            /* Recurrence: current depends on previous */
            int temp = arr[i] * acc;
            acc = (temp + i) & 0xFFFF;
            
            /* Additional parallel operations */
            arr[i-1] = (arr[i-1] + acc) >> 1;
            acc = (acc << 3) | (acc >> 29);  /* Rotate */
        }
        
        /* Break potential optimization */
        arr[0] ^= j;
    }
    
    return acc;
}

int main() {
    int i;
    int result1, result2;
    
    /* Initialize with volatile-like behavior using rand() */
    srand(time(NULL));
    
    /* Arrays with pseudo-random values to prevent constant propagation */
    int a[128], b[128];
    int arr[128];
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
        arr[i] = rand() % 512;
    }
    
    /* Call functions with carried dependencies */
    result1 = compute_loop(a, b, 32);
    result2 = compute_loop2(arr, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional computation to increase optimization opportunities */
    int final = result1 + result2;
    for (i = 0; i < 16; i++) {
        final = (final * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final: %d\n", final);
    
    return 0;
}
