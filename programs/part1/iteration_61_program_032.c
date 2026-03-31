#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to accumulate results */
int accumulate_results(void) {
    int total = 0;
    
    /* ====== PATTERN 1: Basic signed int decrement ====== */
    /* Should produce: do { ... } while (--counter > 0) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* ====== PATTERN 2: Unsigned int with != 0 comparison ====== */
    /* Should produce: do { ... } while (--u_counter != 0) */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* ====== PATTERN 3: Short type with register qualifier ====== */
    /* register qualifier may encourage register allocation */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* ====== PATTERN 4: Char type with pointer side effect ====== */
    {
        char c_counter = 10;
        char buffer[10];
        char *ptr = buffer;
        do {
            *ptr++ = 0;  /* Simple side effect */
            total += 4;
        } while (--c_counter > 0);
    }
    
    /* ====== PATTERN 5: Counter as function parameter ====== */
    /* Testing boundary: counter not a local variable */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* ====== PATTERN 6: Loop inside conditional branch ====== */
    /* Different context for doloop pass */
    if (total > 0) {
        int if_counter = 8;
        do {
            total += 6;
            bar();
        } while (--if_counter > 0);
    }
    
    /* ====== PATTERN 7: Counter starting at 1 (executes once) ====== */
    /* Boundary case for pattern matching */
    {
        int once_counter = 1;
        do {
            total += 7;
            bar();
        } while (--once_counter > 0);
    }
    
    /* ====== PATTERN 8: Compound decrement expression ====== */
    /* Should produce: do { ... } while ((counter -= 1) != 0) */
    {
        int comp_counter = 12;
        do {
            total += 8;
            bar();
        } while ((comp_counter -= 1) != 0);
    }
    
    /* ====== NEGATIVE TEST 1: Post-increment (should NOT match) ====== */
    /* This should fail the GEN_INT(-1) check */
    {
        int post_counter = 3;
        do {
            total += 9;
            bar();
        } while (post_counter++ < 3);
    }
    
    /* ====== NEGATIVE TEST 2: Compare against non-zero (should NOT match) ====== */
    /* This should fail the cmp_arg2 != const0_rtx check */
    {
        int non_zero_counter = 4;
        do {
            total += 10;
            bar();
        } while (--non_zero_counter > 2);
    }
    
    /* ====== PATTERN 9: Loop followed by other statements ====== */
    /* Tests liveness analysis */
    {
        int live_counter = 7;
        int temp = 0;
        do {
            temp += live_counter;
            total += 11;
            bar();
        } while (--live_counter > 0);
        total += temp;  /* Use temp to affect register allocation */
    }
    
    /* ====== PATTERN 10: Volatile counter (may inhibit pattern) ====== */
    /* Testing pattern matcher's resilience */
    {
        volatile int vol_counter = 6;
        do {
            total += 12;
            bar();
        } while (--vol_counter > 0);
    }
    
    return total;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const, non-pure function */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    int result = accumulate_results();
    
    /* Print predictable result for verification */
    printf("Result: %d\n", result);
    
    /* Expected calculation:
       Pattern 1: 100 * 1 = 100
       Pattern 2: 50 * 2 = 100
       Pattern 3: 25 * 3 = 75
       Pattern 4: 10 * 4 = 40
       Pattern 5: 5 * 5 = 25
       Pattern 6: 8 * 6 = 48
       Pattern 7: 1 * 7 = 7
       Pattern 8: 12 * 8 = 96
       Negative 1: 1 * 9 = 9 (only executes once due to post-increment)
       Negative 2: 2 * 10 = 20 (decrements from 4 to 2)
       Pattern 9: 7 * 11 = 77 + sum(1..7)=28 = 105
       Pattern 10: 6 * 12 = 72
       Total: 100+100+75+40+25+48+7+96+9+20+105+72 = 697
    */
    
    return 0;
}
