/* loop-doloop-pattern-test.c
 * 
 * This program is designed to generate specific RTL patterns that trigger
 * the doloop optimization pass in GCC, specifically targeting the uncovered
 * lines in loop-doloop.cc (lines 136-150).
 * 
 * The patterns aim to produce COMPARE(PLUS(reg, -1), 0) sequences from
 * do-while loops with decrementing counters.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop elimination */
extern void bar(void);

int main(void) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    
    /* ===== PATTERN 1: Basic signed int decrement =====
     * Should produce: do { ... } while (--counter > 0);
     * Expected RTL: COMPARE(PLUS(counter_reg, -1), 0)
     */
    {
        int counter = 100;
        int sum = 0;
        do {
            sum += counter;      /* Simple side effect */
            bar();               /* External call prevents elimination */
        } while (--counter > 0);
        total += sum;
        printf("Pattern 1 sum: %d\n", sum);
    }
    
    /* ===== PATTERN 2: Unsigned int with != 0 comparison =====
     * Should produce: do { ... } while (--u_counter != 0);
     */
    {
        unsigned int u_counter = 50;
        register int reg_sum = 0;  /* register variable */
        do {
            reg_sum += (int)u_counter;
            bar();
        } while (--u_counter != 0);
        total += reg_sum;
        printf("Pattern 2 sum: %d\n", reg_sum);
    }
    
    /* ===== PATTERN 3: Short type with explicit assignment =====
     * Should produce: do { ... } while ((counter -= 1) != 0);
     */
    {
        short counter = 25;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while ((counter -= 1) != 0);
        total += sum;
        printf("Pattern 3 sum: %d\n", sum);
    }
    
    /* ===== PATTERN 4: Char type in register storage class =====
     * register keyword encourages register allocation
     */
    {
        register char counter = 10;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 0);
        total += sum;
        printf("Pattern 4 sum: %d\n", sum);
    }
    
    /* ===== PATTERN 5: Loop inside conditional branch =====
     * Tests context sensitivity of doloop pass
     */
    {
        int flag = 1;
        if (flag) {
            int counter = 15;
            int sum = 0;
            do {
                sum += counter;
                bar();
            } while (--counter > 0);
            total += sum;
            printf("Pattern 5 sum: %d\n", sum);
        }
    }
    
    /* ===== PATTERN 6: Counter as function parameter (simulated) =====
     * Parameter might be in different register/stack location
     */
    {
        int param_counter = 20;
        int sum = 0;
        /* Simulate parameter by using it immediately */
        do {
            sum += param_counter;
            bar();
        } while (--param_counter > 0);
        total += sum;
        printf("Pattern 6 sum: %d\n", sum);
    }
    
    /* ===== PATTERN 7: Counter starting at 1 (boundary case) =====
     * Loop executes exactly once
     */
    {
        int counter = 1;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 0);
        total += sum;
        printf("Pattern 7 sum: %d\n", sum);
    }
    
    /* ===== PATTERN 8: Pointer-based side effect =====
     * Different loop body might affect register pressure
     */
    {
        int counter = 30;
        int array[1] = {0};
        int *ptr = array;
        do {
            *ptr += counter;  /* Side effect through pointer */
            bar();
        } while (--counter > 0);
        total += array[0];
        printf("Pattern 8 sum: %d\n", array[0]);
    }
    
    /* ===== NEGATIVE TEST 1: Post-increment (should NOT match) =====
     * Should fail: PLUS operand is +1, not -1
     */
    {
        int counter = 10;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (counter++ < 20);  /* Wrong direction for pattern */
        total += sum;
        printf("Negative test 1 sum: %d\n", sum);
    }
    
    /* ===== NEGATIVE TEST 2: Compare against non-zero (should NOT match) =====
     * Should fail: cmp_arg2 != const0_rtx
     */
    {
        int counter = 10;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 5);  /* Compare against 5, not 0 */
        total += sum;
        printf("Negative test 2 sum: %d\n", sum);
    }
    
    /* ===== NEGATIVE TEST 3: Volatile counter (should inhibit pattern) =====
     * Volatile access prevents the PLUS(-1) optimization
     */
    {
        volatile int counter = 10;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 0);
        vol_total += sum;
        printf("Negative test 3 sum: %d\n", sum);
    }
    
    /* Final output to ensure computations aren't eliminated */
    printf("Total accumulation: %d\n", total);
    printf("Volatile total: %d\n", vol_total);
    
    return total > 0 ? 0 : 1;  /* Return non-zero if all loops were eliminated */
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
