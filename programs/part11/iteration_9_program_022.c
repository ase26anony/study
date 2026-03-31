/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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
volatile int cond5 = 1;

/* Distinct volatile variables for resource separation */
volatile int var_a = 10;
volatile int var_b = 20;
volatile int var_c = 30;
volatile int var_d = 40;
volatile int var_e = 50;
volatile int var_f = 60;
volatile int var_g = 70;
volatile int var_h = 80;
volatile int var_i = 90;
volatile int var_j = 100;

/* Global accumulator to ensure execution */
volatile int global_result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    int local1 = var_a;
    int local2 = var_b;
    
    /* Resource-intensive computation for parent instruction */
    int parent_compute = (local1 * local2) + (var_c / 2);
    
    /* External call creates resource barrier */
    ext_func1(parent_compute);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    var_d = var_e + var_f;
    ext_func2(var_d);
    
target_label1:
    /* Simple non-jump, non-trapping instruction at target */
    /* Uses distinct variables to avoid resource conflicts */
    int target_compute = var_g + var_h;
    global_result += target_compute;
    
    /* More code after label */
    var_i = var_j - 5;
    ext_func3(var_i);
}

/* Another pattern with switch statement */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int switch_var = 2;
    volatile int control = 1;
    
    for (int i = 0; i < 3; i++) {
        switch (switch_var) {
            case 1:
                var_a = var_b * 2;
                break;
            case 2:
                /* Jump to label inside switch case */
                if (control) {
                    goto target_label2;
                }
                var_c = var_d + 1;
                break;
            case 3:
                var_e = var_f - 1;
                break;
        }
        
        /* Some loop computation */
        var_g += i;
    }
    
    return; /* Early return to avoid label */
    
target_label2:
    /* Safe arithmetic operation - no division to avoid trapping */
    int safe_op = (var_h << 1) | (var_i & 0xFF);
    global_result += safe_op;
    
    /* Continue execution */
    for (int j = 0; j < 2; j++) {
        var_j += j;
    }
}

/* Pattern with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int idx = 1;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        : 
        : "r" (var_a)
        : "%eax", "memory"
    );
    
    /* Computed goto */
    goto *labels[idx];
    
label_a:
    var_b = var_c + 10;
    return;
    
label_b:
    /* Target instruction for delay slot candidate */
    /* Simple assignment with no resource conflicts */
    int simple_assign = var_d ^ var_e;
    global_result ^= simple_assign;
    
    /* External call separates resources */
    ext_func2(simple_assign);
    return;
    
label_c:
    var_f = var_g * 2;
    return;
}

/* Pattern with nested loops and multiple labels */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int outer_cond = 1;
    volatile int inner_cond = 0;
    
    for (int i = 0; i < 4; i++) {
        /* First computation block */
        int block1 = var_a + i;
        ext_func1(block1);
        
        for (int j = 0; j < 2; j++) {
            /* Conditional jump inside nested loop */
            if (inner_cond) {
                goto target_label4;
            }
            
            int block2 = var_b * j;
            var_c += block2;
        }
        
        /* Change conditions */
        inner_cond = !inner_cond;
    }
    
    /* Fall-through path */
    var_d = var_e >> 1;
    return;
    
target_label4:
    /* Simple non-trapping instruction */
    /* Uses variables not used in parent computation */
    int safe_compute = (var_f & 0x0F) + (var_g & 0xF0);
    global_result |= safe_compute;
    
    /* Continue with more operations */
    for (int k = 0; k < 3; k++) {
        var_h += k;
        ext_func3(var_h);
    }
}

/* Pattern with mixed control flow */
__attribute__((optimize("O3"), noinline))
void test_pattern5(void) {
    volatile int flags[4] = {1, 0, 1, 0};
    
    /* Multiple potential jump targets */
    for (int i = 0; i < 4; i++) {
        if (flags[i]) {
            switch (i) {
                case 0:
                    if (cond2) goto target_label5a;
                    break;
                case 1:
                    var_a = ext_func1(var_b);
                    break;
                case 2:
                    if (cond3) goto target_label5b;
                    break;
                case 3:
                    var_c = ext_func2(var_d);
                    break;
            }
        }
        
        /* Intermediate computation */
        var_e += i * 2;
    }
    
    /* Default exit */
    var_f = var_g - 10;
    return;
    
target_label5a:
    /* First target - simple arithmetic */
    int result1 = var_h + var_i;
    global_result += result1;
    
    /* Jump to second target */
    if (cond4) goto target_label5b;
    
    var_j = 99;
    return;
    
target_label5b:
    /* Second target - bitwise operation */
    int result2 = var_a ^ var_b;
    global_result ^= result2;
    
    /* Final external call */
    ext_func3(global_result);
}

/* Main test driver */
int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile variables with non-zero values */
    var_a = 100; var_b = 200; var_c = 300;
    var_d = 400; var_e = 500; var_f = 600;
    var_g = 700; var_h = 800; var_i = 900; var_j = 1000;
    
    /* Run all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    /* Add some computation to use results */
    volatile int final_check = global_result;
    for (int i = 0; i < 10; i++) {
        final_check += i;
        if (i % 2 == 0) {
            final_check ^= var_a;
        } else {
            final_check |= var_b;
        }
    }
    
    printf("Final result: %d\n", final_check);
    printf("Global accumulator: %d\n", global_result);
    
    return final_check != 0 ? 0 : 1;
}

/* Dummy external function definitions */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x ^ 0x55AA;
}
