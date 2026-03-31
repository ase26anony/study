/* test-loop-doloop.c */
/* Compile with: gcc -O2 -march=powerpc64 -fdump-rtl-doloop -S test-loop-doloop.c */
/* Or for SPARC: gcc -O3 -mcpu=ultrasparc -fdump-rtl-doloop -S test-loop-doloop.c */

#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent complete optimization */

/* Variant 1: do-while with pre-decrement */
int test_do_while_predec(unsigned int n) {
    int local_sum = 0;
    if (n == 0) return 0;
    
    do {
        local_sum += (n & 1);  /* Simple non-empty body */
        global_sum++;
    } while (--n != 0);  /* Pre-decrement in condition */
    
    return local_sum;
}

/* Variant 2: while with post-decrement */
int test_while_postdec(int n) {
    int local_sum = 0;
    
    while (n-- != 0) {  /* Post-decrement in condition */
        local_sum += (n & 3);  /* Different simple operation */
        global_sum += 2;
    }
    
    return local_sum;
}

/* Variant 3: nested loops - outer loop fixed, inner uses pattern */
int test_nested_loops(unsigned int outer, unsigned int inner) {
    int total = 0;
    unsigned int i, j;
    
    for (i = 0; i < outer; i++) {
        j = inner;
        while (j-- != 0) {  /* Inner loop with post-decrement */
            total += i * j;  /* Non-trivial but simple computation */
            global_sum += 3;
        }
    }
    
    return total;
}

/* Variant 4: mixed signed/unsigned counters */
int test_mixed_types(int start) {
    unsigned int u = (unsigned int)start;
    int s = start;
    int sum = 0;
    
    /* First loop with unsigned */
    do {
        sum += u;
        global_sum += 4;
    } while (--u != 0);
    
    /* Second loop with signed */
    while (s-- != 0) {
        sum -= s;
        global_sum += 5;
    }
    
    return sum;
}

/* Variant 5: loop with compile-time known bound but runtime input */
int test_constant_bound(int multiplier) {
    int sum = 0;
    unsigned int count = 10;  /* Small constant bound */
    
    do {
        sum += multiplier * count;
        global_sum += 6;
    } while (--count != 0);
    
    return sum;
}

int main(int argc, char *argv[]) {
    int base_iterations = 100;
    
    if (argc > 1) {
        base_iterations = atoi(argv[1]);
        if (base_iterations <= 0) base_iterations = 100;
    }
    
    /* Ensure loops run with reasonable bounds */
    unsigned int n1 = (base_iterations % 50) + 10;
    int n2 = (base_iterations % 40) + 10;
    unsigned int outer = 5;
    unsigned int inner = (base_iterations % 30) + 5;
    int start_val = (base_iterations % 20) + 5;
    int multiplier = (base_iterations % 10) + 1;
    
    int result = 0;
    
    result += test_do_while_predec(n1);
    result += test_while_postdec(n2);
    result += test_nested_loops(outer, inner);
    result += test_mixed_types(start_val);
    result += test_constant_bound(multiplier);
    
    /* Print results to prevent dead code elimination */
    printf("Result: %d, Global sum: %d\n", result, global_sum);
    
    return 0;
}
