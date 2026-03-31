#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int accumulate(int value) {
    static int total = 0;
    total += value;
    return total;
}

/* Test function with various do-while patterns */
void test_loops(int param_counter) {
    int result = 0;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        int sum = 0;
        do {
            sum += counter;
            bar();  /* External call prevents optimization */
        } while (--counter > 0);
        result += sum;
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        int prod = 1;
        do {
            prod *= 2;
            bar();
        } while (--u_counter != 0);
        result += prod;
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        int temp = 0;
        do {
            temp += s_counter;
            bar();
        } while (--s_counter > 0);
        result += temp;
    }
    
    /* Pattern 4: Char type with explicit decrement */
    {
        char c_counter = 10;
        int acc = 0;
        do {
            acc += c_counter;
            bar();
        } while ((c_counter -= 1) != 0);
        result += acc;
    }
    
    /* Pattern 5: Counter as function parameter */
    {
        int local_copy = param_counter;
        if (local_copy > 0) {
            int val = 0;
            do {
                val += local_copy;
                bar();
            } while (--local_copy > 0);
            result += val;
        }
    }
    
    /* Pattern 6: Inside conditional branch */
    {
        int flag = 1;
        if (flag) {
            int counter = 15;
            int running = 0;
            do {
                running += counter;
                bar();
            } while (--counter > 0);
            result += running;
        }
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        int once = 0;
        do {
            once = 42;
            bar();
        } while (--counter > 0);
        result += once;
    }
    
    /* Pattern 8: With pointer in body */
    {
        int counter = 8;
        int data[10] = {0};
        int *ptr = data;
        do {
            *ptr = counter;
            ptr++;
            bar();
        } while (--counter > 0);
        result += data[0];
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern 9: Post-increment (should not match GEN_INT(-1)) */
    {
        int counter = 5;
        int dummy = 0;
        do {
            dummy++;
            bar();
        } while (counter++ < 10);
        result += dummy;
    }
    
    /* Pattern 10: Compare against non-zero (should fail const0_rtx check) */
    {
        int counter = 20;
        int check = 0;
        do {
            check += 2;
            bar();
        } while (--counter > 5);
        result += check;
    }
    
    /* Pattern 11: Volatile counter (likely inhibits pattern) */
    {
        volatile int v_counter = 7;
        int volatile_sum = 0;
        do {
            volatile_sum += 1;
            bar();
        } while (--v_counter > 0);
        result += volatile_sum;
    }
    
    /* Store final result */
    accumulate(result);
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different parameters */
    test_loops(12);
    test_loops(3);
    test_loops(7);
    
    /* Final computation to ensure loops aren't optimized away */
    total = accumulate(0);
    
    printf("Result: %d\n", total);
    
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
