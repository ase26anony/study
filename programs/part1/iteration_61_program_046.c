#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int total) {
    static int storage = 0;
    storage += total;
    return storage;
}

/* Test function with various do-while patterns */
void test_doloop_patterns(void) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    int *ptr = &total;
    
    /* PATTERN 1: Basic signed int decrement - should match */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int with != 0 comparison - should match */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with explicit decrement */
    {
        short s_counter = 25;
        do {
            total += 3;
            bar();
        } while ((s_counter -= 1) != 0);
    }
    
    /* PATTERN 4: Char type in register storage class */
    {
        register char c_counter = 10;
        do {
            vol_total += 1;
            bar();
        } while (--c_counter > 0);
    }
    
    /* PATTERN 5: Counter as function parameter */
    void loop_with_param(int param_counter) {
        do {
            total += param_counter;
            bar();
        } while (--param_counter > 0);
    }
    loop_with_param(5);
    
    /* PATTERN 6: Loop inside conditional branch */
    {
        int flag = 1;
        if (flag) {
            int inner_counter = 8;
            do {
                total += 4;
                bar();
            } while (--inner_counter != 0);
        }
    }
    
    /* PATTERN 7: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            total += 5;
            bar();
        } while (--single_counter > 0);
    }
    
    /* PATTERN 8: Loop followed by other statements */
    {
        int counter = 7;
        do {
            total += 6;
            bar();
        } while (--counter > 0);
        /* Additional statements affecting register allocation */
        int temp = total * 2;
        vol_total = temp;
    }
    
    /* NEGATIVE TEST 1: Post-increment (should NOT match) */
    {
        int counter = 3;
        do {
            total += 7;
            bar();
        } while (counter++ < 10);  /* Wrong direction for pattern */
    }
    
    /* NEGATIVE TEST 2: Compare against non-zero (should NOT match) */
    {
        int counter = 4;
        do {
            total += 8;
            bar();
        } while (--counter > 2);  /* Compare against 2, not 0 */
    }
    
    /* PATTERN 9: Complex body but simple decrement */
    {
        int counter = 6;
        int local_sum = 0;
        do {
            /* Simple side effects only */
            local_sum += counter;
            *ptr += 1;
            bar();
        } while (--counter > 0);
        total += local_sum;
    }
    
    /* PATTERN 10: Mixed types in comparison */
    {
        unsigned char uc_counter = 12;
        do {
            total += (int)uc_counter;
            bar();
        } while (--uc_counter != 0);
    }
    
    printf("Final total: %d\n", total);
    printf("Volatile total: %d\n", vol_total);
}

/* Main function with compilation barrier */
int main(void) {
    /* Prevent interprocedural optimizations from removing test */
    __asm__ __volatile__("" : : : "memory");
    
    test_doloop_patterns();
    
    /* Additional context to affect optimization */
    int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += i;
    }
    
    return 0;
}

/* Dummy implementation of bar to allow linking */
void bar(void) {
    static int call_count = 0;
    call_count++;
}
