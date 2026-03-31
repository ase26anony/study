#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Global pointer for side effects */
int global_sum = 0;

/* Function with various do-while loops targeting the RTL pattern */
void test_doloop_patterns(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* volatile to prevent some optimizations */
    int *ptr = &global_sum;
    
    /* Pattern 1: Basic signed int decrement with > 0 condition */
    /* Should match: do { ... } while (--counter > 0); */
    {
        int counter1 = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter1 > 0);
    }
    
    /* Pattern 2: Unsigned int decrement with != 0 condition */
    /* Should match: do { ... } while (--counter != 0); */
    {
        unsigned int counter2 = 50;
        do {
            *ptr = *ptr + 1;  /* Pointer dereference for side effect */
            total += 2;
        } while (--counter2 != 0);
    }
    
    /* Pattern 3: register qualified variable */
    /* Should match if register allocation works */
    {
        register int counter3 = 25;
        do {
            vol_total = vol_total + 3;  /* volatile operation */
            total += 3;
        } while (--counter3 > 0);
    }
    
    /* Pattern 4: short type counter */
    /* Should match with type conversion */
    {
        short counter4 = 10;
        do {
            bar();
            total += 4;
        } while (--counter4 > 0);
    }
    
    /* Pattern 5: char type counter */
    /* Should match with type conversion */
    {
        char counter5 = 5;
        do {
            *ptr = *ptr + 1;
            total += 5;
        } while (--counter5 > 0);
    }
    
    /* Pattern 6: Counter as function parameter */
    /* Tests parameter handling in doloop */
    {
        int counter6 = param_counter;
        if (counter6 > 0) {
            do {
                total += 6;
                bar();
            } while (--counter6 > 0);
        }
    }
    
    /* Pattern 7: Loop inside if statement */
    /* Tests context sensitivity */
    {
        int counter7 = 8;
        if (total < 1000) {  /* Always true, but creates control flow context */
            do {
                total += 7;
                *ptr = *ptr + 1;
            } while (--counter7 > 0);
        }
    }
    
    /* Pattern 8: Counter starting at 1 (edge case) */
    /* Should execute exactly once */
    {
        int counter8 = 1;
        do {
            total += 8;
            bar();
        } while (--counter8 > 0);
    }
    
    /* Pattern 9: Explicit subtraction form */
    /* do { ... } while ((counter -= 1) != 0); */
    {
        int counter9 = 15;
        do {
            total += 9;
            *ptr = *ptr + 1;
        } while ((counter9 -= 1) != 0);
    }
    
    /* Pattern 10: Nested in another control flow with post-loop statements */
    {
        int counter10 = 12;
        int temp = 0;
        do {
            temp++;
            total += 10;
            bar();
        } while (--counter10 > 0);
        
        /* Additional statements affecting register liveness */
        vol_total = temp * 2;
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment instead of decrement */
    /* Should fail: GEN_INT(-1) check */
    {
        int counterA = 10;
        int i = 0;
        do {
            total += 100;
            i++;
        } while (i < counterA);  /* Not a decrement pattern */
    }
    
    /* Pattern B: Compare against non-zero value */
    /* Should fail: cmp_arg2 != const0_rtx check */
    {
        int counterB = 10;
        do {
            total += 200;
            bar();
        } while (--counterB > 5);  /* Compare against 5, not 0 */
    }
    
    /* Pattern C: Complex expression in condition */
    /* Should fail: simple PLUS check */
    {
        int counterC = 10;
        int other = 5;
        do {
            total += 300;
        } while ((counterC -= other) > 0);  /* Not -1 decrement */
    }
    
    /* Pattern D: volatile counter (likely won't match) */
    {
        volatile int counterD = 10;
        do {
            total += 400;
        } while (--counterD > 0);  /* volatile might prevent pattern */
    }
    
    printf("Result: %d\n", total);
    printf("Global sum: %d\n", global_sum);
    printf("Volatile total: %d\n", vol_total);
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Minimal side effect */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    /* Initialize global pointer */
    global_sum = 0;
    
    /* Test with various parameter values */
    test_doloop_patterns(7);  /* Prime number to avoid obvious patterns */
    
    /* Additional test with different parameter */
    test_doloop_patterns(1);  /* Edge case */
    
    return 0;
}
