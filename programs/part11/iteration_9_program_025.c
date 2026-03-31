/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern void ext_side_effect(void);

/* Volatile control variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int result = 0;

/* Test function 1: Basic jump-to-label pattern */
__attribute__((optimize("O2")))
void test_basic_jump(void) {
    volatile int x = 0, y = 0, z = 0;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* Parent instruction for delay slot */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code to avoid fall-through */
    y = c * 2;
    ext_func2(y);
    
target_label1:
    /* Candidate for delay slot: simple arithmetic with different resources */
    z = d - 3;  /* Uses different volatile variables than parent */
    
    /* Ensure result is used */
    result += z;
    
    /* Another external call to prevent merging */
    ext_side_effect();
}

/* Test function 2: Nested control flow with switch */
__attribute__((optimize("O2")))
void test_switch_pattern(void) {
    volatile int i = 2, j = 0, k = 0;
    volatile int temp = 0;
    
    for (int iter = 0; iter < 3; iter++) {
        switch (i) {
            case 1:
                temp = a + b;
                break;
            case 2:
                /* Parent instruction */
                j = c * d;
                
                /* Inline assembly to create artificial resource constraints */
                __asm__ volatile ("" : : : "memory");
                
                /* Jump to label */
                if (cond2) {
                    goto target_label2;
                }
                
                /* Alternative path */
                k = b - a;
                break;
                
            case 3:
                temp = d / 2;
                break;
        }
        
        /* Continue loop */
        i = (i % 3) + 1;
    }
    
    return;
    
target_label2:
    /* Delay slot candidate: safe assignment */
    k = 42;  /* Constant assignment - no trapping */
    
    /* Use result */
    result += k;
}

/* Test function 3: Multiple labels with computed goto */
__attribute__((optimize("O3")))
void test_computed_goto(void) {
    volatile int val1 = 0, val2 = 0, val3 = 0;
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Parent instruction with distinct resources */
    val1 = ext_func3(a);
    
    /* Create jump based on condition */
    if (cond3) {
        goto *labels[1];  /* Jump to label_b */
    }
    
    /* Intermediate code */
    val2 = b + c;
    ext_func1(val2);
    
label_a:
    val3 = 100;
    goto end;
    
label_b:
    /* Target instruction for delay slot candidate */
    val3 = 50;  /* Simple assignment */
    
    /* Ensure no resource conflict with parent */
    __asm__ volatile ("" : : : "memory");
    
    goto end;
    
label_c:
    val3 = 75;
    
end:
    result += val3;
}

/* Test function 4: Loop with conditional exit to label */
__attribute__((optimize("O2")))
void test_loop_exit(void) {
    volatile int counter = 0;
    volatile int sum = 0;
    volatile int temp = 0;
    
    while (counter < 10) {
        /* Parent instruction */
        temp = a + counter;
        
        /* Conditional exit to label */
        if (cond4 && counter > 5) {
            goto exit_label;
        }
        
        /* Normal loop body */
        sum += temp;
        counter++;
        
        /* External call creates resource separation */
        if (counter % 3 == 0) {
            ext_func2(counter);
        }
    }
    
    result += sum;
    return;
    
exit_label:
    /* Delay slot candidate: safe arithmetic */
    sum += 1000;  /* Different operation than parent */
    
    result += sum;
}

/* Test function 5: Complex pattern with multiple jumps */
__attribute__((optimize("O3")))
void test_complex_pattern(void) {
    volatile int x = 0, y = 0, z = 0;
    volatile int *ptr = &x;
    
    /* Multiple parent-like instructions */
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                x = a * 2;
                /* Jump pattern 1 */
                if (cond1) goto target1;
                break;
            case 1:
                y = b + 3;
                /* Jump pattern 2 */
                if (cond2) goto target2;
                break;
            case 2:
                z = c - 1;
                /* Jump pattern 3 */
                if (cond3) goto target3;
                break;
        }
        
        /* Inline assembly between jumps */
        __asm__ volatile ("" : : : "memory");
    }
    
    result += x + y + z;
    return;
    
target1:
    /* Simple assignment - delay slot candidate */
    *ptr = 99;
    return;
    
target2:
    /* Safe arithmetic */
    y = x + 5;
    result += y;
    return;
    
target3:
    /* Another candidate */
    z = 77;
    result += z;
}

/* Dummy external functions */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 1;
}

void ext_side_effect(void) {
    static int counter = 0;
    counter++;
}

int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    
    /* Run test functions multiple times with different conditions */
    for (int i = 0; i < 5; i++) {
        test_basic_jump();
        test_switch_pattern();
        test_computed_goto();
        test_loop_exit();
        test_complex_pattern();
        
        /* Change conditions to explore different paths */
        cond1 = !cond1;
        cond2 = (i % 3 == 0);
        cond3 = (i < 2);
        cond4 = (i % 2 == 0);
    }
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
