#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
#define NOOPT __attribute__((noinline, noipa))

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Function 1: Classic for loop with i != 0 condition */
NOOPT void loop_decrement_for(int n) {
    int i;
    volatile int local_sink = 0;
    
    for (i = n; i != 0; i--) {
        local_sink += i * 3;
    }
    global_sink += local_sink;
}

/* Function 2: While loop with explicit decrement and != 0 check */
NOOPT void loop_decrement_while(int n) {
    int cnt = n;
    volatile int local_sink = 0;
    
    while (cnt != 0) {
        local_sink += cnt * 7;
        cnt--;
    }
    global_sink += local_sink;
}

/* Function 3: Pre-decrement in while condition */
NOOPT void loop_decrement_pre(int n) {
    int cnt = n + 1;  /* Start one higher since we pre-decrement */
    volatile int local_sink = 0;
    
    while (--cnt != 0) {
        local_sink += cnt * 11;
    }
    global_sink += local_sink;
}

/* Function 4: Post-decrement in while condition */
NOOPT void loop_decrement_post(int n) {
    int cnt = n;
    volatile int local_sink = 0;
    
    while (cnt-- != 0) {
        local_sink += cnt * 13;
    }
    global_sink += local_sink;
}

/* Function 5: Do-while with decrement and != 0 check */
NOOPT void loop_decrement_dowhile(int n) {
    int cnt = n;
    volatile int local_sink = 0;
    
    if (cnt > 0) {
        do {
            local_sink += cnt * 17;
            cnt--;
        } while (cnt != 0);
    }
    global_sink += local_sink;
}

/* Function 6: Complex expression in loop body to maintain dependency */
NOOPT void loop_decrement_complex(int n) {
    int i = n;
    volatile int local_sink = 0;
    
    while (i != 0) {
        /* Complex enough to prevent optimization but simple enough
           to keep the counter in a register */
        local_sink = (local_sink * 19 + i) & 0xFFF;
        i--;
    }
    global_sink += local_sink;
}

/* Function 7: Unsigned counter (might generate different RTL) */
NOOPT void loop_decrement_unsigned(unsigned int n) {
    unsigned int i = n;
    volatile int local_sink = 0;
    
    for (i = n; i != 0; i--) {
        local_sink += (int)i * 23;
    }
    global_sink += local_sink;
}

/* Function 8: Counter modified in loop body, checked in condition */
NOOPT void loop_decrement_body(int n) {
    int cnt = n;
    volatile int local_sink = 0;
    
    while (cnt != 0) {
        local_sink += cnt * 29;
        cnt = cnt - 1;  /* Explicit subtraction instead of -- */
    }
    global_sink += local_sink;
}

int main(int argc, char *argv[]) {
    int loop_bound;
    
    /* Make loop bound non-constant to prevent unrolling */
    if (argc > 1) {
        loop_bound = atoi(argv[1]);
    } else {
        /* Use volatile read to prevent constant propagation */
        volatile int seed = time(NULL);
        loop_bound = (seed % 1000) + 100;  /* 100-1099 iterations */
    }
    
    printf("Testing with loop bound: %d\n", loop_bound);
    
    /* Call all loop variants */
    loop_decrement_for(loop_bound);
    loop_decrement_while(loop_bound);
    loop_decrement_pre(loop_bound);
    loop_decrement_post(loop_bound);
    loop_decrement_dowhile(loop_bound);
    loop_decrement_complex(loop_bound);
    loop_decrement_unsigned((unsigned int)loop_bound);
    loop_decrement_body(loop_bound);
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", global_sink);
    
    return 0;
}
