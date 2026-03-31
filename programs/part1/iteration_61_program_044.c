#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function using various do-while patterns */
void test_loops(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    int *ptr = &total;
    
    /* Pattern 1: Basic signed int decrement (should match) */
    int counter1 = 100;
    do {
        total += 1;
        bar();
    } while (--counter1 > 0);
    
    /* Pattern 2: Unsigned int with != 0 comparison (should match) */
    unsigned int counter2 = 50;
    do {
        *ptr += 2;
        bar();
    } while (--counter2 != 0);
    
    /* Pattern 3: Short type with explicit decrement (should match) */
    short counter3 = 25;
    do {
        total += 3;
        bar();
    } while ((counter3 -= 1) != 0);
    
    /* Pattern 4: Char type with register qualifier (should match) */
    register char counter4 = 10;
    do {
        vol_total += 4;
        bar();
    } while (--counter4 > 0);
    
    /* Pattern 5: Counter starts at 1 (boundary case) */
    int counter5 = 1;
    do {
        total += 5;
        bar();
    } while (--counter5 > 0);
    
    /* Pattern 6: Function parameter as counter */
    int counter6 = param_counter;
    if (counter6 > 0) {
        do {
            total += 6;
            bar();
        } while (--counter6 > 0);
    }
    
    /* Pattern 7: Inside if statement with different type */
    if (total > 0) {
        unsigned short counter7 = 15;
        do {
            total += 7;
            bar();
        } while (--counter7 != 0);
    }
    
    /* Pattern 8: Followed by other statements */
    int counter8 = 20;
    do {
        total += 8;
        bar();
    } while (--counter8 > 0);
    
    /* Additional statements affecting register allocation */
    int temp = total * 2;
    vol_total += temp;
    
    /* Pattern 9: Nested in another control flow */
    for (int i = 0; i < 2; i++) {
        int counter9 = 5;
        do {
            total += 9;
            bar();
        } while (--counter9 > 0);
    }
    
    /* NON-MATCHING PATTERNS (for contrast) */
    
    /* Pattern A: Post-increment (should NOT match - wrong operator) */
    int counterA = 10;
    do {
        total += 10;
        bar();
    } while (counterA++ < 20);
    
    /* Pattern B: Compare against non-zero (should NOT match) */
    int counterB = 30;
    do {
        total += 11;
        bar();
    } while (--counterB > 5);
    
    /* Pattern C: Complex expression (should NOT match) */
    int counterC = 40;
    do {
        total += 12;
        bar();
    } while ((counterC -= 2) != 0);
    
    printf("Result: %d (volatile: %d)\n", total, vol_total);
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    /* Test with different parameter values */
    test_loops(8);
    test_loops(0);
    test_loops(3);
    
    return 0;
}
