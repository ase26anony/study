#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int main(void) {
    int total = 0;
    volatile int *ptr = &total;  /* Use volatile pointer for side effects */
    
    /* ====== PATTERN 1: Basic signed int decrement ====== */
    /* Should produce: do { ... } while (--counter > 0) */
    {
        int counter1 = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter1 > 0);
    }
    
    /* ====== PATTERN 2: Unsigned int with != 0 comparison ====== */
    /* Should produce: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;  /* Use volatile pointer access */
        } while (--u_counter != 0);
    }
    
    /* ====== PATTERN 3: register qualified variable ====== */
    /* register hint might encourage REG_P in RTL */
    {
        register int reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* ====== PATTERN 4: short type counter ====== */
    {
        short short_counter = 10;
        do {
            total += 4;
        } while (--short_counter > 0);
    }
    
    /* ====== PATTERN 5: char type counter ====== */
    {
        char char_counter = 5;
        do {
            total += 5;
            bar();
        } while (--char_counter > 0);
    }
    
    /* ====== PATTERN 6: Counter as function parameter pattern ====== */
    /* Simulated by wrapping in a helper */
    {
        int param_counter = 8;
        do {
            total += 6;
        } while ((param_counter -= 1) != 0);  /* Explicit -= 1 form */
    }
    
    /* ====== PATTERN 7: Loop inside conditional ====== */
    /* Tests context sensitivity */
    if (total > 0) {
        int cond_counter = 7;
        do {
            total += 7;
            bar();
        } while (--cond_counter > 0);
    }
    
    /* ====== PATTERN 8: Counter starting at 1 (edge case) ====== */
    {
        int edge_counter = 1;
        do {
            total += 8;
        } while (--edge_counter > 0);
    }
    
    /* ====== PATTERN 9: Followed by other statements ====== */
    {
        int follow_counter = 6;
        do {
            total += 9;
        } while (--follow_counter > 0);
        /* Additional statement affecting register allocation */
        int temp = total * 2;
        (void)temp;  /* Use to prevent optimization */
    }
    
    /* ====== NEGATIVE TEST 1: Post-increment (should NOT match) ====== */
    /* This should fail the GEN_INT(-1) check */
    {
        int post_counter = 5;
        do {
            total += 10;
        } while (post_counter-- > 0);
    }
    
    /* ====== NEGATIVE TEST 2: Compare against non-zero (should NOT match) ====== */
    /* This should fail the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 5;
        do {
            total += 11;
        } while (--non_zero_counter > 3);
    }
    
    /* ====== NEGATIVE TEST 3: volatile counter (likely won't match) ====== */
    /* volatile might inhibit the pattern */
    {
        volatile int vol_counter = 4;
        do {
            total += 12;
        } while (--vol_counter > 0);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}

/* Dummy implementation of bar() if linking standalone */
void bar(void) {
    static int dummy;
    dummy++;
}
