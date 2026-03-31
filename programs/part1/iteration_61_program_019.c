#include <stdio.h>

/* External function to prevent loop elimination */
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
    
    /* Pattern 1: Basic signed int decrement - should match */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison - should match */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier - should match */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char type - should match */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while (--c_counter != 0);
    }
    
    /* Pattern 5: Counter as function parameter - might match */
    {
        int local_param = param_counter;
        if (local_param > 0) {
            do {
                total += 5;
                bar();
            } while (--local_param > 0);
        }
    }
    
    /* Pattern 6: Inside if statement with different context */
    {
        int counter = 7;
        if (total < 1000) {
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
    
    /* Pattern 8: Using -= operator instead of prefix decrement */
    {
        int counter = 15;
        do {
            total += 8;
            bar();
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 9: With volatile qualifier (might not match) */
    {
        volatile int v_counter = 5;
        do {
            total += 9;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern 10: Followed by other statements affecting liveness */
    {
        int counter = 12;
        int temp = 0;
        do {
            total += 10;
            temp++;
            bar();
        } while (--counter > 0);
        total += temp;  /* Use temp to affect register allocation */
    }
    
    /* NON-MATCHING PATTERNS (for contrast) */
    
    /* Pattern A: Post-increment instead of decrement - should NOT match */
    {
        int counter = 5;
        do {
            total += 100;
            bar();
        } while (counter++ < 10);
    }
    
    /* Pattern B: Compare against non-zero value - should NOT match */
    {
        int counter = 20;
        do {
            total += 200;
            bar();
        } while (--counter > 5);
    }
    
    /* Pattern C: Complex condition - should NOT match */
    {
        int counter = 8;
        int other = 3;
        do {
            total += 300;
            bar();
        } while (--counter > 0 && other-- > 0);
    }
    
    /* Store the accumulated result */
    accumulate_result(total);
}

/* Main function with multiple test calls */
int main(void) {
    int final_total = 0;
    
    /* Call test function multiple times with different parameters */
    test_loops(3);
    test_loops(0);
    test_loops(8);
    
    /* Additional loops in main for more coverage */
    
    /* Pattern 11: Simple loop with pointer in body */
    {
        int counter = 6;
        int data[10];
        int *ptr = data;
        do {
            *ptr++ = 0;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 12: Nested function call context */
    {
        int counter = 9;
        do {
            bar();
            final_total += counter;
        } while (--counter > 0);
    }
    
    /* Pattern 13: Different integer type with explicit comparison */
    {
        unsigned char uc_counter = 4;
        do {
            final_total += uc_counter;
            bar();
        } while ((uc_counter -= 1) != 0);
    }
    
    /* Print predictable result */
    printf("Result: %d\n", final_total);
    
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Minimal side effect */
    static int call_count = 0;
    call_count++;
}
