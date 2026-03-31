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
NOINLINE void loop_decrement_while_pre(int n);
NOINLINE void loop_decrement_while_post(int n);
NOINLINE void loop_decrement_do_while(int n);
NOINLINE void loop_decrement_complex(int n);

/* Variant 1: for loop with i != 0 condition */
NOINLINE void loop_decrement_for(int n) {
    volatile int local_sum = 0;
    /* Counter in register, decrement by 1, exit when != 0 */
    for (int i = n; i != 0; i--) {
        /* Side effect that depends on counter */
        local_sum += (i * 2);
        /* Additional side effect to prevent optimization */
        global_result ^= i;
    }
    global_counter += local_sum;
}

/* Variant 2: while loop with pre-decrement */
NOINLINE void loop_decrement_while_pre(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    /* Pattern: --cnt != 0 */
    while (--cnt != 0) {
        local_sum += (cnt * 3);
        global_result ^= cnt;
    }
    global_counter += local_sum;
}

/* Variant 3: while loop with post-decrement */
NOINLINE void loop_decrement_while_post(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    /* Pattern: cnt-- != 0 */
    while (cnt-- != 0) {
        local_sum += (cnt * 5);
        global_result ^= cnt;
    }
    global_counter += local_sum;
}

/* Variant 4: do-while with explicit comparison */
NOINLINE void loop_decrement_do_while(int n) {
    volatile int local_sum = 0;
    int cnt = n;
    if (cnt > 0) {
        do {
            local_sum += (cnt * 7);
            global_result ^= cnt;
            /* Decrement and compare with 0 */
        } while (--cnt != 0);
    }
    global_counter += local_sum;
}

/* Variant 5: More complex but still matching the pattern */
NOINLINE void loop_decrement_complex(int n) {
    volatile int local_sum = 0;
    unsigned int counter = (unsigned int)n;
    /* Using unsigned to ensure != 0 comparison */
    for (unsigned int i = counter; i != 0; i--) {
        /* Mix of operations to maintain dependency */
        local_sum += ((int)i * 11) % 256;
        global_result ^= i;
    }
    global_counter += local_sum;
}

/* Helper to get non-constant loop bound */
static int get_loop_bound(void) {
    /* Use multiple sources to prevent constant propagation */
    volatile int seed = time(NULL);
    int bound = (seed % 1000) + 500;  /* Between 500 and 1499 */
    
    /* Add some computation to make it non-trivial */
    bound = (bound * 3) / 2;
    bound = bound & 0x3FF;  /* Keep within 0-1023 */
    
    /* Ensure it's positive and not too small */
    return (bound < 100) ? 100 : bound;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Get non-constant loop bound */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
        if (loop_bound <= 0) loop_bound = 1000;
    } else {
        loop_bound = get_loop_bound();
    }
    
    printf("Testing doloop optimization with bound = %d\n", loop_bound);
    
    /* Reset global variables */
    global_counter = 0;
    global_result = 0;
    
    /* Execute all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while_pre(loop_bound);
    loop_decrement_while_post(loop_bound);
    loop_decrement_do_while(loop_bound);
    loop_decrement_complex(loop_bound);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter ^ global_result;
    printf("Checksum: %d (0x%08x)\n", checksum, checksum);
    
    /* Additional verification */
    if (checksum != 0) {
        printf("All loops executed successfully.\n");
    }
    
    return 0;
}
