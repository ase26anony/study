#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void bar(void);

/* Global pointer for side effects */
int global_counter = 0;

/* Function with various do-while loops targeting the RTL pattern */
void test_doloop_patterns(int param_counter) {
    int total = 0;
    int *ptr = &total;
    
    /* Pattern 1: Basic signed int decrement */
    int counter1 = 100;
    do {
        total += 1;
        bar();
    } while (--counter1 > 0);
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    unsigned int u_counter = 50;
    do {
        *ptr += 2;
        bar();
    } while (--u_counter != 0);
    
    /* Pattern 3: Short type with register qualifier */
    register short reg_counter = 25;
    do {
        total += 3;
        bar();
    } while (--reg_counter > 0);
    
    /* Pattern 4: Char type */
    char char_counter = 10;
    do {
        total += 4;
        bar();
    } while (--char_counter != 0);
    
    /* Pattern 5: Inside if statement */
    if (global_counter > 0) {
        int if_counter = 5;
        do {
            total += 5;
            bar();
        } while (--if_counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter */
    do {
        total += 6;
        bar();
    } while (--param_counter > 0);
    
    /* Pattern 7: Counter starting at 1 (executes once) */
    int single_counter = 1;
    do {
        total += 7;
        bar();
    } while (--single_counter > 0);
    
    /* Pattern 8: With explicit decrement in condition */
    int explicit_counter = 8;
    do {
        total += 8;
        bar();
    } while ((explicit_counter -= 1) != 0);
    
    /* Pattern 9: Followed by other statements */
    int followed_counter = 9;
    do {
        total += 9;
        bar();
    } while (--followed_counter > 0);
    /* Additional statement affecting register allocation */
    int temp = followed_counter * 2;
    (void)temp;
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern 10: Post-increment (should NOT match GEN_INT(-1)) */
    int post_counter = 3;
    do {
        total += 10;
        bar();
    } while (post_counter++ < 5);
    
    /* Pattern 11: Compare against non-zero (should fail const0_rtx check) */
    int non_zero_counter = 4;
    do {
        total += 11;
        bar();
    } while (--non_zero_counter > 2);
    
    /* Pattern 12: Volatile counter (may inhibit pattern) */
    volatile int vol_counter = 2;
    do {
        total += 12;
        bar();
    } while (--vol_counter > 0);
    
    /* Pattern 13: Different decrement amount */
    int dec_by_two = 6;
    do {
        total += 13;
        bar();
    } while ((dec_by_two -= 2) > 0);
    
    printf("Accumulated total: %d\n", total);
    global_counter = total;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Simple side effect */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    /* Call with different parameter values */
    test_doloop_patterns(7);
    test_doloop_patterns(3);
    test_doloop_patterns(0);
    
    printf("Final global counter: %d\n", global_counter);
    return 0;
}
