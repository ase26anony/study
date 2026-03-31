/* test-loop-doloop.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c */
/* Or for SPARC: gcc -O3 -mcpu=ultrasparc -fdump-rtl-doloop -S test-loop-doloop.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    unsigned int counter = n;
    
    do {
        local_sum += (counter & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--counter != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    int counter = n;
    
    while (counter-- != 0) {  /* Post-decrement in condition */
        local_sum += (counter & 3);
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - inner loop with decrement */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int local_sum = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned int counter = inner;
        
        /* Inner do-while loop */
        do {
            local_sum += (i * j) & 0xF;
            global_sum += 3;
            j = counter;
        } while (--counter != 0);
    }
    
    return local_sum;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int n) {
    int local_sum = 0;
    unsigned int u_counter = (unsigned int)n;
    int s_counter = n;
    
    /* First loop with unsigned */
    do {
        local_sum += u_counter % 5;
        global_sum += 4;
    } while (--u_counter != 0);
    
    /* Second loop with signed */
    while (s_counter-- != 0) {
        local_sum += s_counter % 3;
        global_sum += 5;
    }
    
    return local_sum;
}

/* Variant 5: simple countdown loop that should generate clean pattern */
int test_simple_countdown(unsigned int n) {
    int local_sum = 0;
    
    /* This should generate the cleanest pattern */
    while (n-- != 0) {
        local_sum += 1;
        global_sum += 6;
    }
    
    return local_sum;
}

int main(int argc, char *argv[]) {
    int base_count;
    
    if (argc < 2) {
        base_count = 100;  /* Default if no argument */
    } else {
        base_count = atoi(argv[1]);
        if (base_count <= 0) base_count = 50;
    }
    
    int total = 0;
    
    /* Reset global sum for each test */
    global_sum = 0;
    total += test_do_while_predec(base_count);
    
    global_sum = 0;
    total += test_while_postdec(base_count);
    
    global_sum = 0;
    total += test_nested_loops(base_count / 10, 10);
    
    global_sum = 0;
    total += test_mixed_types(base_count);
    
    global_sum = 0;
    total += test_simple_countdown(base_count);
    
    /* Print results to ensure loops executed */
    printf("Total from all loops: %d\n", total);
    printf("Global sum: %d\n", global_sum);
    
    /* Return 0 for success, non-zero if something went wrong */
    return (total == 0) ? 1 : 0;
}
