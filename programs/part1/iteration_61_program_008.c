#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int main(void) {
    int total = 0;
    volatile int *ptr = &total;  /* Use volatile pointer for side effects */
    
    /* ===== PATTERN 1: Basic signed int decrement ===== */
    /* Should generate: do { ... } while (--counter > 0) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter > 0);
    }
    
    /* ===== PATTERN 2: Unsigned int with != 0 comparison ===== */
    /* Should generate: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;  /* Use volatile pointer access */
        } while (--u_counter != 0);
    }
    
    /* ===== PATTERN 3: register-qualified short counter ===== */
    /* May encourage register allocation for the PLUS operand */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* ===== PATTERN 4: char counter in if statement context ===== */
    /* Tests pattern matching in different control flow */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += 4;
            } while (--char_counter > 0);
        }
    }
    
    /* ===== PATTERN 5: Counter as function parameter simulation ===== */
    /* Pass counter through helper to simulate parameter context */
    {
        int param_counter = 5;
        /* Helper to use parameter-like counter */
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* ===== PATTERN 6: Counter starting at 1 (edge case) ===== */
    /* Tests single iteration case */
    {
        int single_counter = 1;
        do {
            total += 6;
        } while (--single_counter > 0);
    }
    
    /* ===== PATTERN 7: Explicit decrement with assignment ===== */
    /* Should generate: do { ... } while ((counter -= 1) != 0) */
    {
        int explicit_counter = 15;
        do {
            total += 7;
        } while ((explicit_counter -= 1) != 0);
    }
    
    /* ===== NEGATIVE TEST 1: Post-increment (should NOT match) ===== */
    /* Should fail GEN_INT(-1) check */
    {
        int post_counter = 8;
        do {
            total += 8;
        } while (post_counter++ < 7);  /* Post-increment, not decrement */
    }
    
    /* ===== NEGATIVE TEST 2: Compare against non-zero (should NOT match) ===== */
    /* Should fail cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 12;
        do {
            total += 9;
        } while (--non_zero_counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* ===== PATTERN 8: volatile counter (may inhibit pattern) ===== */
    /* Tests if volatile prevents the optimization */
    {
        volatile int vol_counter = 3;
        do {
            total += 10;
        } while (--vol_counter > 0);
    }
    
    /* ===== PATTERN 9: Nested context with multiple statements ===== */
    /* Tests pattern matching with surrounding code */
    {
        int context_counter = 7;
        int dummy = 0;
        dummy = total * 2;  /* Code before loop */
        do {
            total += 11;
            bar();
        } while (--context_counter > 0);
        dummy += total;  /* Code after loop */
        (void)dummy;     /* Prevent unused warning */
    }
    
    /* ===== PATTERN 10: Different integer type with compound body ===== */
    {
        long long_counter = 4;
        do {
            total += 12;
            *ptr = *ptr + 1;  /* Additional side effect */
            bar();
        } while (--long_counter > 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
