#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to accumulate results */
int accumulate_results(void) {
    int total = 0;
    
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
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char type with explicit subtraction */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while ((c_counter -= 1) != 0);
    }
    
    /* Pattern 5: Counter as function parameter (simulated) */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 6: Loop inside conditional branch */
    {
        int flag = 1;
        if (flag) {
            int counter = 8;
            do {
                total += 6;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        do {
            total += 7;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 8: Followed by other statements */
    {
        int counter = 3;
        int temp = 0;
        do {
            total += 8;
            bar();
            temp++;
        } while (--counter > 0);
        total += temp;  /* Additional statement affecting register usage */
    }
    
    /* Pattern 9: Volatile counter (should NOT match pattern) */
    {
        volatile int v_counter = 4;
        do {
            total += 9;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern 10: Non-zero comparison (should NOT match pattern) */
    {
        int counter = 7;
        do {
            total += 10;
            bar();
        } while (--counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* Pattern 11: Post-increment (should NOT match pattern) */
    {
        int counter = 6;
        do {
            total += 11;
            bar();
        } while (counter++ < 5);  /* Post-increment, not decrement */
    }
    
    /* Pattern 12: Pointer-based side effect in body */
    {
        int counter = 9;
        int local_var = 0;
        int *ptr = &local_var;
        do {
            *ptr = total % 100;  /* Simple side effect */
            total += 12;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 13: Nested simple operations */
    {
        int counter = 2;
        do {
            total = total * 2 + 1;  /* Non-trivial but simple operation */
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 14: Different storage duration simulation */
    {
        static int static_counter = 4;  /* static might affect optimization */
        do {
            total += 14;
            bar();
        } while (--static_counter > 0);
    }
    
    return total;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Empty but non-const/non-pure */
    asm volatile("" ::: "memory");
}

int main(void) {
    int result = accumulate_results();
    
    /* Calculate expected result for verification */
    /* Pattern 1: 100 iterations × 1 = 100 */
    /* Pattern 2: 50 iterations × 2 = 100 */
    /* Pattern 3: 25 iterations × 3 = 75 */
    /* Pattern 4: 10 iterations × 4 = 40 */
    /* Pattern 5: 5 iterations × 5 = 25 */
    /* Pattern 6: 8 iterations × 6 = 48 */
    /* Pattern 7: 1 iteration × 7 = 7 */
    /* Pattern 8: 3 iterations × 8 = 24, plus temp=3 = 27 */
    /* Pattern 9: 4 iterations × 9 = 36 */
    /* Pattern 10: 2 iterations × 10 = 20 (counter from 7 down to 5) */
    /* Pattern 11: 0 iterations × 11 = 0 (condition false initially) */
    /* Pattern 12: 9 iterations × 12 = 108 */
    /* Pattern 13: 2 iterations: ((0*2+1)*2+1) = 3 */
    /* Pattern 14: 4 iterations × 14 = 56 */
    /* Total: 100+100+75+40+25+48+7+27+36+20+0+108+3+56 = 645 */
    
    printf("Result: %d\n", result);
    printf("Expected: 645\n");
    
    return (result == 645) ? 0 : 1;
}
