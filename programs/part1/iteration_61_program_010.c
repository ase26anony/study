#include <stdio.h>

/* External function to prevent optimization */
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
            *((volatile int*)&total) = total;  /* Simple side effect */
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register qualified variable */
    {
        register int reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short s_counter = 10;
        do {
            total += 4;
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
    
    /* Pattern 6: Counter as function parameter (simulated) */
    {
        int param_counter = 8;
        /* Simulate parameter by using in nested scope */
        do {
            total += 6;
            *((volatile int*)&total) = total;
        } while (--param_counter > 0);
    }
    
    /* Pattern 7: Inside if statement */
    {
        int flag = 1;
        if (flag) {
            int if_counter = 7;
            do {
                total += 7;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 8: With post-statements affecting register allocation */
    {
        int counter = 6;
        int temp = 0;
        do {
            total += 8;
            temp = total;  /* Additional statement */
        } while (--counter > 0);
        total += temp;  /* Use temp after loop */
    }
    
    /* Pattern 9: Counter starts at 1 (edge case) */
    {
        int edge_counter = 1;
        do {
            total += 9;
        } while (--edge_counter > 0);
    }
    
    /* Pattern 10: Using -= operator instead of prefix decrement */
    {
        int counter = 4;
        do {
            total += 10;
        } while ((counter -= 1) > 0);
    }
    
    return total;
}

/* Negative test cases that should NOT match the pattern */
void negative_tests(int *partial) {
    /* Should fail: post-increment instead of decrement */
    {
        int counter = 3;
        do {
            *partial += 1;
        } while (counter++ < 3);  /* Wrong direction, should not match */
    }
    
    /* Should fail: comparing against non-zero */
    {
        int counter = 10;
        do {
            *partial += 2;
        } while (--counter > 5);  /* cmp_arg2 != const0_rtx check should fail */
    }
    
    /* Should fail: volatile counter (likely inhibits pattern) */
    {
        volatile int vol_counter = 3;
        do {
            *partial += 3;
        } while (--vol_counter > 0);
    }
}

/* Mock external function definition */
void bar(void) {
    /* Empty but external linkage prevents optimization */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    int result = accumulate_results();
    
    int negative_partial = 0;
    negative_tests(&negative_partial);
    
    /* Add negative tests partial result to ensure they're not optimized away */
    result += negative_partial;
    
    printf("Result: %d\n", result);
    
    /* Expected calculation:
     * Pattern 1: 100 iterations * 1 = 100
     * Pattern 2: 50 iterations * 2 = 100
     * Pattern 3: 25 iterations * 3 = 75
     * Pattern 4: 10 iterations * 4 = 40
     * Pattern 5: 5 iterations * 5 = 25
     * Pattern 6: 8 iterations * 6 = 48
     * Pattern 7: 7 iterations * 7 = 49
     * Pattern 8: 6 iterations * 8 = 48
     * Pattern 9: 1 iteration * 9 = 9
     * Pattern 10: 4 iterations * 10 = 40
     * Negative: 3+10+3 = 16
     * Total: 100+100+75+40+25+48+49+48+9+40+16 = 550
     */
    
    return 0;
}
