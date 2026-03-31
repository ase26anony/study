/* Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fno-tree-vectorize -fno-unroll-loops -o modulo_test modulo_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void compute_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimization */
    
    /* Outer loop to increase scheduling analysis opportunities */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        /* This creates a recurrence cycle challenging for the scheduler */
        for (int i = 0; i < 32; i++) {
            /* Multiple arithmetic operations with carried dependency */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 256);
            
            /* Use volatile to prevent dead code elimination */
            temp = sum;
        }
        
        /* Modify input slightly for outer loop variation */
        b[0] += sum;
        a[outer % 32] = sum & 0xFF;
    }
    
    /* Print result to prevent elimination */
    printf("Final sum: %d\n", sum);
}

/* Another function with different recurrence pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void compute_loop2(int *arr, int n) {
    int acc1 = 5, acc2 = 3;
    
    for (int outer = 0; outer < 5; outer++) {
        /* Loop with multiple interleaved dependencies */
        for (int i = 1; i < 64; i++) {
            /* Two separate recurrence chains */
            acc1 = (acc1 * 3 + arr[i]) % 1000;
            acc2 = (acc2 + acc1 * arr[i-1]) >> 1;
            
            /* Cross-dependency between chains */
            arr[i] = (acc1 + acc2) & 0xFF;
        }
        
        /* Feedback to create outer loop dependency */
        arr[0] = (acc1 + acc2) % 256;
    }
    
    printf("Acc1: %d, Acc2: %d\n", acc1, acc2);
}

/* Function with pointer-based recurrence */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void pointer_recurrence(int *data, int size) {
    int *ptr = data;
    int sum = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            /* Pointer arithmetic with dependency */
            sum += *ptr * (sum + 1);
            ptr++;
            
            /* Additional computation */
            sum = (sum << 3) | (sum >> 29);  /* Rotate */
            sum = sum ^ data[j];
        }
        ptr = data;  /* Reset pointer */
    }
    
    printf("Pointer sum: %d\n", sum);
}

int main() {
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values */
    int a[128], b[128];
    for (int i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call multiple loop functions to increase coverage chances */
    compute_loop(a, b, 128);
    
    int arr[128];
    for (int i = 0; i < 128; i++) {
        arr[i] = rand() % 256;
    }
    compute_loop2(arr, 128);
    
    int data[128];
    for (int i = 0; i < 128; i++) {
        data[i] = rand() % 256;
    }
    pointer_recurrence(data, 128);
    
    return 0;
}
