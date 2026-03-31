#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to create various loop patterns */
void test_loops(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent optimization */
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            *((volatile int*)&vol_total) = total;
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register qualified variable */
    {
        register int reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short short_counter = 10;
        do {
            total += 4;
        } while (--short_counter > 0);
    }
    
    /* Pattern 5: char type counter */
    {
        char char_counter = 5;
        do {
            total += 5;
            bar();
        } while (--char_counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter */
    {
        int local_param = param_counter;
        if (local_param > 0) {
            do {
                total += 6;
                *((volatile int*)&vol_total) = total;
            } while (--local_param > 0);
        }
    }
    
    /* Pattern 7: Loop inside if statement */
    {
        int counter = 8;
        if (total < 1000) {
            do {
                total += 7;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 8: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        do {
            total += 8;
        } while (--counter > 0);
    }
    
    /* Pattern 9: Alternative form with explicit subtraction */
    {
        int counter = 15;
        do {
            total += 9;
            bar();
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 10: Different storage context */
    {
        static int static_counter = 12;  /* static might affect pattern */
        int local_ref = static_counter;
        do {
            total += 10;
            *((volatile int*)&vol_total) = total;
        } while (--local_ref > 0);
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment (should not match GEN_INT(-1)) */
    {
        int counter = 10;
        do {
            total += 100;
            bar();
        } while (counter++ < 20);
    }
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    {
        int counter = 10;
        do {
            total += 200;
        } while (--counter > 5);
    }
    
    /* Pattern C: Complex expression in condition */
    {
        int counter = 10;
        int limit = 0;
        do {
            total += 300;
            bar();
        } while (--counter != limit);
    }
    
    /* Pattern D: volatile counter (should inhibit the pattern) */
    {
        volatile int vol_counter = 5;
        do {
            total += 400;
        } while (vol_counter-- > 0);
    }
    
    printf("Total: %d\n", total + vol_total);
}

/* Main function with multiple test cases */
int main() {
    /* Test with different parameter values */
    test_loops(7);
    test_loops(3);
    test_loops(0);
    
    /* Additional direct loops in main */
    int main_total = 0;
    
    /* Simple decrement pattern */
    {
        int counter = 20;
        do {
            main_total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Unsigned pattern with external call */
    {
        unsigned int counter = 15;
        do {
            main_total += 2;
            bar();
        } while (--counter != 0);
    }
    
    printf("Main total: %d\n", main_total);
    
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
#ifdef NO_EXTERNAL_LINK
void bar(void) {
    static int dummy = 0;
    dummy++;
}
#endif
