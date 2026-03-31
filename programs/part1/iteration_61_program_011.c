#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to test loops with parameter counter */
int test_param_counter(int param_counter) {
    int sum = 0;
    do {
        sum += 1;
        bar();
    } while (--param_counter > 0);
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Test 1: Basic int counter with > 0 condition */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Test 2: Unsigned int counter with != 0 condition */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Test 3: Short counter with register qualifier */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Test 4: Char counter with -= 1 syntax */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while ((c_counter -= 1) != 0);
    }
    
    /* Test 5: Counter starting at 1 (executes once) */
    {
        int counter = 1;
        do {
            total += 5;
            bar();
        } while (--counter > 0);
    }
    
    /* Test 6: Loop inside if statement */
    {
        int flag = 1;
        if (flag) {
            int counter = 15;
            do {
                total += 6;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* Test 7: Loop followed by other statements */
    {
        int counter = 20;
        int local_sum = 0;
        do {
            local_sum += 7;
            bar();
        } while (--counter > 0);
        total += local_sum;
    }
    
    /* Test 8: Function parameter counter */
    total += test_param_counter(5);
    
    /* Test 9: Counter with pointer in body */
    {
        int counter = 8;
        int *ptr = &total;
        do {
            *ptr += 8;
            bar();
        } while (--counter > 0);
    }
    
    /* Test 10: Long counter type */
    {
        long l_counter = 6;
        do {
            total += 9;
            bar();
        } while (--l_counter > 0);
    }
    
    /* NON-MATCHING PATTERNS (should not trigger the uncovered code) */
    
    /* Test A: Post-increment (should not match GEN_INT(-1) check) */
    {
        int counter = 3;
        do {
            total += 10;
            bar();
            counter++;
        } while (counter < 10);
    }
    
    /* Test B: Compare against non-zero (should fail cmp_arg2 != const0_rtx) */
    {
        int counter = 10;
        do {
            total += 11;
            bar();
        } while (--counter > 5);
    }
    
    /* Test C: Volatile counter (may inhibit the pattern) */
    {
        volatile int v_counter = 4;
        do {
            total += 12;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Test D: Different decrement pattern */
    {
        int counter = 7;
        do {
            total += 13;
            bar();
            counter = counter - 1;
        } while (counter > 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
