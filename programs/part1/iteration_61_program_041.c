#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
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
    volatile int vol_total = 0; /* Prevent some optimizations */
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
    
    /* Pattern 3: register qualified short */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: char type with compound assignment */
    {
        char char_counter = 10;
        do {
            total += 4;
            bar();
        } while ((char_counter -= 1) != 0);
    }
    
    /* Pattern 5: Inside if statement */
    {
        int flag = 1;
        if (flag) {
            int if_counter = 15;
            do {
                vol_total += 5;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 6: Counter as function parameter */
    {
        int param_counter = 8;
        do {
            total += 6;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 7: Counter starting at 1 (executes once) */
    {
        int single_counter = 1;
        do {
            total += 7;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 8: Followed by other statements */
    {
        int follow_counter = 12;
        do {
            total += 8;
            bar();
        } while (--follow_counter > 0);
        
        /* Additional statements affecting register allocation */
        int temp = total * 2;
        vol_total += temp;
    }
    
    /* Pattern 9: Nested in another control flow */
    {
        for (int i = 0; i < 2; i++) {
            int nested_counter = 5;
            do {
                total += 9;
                bar();
            } while (--nested_counter > 0);
        }
    }
    
    /* Pattern 10: Different storage class combinations */
    {
        auto int auto_counter = 7;
        register int reg_auto_counter = 6;
        
        do {
            total += 10;
            bar();
        } while (--auto_counter > 0);
        
        do {
            total += 11;
            bar();
        } while (--reg_auto_counter > 0);
    }
    
    /* NON-MATCHING PATTERNS (should not trigger the uncovered code) */
    
    /* Pattern A: Post-increment (should NOT match) */
    {
        int post_counter = 5;
        do {
            total += 100;
            bar();
        } while (post_counter++ < 10);
    }
    
    /* Pattern B: Compare against non-zero (should NOT match) */
    {
        int non_zero_counter = 20;
        do {
            total += 200;
            bar();
        } while (--non_zero_counter > 5);
    }
    
    /* Pattern C: Volatile counter (likely won't match) */
    {
        volatile int vol_counter = 3;
        do {
            total += 300;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern D: While loop instead of do-while (different structure) */
    {
        int while_counter = 4;
        while (--while_counter > 0) {
            total += 400;
            bar();
        }
    }
    
    printf("Accumulated total: %d\n", total);
    printf("Volatile total: %d\n", vol_total);
}

/* Main function with multiple test cases */
int main(int argc, char *argv[]) {
    int final_result = 0;
    
    /* Run the pattern tests multiple times */
    for (int run = 0; run < 3; run++) {
        test_doloop_patterns();
        final_result = accumulate_result(run * 100);
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Additional test with parameterized counter */
    if (argc > 1) {
        int arg_counter = atoi(argv[1]);
        if (arg_counter > 0) {
            do {
                final_result += 1;
                bar();
            } while (--arg_counter > 0);
            printf("Argument loop result: %d\n", final_result);
        }
    }
    
    return final_result != 0 ? 0 : 1;
}

/* Dummy implementation of bar() if not linked externally */
#ifdef NO_EXTERNAL_LINK
void bar(void) {
    static int counter = 0;
    counter++;
}
#endif
