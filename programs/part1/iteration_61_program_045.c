#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Global pointer for side effects */
volatile int *global_ptr;

int main(void) {
    int total = 0;
    volatile int *ptr = &total;
    
    /* ====== PATTERN 1: Basic signed int decrement ====== */
    /* Should produce: do { ... } while (--counter > 0) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* ====== PATTERN 2: Unsigned int with != 0 comparison ====== */
    /* Should produce: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* ====== PATTERN 3: register-qualified short ====== */
    /* May produce different register allocation patterns */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* ====== PATTERN 4: char type with explicit decrement ====== */
    /* Tests different integer mode expansions */
    {
        char char_counter = 10;
        do {
            total += 4;
            bar();
        } while ((char_counter -= 1) != 0);
    }
    
    /* ====== PATTERN 5: Inside conditional branch ====== */
    /* Tests context sensitivity */
    if (total > 0) {
        int if_counter = 5;
        do {
            total += 5;
            bar();
        } while (--if_counter > 0);
    }
    
    /* ====== PATTERN 6: Counter as function parameter simulation ====== */
    /* Tests liveness analysis */
    {
        int param_counter = 3;
        /* Simulate parameter by passing through local */
        int counter_copy = param_counter;
        do {
            total += 6;
            bar();
        } while (--counter_copy > 0);
    }
    
    /* ====== PATTERN 7: Edge case - counter starts at 1 ====== */
    /* Tests single-iteration loop */
    {
        int single_counter = 1;
        do {
            total += 7;
            bar();
        } while (--single_counter > 0);
    }
    
    /* ====== PATTERN 8: Should NOT match - post-increment ====== */
    /* Should fail the GEN_INT(-1) check */
    {
        int post_counter = 5;
        do {
            total += 8;
            bar();
        } while (post_counter-- > 0);
    }
    
    /* ====== PATTERN 9: Should NOT match - compare against non-zero ====== */
    /* Should fail the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 5;
        do {
            total += 9;
            bar();
        } while (--non_zero_counter > 3);
    }
    
    /* ====== PATTERN 10: volatile counter (likely won't match) ====== */
    /* Tests inhibition of optimization */
    {
        volatile int vol_counter = 5;
        do {
            total += 10;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* ====== PATTERN 11: Nested decrement pattern ====== */
    /* Tests pattern recognition in more complex flow */
    {
        int outer_counter = 3;
        do {
            int inner_counter = 2;
            do {
                total += 11;
                bar();
            } while (--inner_counter > 0);
        } while (--outer_counter > 0);
    }
    
    /* ====== PATTERN 12: Different comparison operator ====== */
    /* Still uses decrement and compare against zero */
    {
        int cmp_counter = 4;
        do {
            total += 12;
            bar();
        } while (cmp_counter-- != 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    static int dummy;
    dummy++;
}
