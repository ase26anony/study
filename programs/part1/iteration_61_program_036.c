#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to use loops in different contexts */
int test_loops(int param_counter) {
    int total = 0;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 10;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 8;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register qualified variable */
    {
        register int reg_counter = 6;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short s_counter = 4;
        do {
            total += 4;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 5: char type counter */
    {
        char c_counter = 3;
        do {
            total += 5;
            bar();
        } while (--c_counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter */
    {
        int counter = param_counter;
        if (counter > 0) {
            do {
                total += 6;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 7: Loop inside if statement */
    {
        int flag = 1;
        if (flag) {
            int counter = 5;
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
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 9: Explicit subtraction form */
    {
        int counter = 7;
        do {
            total += 9;
            bar();
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 10: Loop with pointer in body */
    {
        int counter = 4;
        int *ptr = &total;
        do {
            *ptr += 10;
            bar();
        } while (--counter > 0);
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment (should not match GEN_INT(-1)) */
    {
        int counter = 3;
        do {
            total += 100;
            bar();
        } while (counter++ < 5);
    }
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    {
        int counter = 10;
        do {
            total += 200;
            bar();
        } while (--counter > 5);
    }
    
    /* Pattern C: volatile counter (likely inhibits the pattern) */
    {
        volatile int vol_counter = 3;
        do {
            total += 300;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern D: Different decrement amount */
    {
        int counter = 12;
        do {
            total += 400;
            bar();
        } while ((counter -= 2) > 0);
    }
    
    return total;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy;
    dummy++;
}

int main(void) {
    int result = test_loops(3);
    printf("Result: %d\n", result);
    
    /* Additional loops in main for more coverage */
    {
        int counter = 5;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 0);
        printf("Sum: %d\n", sum);
    }
    
    {
        unsigned char uc_counter = 4;
        int prod = 1;
        do {
            prod *= 2;
            bar();
        } while (--uc_counter != 0);
        printf("Prod: %d\n", prod);
    }
    
    return 0;
}
