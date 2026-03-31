#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent constant propagation */
volatile int global_counter;

/* Function to get non-constant loop bound */
static int get_bound(void) {
    return global_counter;
}

/* Variant 1: for loop with i-- and != 0 condition */
NOOPT void loop_decrement_for(int bound, int *result) {
    int sum = 0;
    for (int i = bound; i != 0; i--) {
        sum += i * 3;  /* Side effect depending on counter */
    }
    *result = sum;
}

/* Variant 2: while loop with pre-decrement and != 0 condition */
NOOPT void loop_decrement_while_pre(int bound, int *result) {
    int sum = 0;
    int cnt = bound;
    while (--cnt != 0) {
        sum += cnt * 5;
    }
    *result = sum;
}

/* Variant 3: while loop with post-decrement and != 0 condition */
NOOPT void loop_decrement_while_post(int bound, int *result) {
    int sum = 0;
    int cnt = bound;
    while (cnt-- != 0) {
        sum += (cnt + 1) * 7;
    }
    *result = sum;
}

/* Variant 4: do-while with explicit decrement and != 0 check */
NOOPT void loop_decrement_dowhile(int bound, int *result) {
    int sum = 0;
    int cnt = bound;
    if (cnt > 0) {
        do {
            sum += cnt * 11;
            cnt--;
        } while (cnt != 0);
    }
    *result = sum;
}

/* Variant 5: for loop with unsigned counter (common pattern) */
NOOPT void loop_decrement_unsigned(unsigned int bound, int *result) {
    unsigned int sum = 0;
    for (unsigned int i = bound; i != 0; i--) {
        sum += i * 13;
    }
    *result = (int)sum;
}

/* Variant 6: Complex decrement pattern to force register usage */
NOOPT void loop_decrement_complex(int bound, int *result) {
    int sum = 0;
    int reg_counter = bound;  /* Explicitly named to hint register */
    
    /* Force counter into register with arithmetic */
    reg_counter = reg_counter + 0;
    
    while (reg_counter != 0) {
        sum += reg_counter * 17;
        reg_counter = reg_counter - 1;  /* Explicit decrement */
    }
    *result = sum;
}

/* Variant 7: Nested loops to create different optimization context */
NOOPT void loop_decrement_nested(int bound, int *result) {
    int sum = 0;
    int outer = 3;
    
    while (outer-- != 0) {
        int inner = bound;
        while (inner != 0) {
            sum += (inner * 19) + outer;
            inner--;
        }
    }
    *result = sum;
}

int main(int argc, char *argv[]) {
    int bound;
    
    /* Use command line or volatile to get non-constant bound */
    if (argc > 1) {
        bound = atoi(argv[1]);
        if (bound <= 0) bound = 1000;
    } else {
        /* Use volatile to prevent compile-time constant propagation */
        volatile int vol_bound = 1000;
        bound = vol_bound;
    }
    
    /* Initialize global volatile */
    global_counter = bound;
    
    int results[8] = {0};
    int total = 0;
    
    /* Call all loop variants */
    loop_decrement_for(bound, &results[0]);
    loop_decrement_while_pre(bound, &results[1]);
    loop_decrement_while_post(bound, &results[2]);
    loop_decrement_dowhile(bound, &results[3]);
    loop_decrement_unsigned((unsigned int)bound, &results[4]);
    loop_decrement_complex(bound, &results[5]);
    loop_decrement_nested(bound, &results[6]);
    
    /* Calculate checksum to prevent dead code elimination */
    for (int i = 0; i < 7; i++) {
        total += results[i];
    }
    
    /* Additional volatile write to ensure side effects */
    volatile int checksum = total;
    
    printf("Loop bound: %d\n", bound);
    printf("Checksum: %d\n", total);
    
    return 0;
}
