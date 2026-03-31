#include <stdio.h>

/* External function to prevent loop elimination */
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
    
    /* Pattern 5: Inside if statement branch */
    {
        int flag = 1;
        if (flag) {
            int counter = 15;
            do {
                total += 5;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 6: Counter as function parameter */
    {
        int process_count(int count) {
            int local_total = 0;
            do {
                local_total += 6;
                bar();
            } while (--count > 0);
            return local_total;
        }
        total += process_count(8);
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        do {
            total += 7;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 8: Followed by other statements affecting liveness */
    {
        int counter = 12;
        int temp = 0;
        do {
            total += 8;
            temp = total;
            bar();
        } while (--counter > 0);
        /* Additional statement to affect register allocation */
        temp += 100;
    }
    
    /* Pattern 9: Using pointer in loop body */
    {
        int counter = 9;
        int *ptr = &total;
        do {
            *ptr += 9;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 10: Nested in another control flow */
    {
        for (int outer = 0; outer < 2; outer++) {
            int counter = 5;
            do {
                total += 10;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* NON-MATCHING PATTERNS (should fail checks) */
    
    /* Pattern A: Post-increment (should fail GEN_INT(-1) check) */
    {
        int counter = 5;
        do {
            total += 100;
            bar();
        } while (counter++ < 10);
    }
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    {
        int counter = 10;
        do {
            total += 200;
            bar();
        } while (--counter > 5);
    }
    
    /* Pattern C: Volatile counter (may inhibit pattern) */
    {
        volatile int v_counter = 7;
        do {
            total += 300;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern D: Different decrement amount */
    {
        int counter = 20;
        do {
            total += 400;
            bar();
        } while ((counter -= 2) > 0);
    }
    
    return total;
}

/* Dummy implementation of bar() */
void bar(void) {
    /* Empty but external linkage prevents optimization */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int result = accumulate_results();
    
    /* Calculate expected result for verification */
    /* Pattern 1: 100 iterations * 1 = 100 */
    /* Pattern 2: 50 iterations * 2 = 100 */
    /* Pattern 3: 25 iterations * 3 = 75 */
    /* Pattern 4: 10 iterations * 4 = 40 */
    /* Pattern 5: 15 iterations * 5 = 75 */
    /* Pattern 6: 8 iterations * 6 = 48 */
    /* Pattern 7: 1 iteration * 7 = 7 */
    /* Pattern 8: 12 iterations * 8 = 96 */
    /* Pattern 9: 9 iterations * 9 = 81 */
    /* Pattern 10: 2 * 5 iterations * 10 = 100 */
    /* Non-matching patterns still execute */
    /* Pattern A: 6 iterations * 100 = 600 */
    /* Pattern B: 5 iterations * 200 = 1000 */
    /* Pattern C: 7 iterations * 300 = 2100 */
    /* Pattern D: 10 iterations * 400 = 4000 */
    
    int expected = 100 + 100 + 75 + 40 + 75 + 48 + 7 + 96 + 81 + 100 +
                   600 + 1000 + 2100 + 4000;
    
    printf("Result: %d\n", result);
    printf("Expected: %d\n", expected);
    printf("Match: %s\n", result == expected ? "YES" : "NO");
    
    return 0;
}
