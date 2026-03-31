#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile variables to prevent constant propagation */
static volatile int global_counter;
static volatile int global_result;

/* Function prototypes */
NOINLINE void loop_decrement_for(int n);
NOINLINE void loop_decrement_while_predec(int n);
NOINLINE void loop_decrement_while_postdec(int n);
NOINLINE void loop_decrement_do_while(int n);
NOINLINE void loop_decrement_complex(int n);

/* Variant 1: Classic for loop with i != 0 condition */
NOINLINE void loop_decrement_for(int n) {
    volatile int local_sum = 0;
    int i;
    
    /* Counter in register, decrement by 1, exit when i != 0 */
    for (i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        local_sum += (i * 2) & 0xFF;
    }
    
    global_result ^= local_sum;
}

/* Variant 2: While loop with pre-decrement */
NOINLINE void loop_decrement_while_predec(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    
    /* Counter decremented before comparison */
    while (--cnt != 0) {
        local_sum += (cnt * 3) & 0xFF;
    }
    
    global_result ^= local_sum;
}

/* Variant 3: While loop with post-decrement */
NOINLINE void loop_decrement_while_postdec(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    
    /* Counter decremented after comparison */
    while (cnt-- != 0) {
        local_sum += (cnt * 5) & 0xFF;
    }
    
    global_result ^= local_sum;
}

/* Variant 4: Do-while loop with explicit check */
NOINLINE void loop_decrement_do_while(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    
    if (cnt > 0) {
        do {
            local_sum += (cnt * 7) & 0xFF;
        } while (--cnt != 0);
    }
    
    global_result ^= local_sum;
}

/* Variant 5: More complex but still matching pattern */
NOINLINE void loop_decrement_complex(int n) {
    volatile int local_sum = 0;
    int counter = n;
    int temp;
    
    while (1) {
        temp = counter - 1;
        if (temp == 0) break;
        counter = temp;
        local_sum += (counter * 11) & 0xFF;
    }
    
    global_result ^= local_sum;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        global_counter = 1000;
        loop_bound = global_counter;
    }
    
    /* Ensure bound is reasonable */
    if (loop_bound <= 0) loop_bound = 1000;
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Reset global result */
    global_result = 0;
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_predec(loop_bound);
    loop_decrement_while_postdec(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_complex(loop_bound);
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", global_result);
    
    return 0;
}
