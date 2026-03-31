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
    
    /* Pattern 5: Counter as function parameter */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 6: Loop inside conditional */
    {
        int flag = 1;
        if (flag) {
            int inner_counter = 8;
            do {
                total += 6;
                bar();
            } while (--inner_counter > 0);
        }
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            total += 7;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 8: Loop followed by other statements */
    {
        int follow_counter = 3;
        int temp = 0;
        do {
            total += 8;
            temp++;
            bar();
        } while (--follow_counter > 0);
        total += temp;  /* Additional statement affecting register usage */
    }
    
    /* Pattern 9: Using pointer in loop body */
    {
        int ptr_counter = 4;
        int value = 0;
        int *ptr = &value;
        do {
            *ptr = total % 100;
            total += 9;
            bar();
        } while (--ptr_counter > 0);
    }
    
    /* Pattern 10: Volatile counter (should NOT match pattern) */
    {
        volatile int vol_counter = 2;
        do {
            total += 10;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern 11: Post-increment (should NOT match pattern) */
    {
        int post_counter = 3;
        do {
            total += 11;
            bar();
        } while (post_counter++ < 2);  /* Different pattern */
    }
    
    /* Pattern 12: Compare against non-zero (should NOT match pattern) */
    {
        int non_zero_counter = 4;
        do {
            total += 12;
            bar();
        } while (--non_zero_counter > 2);  /* Not comparing against 0 */
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
    printf("Result: %d\n", result);
    
    /* Verify expected result */
    int expected = 
        (100 * 1) +    /* Pattern 1 */
        (50 * 2) +     /* Pattern 2 */
        (25 * 3) +     /* Pattern 3 */
        (10 * 4) +     /* Pattern 4 */
        (5 * 5) +      /* Pattern 5 */
        (8 * 6) +      /* Pattern 6 */
        (1 * 7) +      /* Pattern 7 */
        (3 * 8 + 3) +  /* Pattern 8 */
        (4 * 9) +      /* Pattern 9 */
        (2 * 10) +     /* Pattern 10 */
        (0 * 11) +     /* Pattern 11 (0 iterations of do-while) */
        (2 * 12);      /* Pattern 12 (2 iterations: 4→3, 3→2) */
    
    printf("Expected: %d\n", expected);
    printf("Match: %s\n", result == expected ? "YES" : "NO");
    
    return 0;
}
