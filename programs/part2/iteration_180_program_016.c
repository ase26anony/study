/* Compile with: gcc -O2 -fno-guess-branch-probability -fno-if-conversion -o delay_slot_test delay_slot_test.c */
/* For MIPS: gcc -O3 -march=mips64 -mtune=mips64 -fno-guess-branch-probability -o delay_slot_test delay_slot_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Function to create runtime-dependent values and control flow */
int test_delay_slot_filling(int argc, char **argv) {
    /* Declare and initialize variables - use volatile to prevent optimization */
    volatile int a = 1;
    volatile int b = 2;
    volatile int c = 3;
    volatile int d = 4;
    volatile int e = 5;
    volatile int f = 6;
    
    /* Use argc to create runtime-dependent loop bounds */
    int iterations = (argc > 1) ? 100 : 200;
    int result = 0;
    
    /* Complex loop to provide scheduling context */
    for (int i = 0; i < iterations; ++i) {
        /* Create a non-trivial condition that can't be optimized away */
        int condition = (i * 17 + argc) % 13;
        
        /* 
         * KEY CONSTRUCT: Conditional jump with potential delay slot candidate
         * The target instruction (at target_label) should be a simple, safe
         * arithmetic operation that doesn't conflict with jump resources
         */
        if (condition == 0) {
            /* 
             * Use goto to create a simplejump_p in RTL
             * The compiler should see this as a simple conditional jump to label
             */
            goto target_label;
        }
        
        /* Some intermediate computations to separate control flow */
        a = b + c;
        b = c * d;
        c = d ^ e;
        
        /* Skip the target block when not jumping */
        goto continue_loop;
        
    target_label:
        /* 
         * DELAY SLOT CANDIDATE: Simple, safe arithmetic operation
         * - Not a jump
         * - Doesn't set condition codes (CC)
         * - Doesn't use stack pointer
         * - Non-trapping (addition is safe)
         * - Uses different variables than the jump condition
         */
        d = e + f;  /* Simple addition - good candidate for delay slot */
        
        /* Additional operations to ensure this isn't a single-instruction block */
        e = f - a;
        f = a | b;
        
    continue_loop:
        /* More computations to create register pressure */
        result += (a ^ b) + (c & d) - (e | f);
        
        /* Prevent loop unrolling */
        if (i % 5 == 0) {
            a = b;
            b = c;
        }
    }
    
    /* Create observable side effect */
    printf("Result: %d\n", result);
    
    /* Use all variables to prevent dead code elimination */
    return result + a + b + c + d + e + f;
}

/* Second function with different pattern to increase coverage chances */
int alternative_pattern(int x, int y) {
    volatile int p = x;
    volatile int q = y;
    volatile int r = 0;
    volatile int s = 0;
    
    /* Nested loops for more complex control flow */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Another conditional jump pattern */
            if ((i * j + p) % 11 == 0) {
                goto alt_target;
            }
            
            p = q + i;
            q = p - j;
            goto alt_continue;
            
        alt_target:
            /* Another delay slot candidate - register move pattern */
            r = s + 1;  /* Simple increment - good for delay slot */
            s = r * 2;
            
        alt_continue:
            p = (p ^ q) + r;
        }
    }
    
    return p + q + r + s;
}

/* Third pattern with pointer operations (but safe) */
int safe_pointer_pattern(int *ptr1, int *ptr2) {
    volatile int temp1 = *ptr1;
    volatile int temp2 = *ptr2;
    volatile int sum = 0;
    
    /* Ensure pointers are valid to avoid trapping */
    if (ptr1 == NULL || ptr2 == NULL) {
        return 0;
    }
    
    for (int i = 0; i < 30; i++) {
        /* Conditional jump based on array access */
        if ((temp1 + i) % 7 == 0) {
            goto ptr_target;
        }
        
        temp1 = temp2 + i;
        temp2 = temp1 - i;
        goto ptr_continue;
        
    ptr_target:
        /* Safe memory store as delay slot candidate */
        *ptr1 = temp2;  /* Simple store - should be safe if ptr1 is valid */
        sum += temp1;
        
    ptr_continue:
        temp2 = (temp1 ^ temp2) + i;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int result1 = test_delay_slot_filling(argc, argv);
    
    /* Create some test data for other patterns */
    int x = 10, y = 20;
    int result2 = alternative_pattern(x, y);
    
    int data1 = 100, data2 = 200;
    int result3 = safe_pointer_pattern(&data1, &data2);
    
    /* Final result combines all patterns */
    int final_result = result1 + result2 + result3;
    
    printf("Final result: %d\n", final_result);
    printf("Modified data: %d, %d\n", data1, data2);
    
    return final_result % 256;
}
