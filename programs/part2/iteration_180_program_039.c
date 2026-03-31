/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o test_delay test_delay.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and prevent optimization */
int fill_delay_slots_test(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent constant folding */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    int result = 0;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    
    /* Loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a runtime-dependent condition that's not always true/false */
        /* Using modulo with prime number to prevent pattern recognition */
        if ((i % 7) == 0) {
            /* This should become a simple conditional jump to label */
            goto delay_target;
        }
        
        /* Some computation to use variables and prevent dead code elimination */
        a = b + c;
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        /* Skip the target code when not jumping */
        continue;
        
delay_target:
        /* TARGET INSTRUCTION: This should be the candidate for delay slot filling */
        /* Simple, safe arithmetic operation that doesn't trap */
        /* Uses different variables than the jump condition (i) */
        a = b + c;  /* Simple add - no trapping possible */
        
        /* Additional operations to ensure target isn't isolated */
        b = c * d;
        c = d - a;
        d = a ^ b;
        
        /* Continue loop */
    }
    
    /* Use all variables to create observable side effects */
    result = a + b + c + d;
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return result;
}

/* Second test case with different pattern */
int another_test_case(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 5;
    volatile int s = 6;
    
    /* Nested loop for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Another conditional jump pattern */
            if ((i * j) % 11 == 0) {
                goto another_target;
            }
            
            p = q + r;
            q = r * s;
            r = s - p;
            s = p ^ q;
            continue;
            
another_target:
            /* Another candidate instruction for delay slot */
            p = q + r;  /* Safe arithmetic operation */
            
            /* Follow-up operations */
            q = r * s;
            r = s - p;
            s = p ^ q;
        }
    }
    
    return p + q + r + s;
}

/* Third test with pointer operations (safe) */
int pointer_safe_test(int *ptr1, int *ptr2) {
    volatile int temp;
    
    /* Ensure pointers are valid (non-null) */
    if (!ptr1 || !ptr2) return -1;
    
    /* Initialize some values */
    *ptr1 = 10;
    *ptr2 = 20;
    
    for (int i = 0; i < 100; i++) {
        if (i % 13 == 0) {
            goto ptr_target;
        }
        
        temp = *ptr1 + *ptr2;
        *ptr1 = *ptr2 - temp;
        *ptr2 = temp ^ i;
        continue;
        
ptr_target:
        /* Safe memory operation - pointers are guaranteed non-null */
        temp = *ptr1 + *ptr2;  /* Candidate for delay slot */
        
        *ptr1 = *ptr2 - temp;
        *ptr2 = temp ^ i;
    }
    
    return *ptr1 + *ptr2;
}

int main(int argc, char **argv) {
    int result1, result2, result3;
    int array1[2], array2[2];
    
    /* Run first test */
    result1 = fill_delay_slots_test(argc, argv);
    
    /* Run second test with runtime-dependent inputs */
    result2 = another_test_case(argc, argv[0] ? argv[0][0] : 'A');
    
    /* Run third test with safe pointers */
    result3 = pointer_safe_test(&array1[0], &array2[0]);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    printf("Final result: %d\n", final_result);
    
    /* Return non-constant result to prevent optimization */
    return (final_result > 100) ? 0 : 1;
}
