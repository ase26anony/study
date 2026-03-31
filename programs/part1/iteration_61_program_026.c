#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int base, int value) {
    return base + value;
}

/* Main test function with various do-while patterns */
int main(void) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    
    /* ===== PATTERN 1: Basic signed int decrement ===== */
    /* Should generate: do { ... } while (--counter > 0) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter > 0);
    }
    
    /* ===== PATTERN 2: Unsigned int with != 0 comparison ===== */
    /* Should generate: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            vol_total += 1;
        } while (--u_counter != 0);
    }
    
    /* ===== PATTERN 3: register-qualified short counter ===== */
    /* May encourage register allocation for the decrement */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* ===== PATTERN 4: char counter in if statement context ===== */
    /* Tests pattern matching within conditional context */
    {
        char char_counter = 10;
        int local_sum = 0;
        
        if (total > 0) {
            do {
                local_sum += 4;
                bar();
            } while (--char_counter > 0);
        }
        total += local_sum;
    }
    
    /* ===== PATTERN 5: Counter as function parameter ===== */
    /* Tests pattern with counter not being a simple local */
    {
        int param_counter = 5;
        /* Helper function to use parameter as counter */
        void loop_with_param(int cnt) {
            do {
                total += 5;
            } while (--cnt > 0);
        }
        loop_with_param(param_counter);
    }
    
    /* ===== PATTERN 6: Counter starting at 1 (edge case) ===== */
    /* Tests single-iteration loop */
    {
        int single_counter = 1;
        do {
            total += 6;
            bar();
        } while (--single_counter > 0);
    }
    
    /* ===== PATTERN 7: Counter with explicit subtraction ===== */
    /* Should generate: do { ... } while ((counter -= 1) != 0) */
    {
        int sub_counter = 15;
        do {
            total += 7;
        } while ((sub_counter -= 1) != 0);
    }
    
    /* ===== NEGATIVE TEST 1: Post-increment (should NOT match) ===== */
    /* Should fail the GEN_INT(-1) check */
    {
        int inc_counter = 8;
        do {
            total += 8;
        } while (inc_counter++ < 7);  /* Post-increment, not decrement */
    }
    
    /* ===== NEGATIVE TEST 2: Compare against non-zero (should NOT match) ===== */
    /* Should fail the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 12;
        do {
            total += 9;
        } while (--non_zero_counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* ===== PATTERN 8: volatile counter (may inhibit pattern) ===== */
    /* Tests if volatile prevents the optimization */
    {
        volatile int vol_counter = 3;
        do {
            total += 10;
        } while (--vol_counter > 0);
    }
    
    /* ===== PATTERN 9: Nested in complex control flow ===== */
    /* Tests pattern recognition with surrounding code */
    {
        int complex_counter = 7;
        int temp = 0;
        
        /* Some preceding code */
        for (int i = 0; i < 3; i++) {
            temp += i;
        }
        
        /* Target do-while */
        do {
            total += 11 + temp;
            bar();
        } while (--complex_counter > 0);
        
        /* Following code that might affect liveness */
        temp = accumulate_result(total, 100);
    }
    
    /* ===== PATTERN 10: Multiple decrements in same function ===== */
    /* Tests multiple instances of the pattern */
    {
        int multi_counter1 = 4;
        int multi_counter2 = 6;
        
        do {
            total += 12;
        } while (--multi_counter1 > 0);
        
        /* Some intermediate code */
        bar();
        
        do {
            total += 13;
        } while (--multi_counter2 > 0);
    }
    
    /* Final result output */
    printf("Result: %d\n", total + vol_total);
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
