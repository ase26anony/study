/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Function with O2 optimization to ensure reorg pass runs */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource set for parent instruction */
    int res1 = ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    z = x * y;
    ext_func2(z);
    
target_label1:
    /* Candidate for delay slot filling - simple arithmetic */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    c = b + 15;  /* Simple addition, no trapping */
    
    /* Use result to prevent dead code elimination */
    result += c + res1;
}

__attribute__((optimize("O2")))
void test_pattern2(void) {
    volatile int p = 100, q = 200, r = 0;
    volatile int m = 50, n = 25;
    
    /* Loop to create more complex control flow */
    for (volatile int i = 0; i < 3; i++) {
        int res2 = ext_func2(p);
        
        /* Multiple conditional jumps */
        if (cond2) {
            goto target_label2;
        }
        
        if (cond3) {
            __asm__ volatile ("" : : : "memory");
            goto target_label2;
        }
        
        /* Intermediate computation */
        m = n * 2;
        continue;
        
    target_label2:
        /* Safe assignment - no division or trapping operations */
        r = q - 45;
        
        result += r + res2 + i;
        
        /* Break to prevent infinite loop */
        if (i > 1) break;
    }
}

__attribute__((optimize("O3")))
void test_pattern3(void) {
    volatile int var1 = 1000, var2 = 2000, var3 = 0;
    volatile int temp1 = 100, temp2 = 200;
    
    /* Switch statement for varied control flow */
    volatile int choice = 2;
    
    switch (choice) {
        case 1:
            ext_func1(var1);
            if (cond4) {
                goto target_label3;
            }
            break;
        case 2:
            /* This is the path we'll take */
            __asm__ volatile ("" : : : "memory");
            if (cond1) {
                goto target_label3;
            }
            break;
        case 3:
            temp1 = temp2 * 3;
            goto target_label3;
        default:
            return;
    }
    
    /* Some code that won't be executed but prevents optimization */
    var3 = var1 / 1;  /* Safe division by 1 */
    
target_label3:
    /* Simple memory operation - assignment */
    var3 = var2 + 500;
    
    result += var3;
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    volatile int u = 300, v = 400, w = 0;
    volatile int control = 1;
    
    /* Labels for computed goto */
    void* labels[] = { &&label1, &&label2, &&target_label4 };
    
    /* Resource computation before jump */
    int res3 = ext_func3(u);
    
    /* Computed goto */
    goto *labels[control];
    
label1:
    v = v * 2;
    return;
    
label2:
    w = u - v;
    return;
    
target_label4:
    /* Safe operation - subtraction */
    w = v - u;
    
    result += w + res3;
}

/* Function with nested control flow */
__attribute__((optimize("O2")))
void test_pattern5(void) {
    volatile int counter = 0;
    volatile int val1 = 10, val2 = 20, val3 = 0;
    
    while (counter < 2) {
        /* Multiple external calls to create resource complexity */
        int res_before = ext_func1(counter);
        
        /* Deeply nested conditionals */
        if (cond1) {
            if (cond3) {
                __asm__ volatile ("" : : : "memory");
                for (volatile int j = 0; j < 1; j++) {
                    if (cond2 || !cond4) {
                        goto target_label5;
                    }
                }
            }
        }
        
        /* Alternative path */
        val3 = val1 * val2;
        ext_func2(val3);
        
        continue;  /* Skip the target code if not jumping */
        
    target_label5:
        /* Simple assignment - no function calls, no complex ops */
        val3 = val2 + val1;
        
        result += val3 + res_before + counter;
        
        counter++;
    }
}

/* External function declarations (simulated) */
int ext_func1(int x) {
    /* Simulate side effects */
    static int state = 0;
    state += x;
    return state;
}

int ext_func2(int x) {
    /* Another simulated external */
    return x ^ 0x55AA;
}

int ext_func3(int x) {
    /* Third simulated external */
    return x * 2;
}

int main(void) {
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Execute all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    /* Additional patterns with different optimization attributes */
    for (volatile int i = 0; i < 2; i++) {
        /* Vary conditions dynamically */
        cond1 = i & 1;
        cond2 = (i >> 1) & 1;
        
        test_pattern1();
        test_pattern2();
    }
    
    printf("Result: %d\n", result);
    printf("Test patterns completed.\n");
    
    return 0;
}
