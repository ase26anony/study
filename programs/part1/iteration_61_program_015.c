#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

int main(void) {
    int total = 0;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 10;
        do {
            total += 1;  /* Simple side effect */
            bar();       /* External call prevents optimization */
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 20;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register-qualified counter */
    {
        register int reg_counter = 15;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short s_counter = 8;
        do {
            total += 4;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 5: char type counter */
    {
        char c_counter = 5;
        do {
            total += 5;
            bar();
        } while (--c_counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter simulation */
    {
        int param_counter = 12;
        /* Simulate parameter by using it in a wrapper */
        do {
            total += 6;
            bar();
        } while ((param_counter -= 1) != 0);
    }
    
    /* Pattern 7: Loop inside if statement */
    {
        int if_counter = 7;
        if (total > 0) {
            do {
                total += 7;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 8: Counter starting at 1 (executes once) */
    {
        int once_counter = 1;
        do {
            total += 8;
            bar();
        } while (--once_counter > 0);
    }
    
    /* Pattern 9: Following other statements */
    {
        int temp = total;
        int follow_counter = 9;
        temp *= 2;
        do {
            total += 9;
            bar();
        } while (--follow_counter > 0);
        total += temp;
    }
    
    /* Pattern 10: Using pointer in loop body */
    {
        int ptr_counter = 6;
        int *ptr = &total;
        do {
            *ptr += 10;
            bar();
        } while (--ptr_counter > 0);
    }
    
    /* NEGATIVE TEST CASES (should NOT match the pattern) */
    
    /* Pattern 11: Post-increment (should fail GEN_INT(-1) check) */
    {
        int post_counter = 3;
        do {
            total += 11;
            bar();
            post_counter++;
        } while (post_counter < 10);
    }
    
    /* Pattern 12: Compare against non-zero (should fail const0_rtx check) */
    {
        int non_zero_counter = 4;
        do {
            total += 12;
            bar();
        } while (--non_zero_counter > 2);
    }
    
    /* Pattern 13: volatile counter (may inhibit pattern) */
    {
        volatile int vol_counter = 3;
        do {
            total += 13;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern 14: Different decrement pattern */
    {
        int diff_counter = 5;
        do {
            total += 14;
            bar();
            diff_counter = diff_counter - 1;
        } while (diff_counter > 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    static int dummy = 0;
    dummy++;
}
