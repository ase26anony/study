#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int base, int add) {
    return base + add;
}

/* Main test function with various do-while patterns */
int main(void) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent optimization */
    
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
            vol_total += 2;
            total = accumulate_result(total, 2);
        } while (--u_counter != 0);
    }
    
    /* ===== PATTERN 3: Short type with register qualifier ===== */
    /* register keyword may encourage register allocation */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* ===== PATTERN 4: Char type in if statement context ===== */
    /* Tests pattern matching inside control flow */
    if (total > 0) {
        char c_counter = 10;
        do {
            total += 4;
            vol_total += 1;
        } while (--c_counter > 0);
    }
    
    /* ===== PATTERN 5: Counter as function parameter ===== */
    /* Tests pattern with parameter instead of local */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* ===== PATTERN 6: Counter starting at 1 (edge case) ===== */
    /* Tests single iteration case */
    {
        int single_counter = 1;
        do {
            total += 6;
        } while (--single_counter > 0);
    }
    
    /* ===== PATTERN 7: Explicit subtraction form ===== */
    /* Should generate: do { ... } while ((counter -= 1) != 0) */
    {
        int sub_counter = 15;
        do {
            total += 7;
            bar();
        } while ((sub_counter -= 1) != 0);
    }
    
    /* ===== NEGATIVE TEST 1: Post-increment (should NOT match) ===== */
    /* This should fail the GEN_INT(-1) check */
    {
        int post_counter = 8;
        do {
            total += 8;
        } while (post_counter++ < 7);  /* Wrong direction for pattern */
    }
    
    /* ===== NEGATIVE TEST 2: Compare against non-zero (should NOT match) ===== */
    /* This should fail the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 12;
        do {
            total += 9;
        } while (--non_zero_counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* ===== PATTERN 8: With pointer side effect ===== */
    /* Different loop body to test RTL generation */
    {
        int ptr_counter = 7;
        int dummy = 0;
        int *ptr = &dummy;
        do {
            *ptr = ptr_counter;  /* Simple store */
            total += 10;
        } while (--ptr_counter > 0);
    }
    
    /* ===== PATTERN 9: Mixed with other statements ===== */
    /* Tests liveness analysis */
    {
        int mixed_counter = 9;
        int temp = total;
        do {
            temp += 11;
            bar();
        } while (--mixed_counter > 0);
        total = accumulate_result(total, temp);
    }
    
    /* ===== PATTERN 10: Volatile counter (likely won't match but tests) ===== */
    {
        volatile int vol_counter = 3;
        do {
            total += 12;
        } while (--vol_counter > 0);
    }
    
    printf("Result: %d (volatile: %d)\n", total, vol_total);
    return total > 0 ? 0 : 1;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
