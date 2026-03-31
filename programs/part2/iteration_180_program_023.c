/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_reorg test_reorg.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -o test_reorg test_reorg.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slot_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent constant folding */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    volatile int e = 5;
    volatile int f = 6;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    int result = 0;
    
    /* Main loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a conditional check based on runtime value */
        /* The condition should not be always true/false to prevent optimization */
        if ((i + (int)(argv[0][0])) % 7 == 0) {
            /* This goto creates a simple conditional jump to a label */
            /* The compiler should generate a simplejump_p instruction */
            goto target_label;
        }
        
        /* Some other computations to prevent the block from being trivial */
        d = e * f;
        e = f + 1;
        continue;
        
        /* Target label with a simple, safe instruction */
        /* This instruction should be eligible for delay slot filling */
        target_label:
        /* Simple arithmetic that doesn't trap and uses different variables */
        /* Avoids CC, stack pointer, and other critical resources */
        a = b + c;  /* Candidate for delay slot filling */
        
        /* Continue with other operations after the label */
        b = c * d;
        c = d + a;
        
        /* Additional computation to use variables and prevent dead code elimination */
        result += a + b + c;
    }
    
    /* Use the variables to create observable side effects */
    printf("Result: %d, a=%d, b=%d, c=%d\n", result, a, b, c);
    
    /* Return value based on computations to prevent optimization */
    return result & 0xFF;
}

/* Second test function with different pattern */
int alternative_test(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 10;
    volatile int s = 20;
    int sum = 0;
    
    /* Loop with multiple conditional jumps */
    for (int i = 0; i < 50; i++) {
        /* Different condition to create various jump patterns */
        if ((p + i) & 1) {
            goto alt_label1;
        }
        
        if ((q - i) % 3 == 0) {
            goto alt_label2;
        }
        
        r = s + i;
        continue;
        
        alt_label1:
        /* Simple assignment with different variables */
        p = q + r;  /* Potential delay slot candidate */
        s = p * 2;
        sum += p;
        continue;
        
        alt_label2:
        /* Another simple assignment */
        q = r + s;  /* Potential delay slot candidate */
        p = q / 2;  /* Division is safe here since q won't be odd when we reach here */
        sum += q;
    }
    
    return sum;
}

/* Third test with pointer operations (safe) */
int pointer_test(int *arr, int size) {
    volatile int temp = 0;
    volatile int idx = 0;
    int total = 0;
    
    if (size <= 0) return 0;
    
    /* Ensure arr is not NULL to avoid trapping */
    if (arr == NULL) return 0;
    
    for (int i = 0; i < size && i < 100; i++) {
        /* Conditional jump based on array value */
        if (arr[i] > 100) {
            goto ptr_label;
        }
        
        temp = arr[i] * 2;
        idx = i + 1;
        continue;
        
        ptr_label:
        /* Safe memory store - using known valid address */
        arr[i] = temp + idx;  /* Potential delay slot candidate */
        total += arr[i];
    }
    
    return total;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int test_array[10] = {1, 150, 3, 200, 5, 75, 7, 250, 9, 100};
    
    /* Call first test function */
    result1 = fill_delay_slot_test(argc, argv);
    
    /* Call second test function with runtime-dependent values */
    result2 = alternative_test(argc, (int)(argv[0][0]));
    
    /* Call third test function */
    result3 = pointer_test(test_array, 10);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    printf("Final result: %d\n", final_result);
    
    /* Return non-zero to indicate execution */
    return (final_result != 0) ? 0 : 1;
}
