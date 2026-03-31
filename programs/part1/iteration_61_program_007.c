#include <stdio.h>

/* External function to prevent optimization */
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
    int *ptr = &total;
    
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
            *ptr += 2;
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
    
    /* Pattern 4: Char type with explicit decrement */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while ((c_counter -= 1) != 0);
    }
    
    /* Pattern 5: Counter as function parameter */
    {
        int counter = param_counter;
        if (counter > 0) {
            do {
                total += 5;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Pattern 6: Inside if statement with different type */
    {
        unsigned char uc_counter = 5;
        if (total < 1000) {
            do {
                total += 6;
                bar();
            } while (--uc_counter != 0);
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
    
    /* Pattern 8: With volatile (should NOT match pattern) */
    {
        volatile int v_counter = 3;
        do {
            total += 8;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern 9: Followed by other statements */
    {
        int counter = 7;
        do {
            total += 9;
            bar();
        } while (--counter > 0);
        
        /* Additional statement affecting register allocation */
        int temp = total * 2;
        total = temp / 2;
    }
    
    /* NON-MATCHING PATTERNS (for contrast) */
    
    /* Pattern 10: Post-increment (should NOT match) */
    {
        int counter = 4;
        do {
            total += 10;
            bar();
        } while (counter++ < 10);
    }
    
    /* Pattern 11: Compare against non-zero (should NOT match) */
    {
        int counter = 8;
        do {
            total += 11;
            bar();
        } while (--counter > 5);
    }
    
    /* Pattern 12: Different comparison operator */
    {
        int counter = 6;
        do {
            total += 12;
            bar();
        } while (--counter >= 1);
    }
    
    accumulate_result(total);
}

/* Main function with multiple test calls */
int main() {
    int i;
    
    /* Call test_loops multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        test_loops(15 - i * 5);
    }
    
    /* Final verification computation */
    int final_result = 0;
    int loop_counter = 20;
    
    /* One more basic pattern in main */
    do {
        final_result += loop_counter;
        bar();
    } while (--loop_counter > 0);
    
    printf("Result: %d\n", final_result);
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}
