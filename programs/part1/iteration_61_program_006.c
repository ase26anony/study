#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int total) {
    static int storage = 0;
    storage += total;
    return storage;
}

/* Test function with various do-while patterns */
void test_loops(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    int *ptr = &total;
    
    /* PATTERN 1: Basic signed int decrement */
    /* Should generate: do { ... } while (--counter > 0) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int decrement with != 0 */
    /* Should generate: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with decrement */
    {
        short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* PATTERN 4: Char type with decrement */
    {
        char c_counter = 10;
        do {
            vol_total += 4;
            bar();
        } while (--c_counter > 0);
    }
    
    /* PATTERN 5: Register qualified variable */
    /* Might encourage register allocation for the counter */
    {
        register int reg_counter = 15;
        do {
            total += 5;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* PATTERN 6: Counter as function parameter */
    /* Tests pattern matching with parameter in different context */
    {
        int local_param = param_counter > 0 ? param_counter : 20;
        do {
            total += 6;
            bar();
        } while (--local_param > 0);
    }
    
    /* PATTERN 7: Loop inside conditional branch */
    /* Tests context sensitivity */
    if (total > 0) {
        int if_counter = 8;
        do {
            total += 7;
            bar();
        } while (--if_counter > 0);
    }
    
    /* PATTERN 8: Counter starting at 1 (edge case) */
    {
        int edge_counter = 1;
        do {
            total += 8;
            bar();
        } while (--edge_counter > 0);
    }
    
    /* PATTERN 9: Alternative decrement syntax */
    /* Should generate: do { ... } while ((counter -= 1) != 0) */
    {
        int alt_counter = 12;
        do {
            total += 9;
            bar();
        } while ((alt_counter -= 1) != 0);
    }
    
    /* PATTERN 10: Post-increment (SHOULD NOT MATCH) */
    /* Tests the GEN_INT(-1) check - this uses +1 not -1 */
    {
        int post_counter = 5;
        do {
            total += 10;
            bar();
        } while (post_counter++ < 10);
    }
    
    /* PATTERN 11: Compare against non-zero (SHOULD NOT MATCH) */
    /* Tests the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 7;
        do {
            total += 11;
            bar();
        } while (--non_zero_counter > 3);
    }
    
    /* PATTERN 12: Complex body but simple decrement */
    {
        int complex_counter = 9;
        int local_sum = 0;
        do {
            /* Simple but non-trivial body */
            local_sum += complex_counter;
            total += local_sum % 5;
            bar();
        } while (--complex_counter > 0);
    }
    
    /* Store final result */
    accumulate_result(total + vol_total);
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int i;
    
    /* Call test_loops multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        test_loops(i * 10 + 5);
    }
    
    /* Final computation that depends on all loops */
    int final_result = accumulate_result(0);
    printf("Result: %d\n", final_result);
    
    return 0;
}
