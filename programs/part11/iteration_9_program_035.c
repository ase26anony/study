/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);
extern void ext_side_effect(void);

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

volatile int result = 0;

/* Function with attribute to force O2 optimization */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int local_cond = cond1;
    volatile int local_a = var_a;
    volatile int local_b = var_b;
    volatile int local_c = var_c;
    
    /* Create artificial resource constraints with inline assembly */
    asm volatile ("" : : : "memory");
    
    if (local_cond) {
        /* Jump to label with simple arithmetic at target */
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    ext_func1(local_a);
    local_b = local_a + 5;
    
target_label1:
    /* Simple arithmetic - candidate for delay slot */
    local_c = local_a + local_b;
    
    /* External call creates resource barrier */
    ext_func2(local_c);
    
    result += local_c;
}

/* Function with different optimization level */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int local_cond = cond2;
    volatile int local_d = var_d;
    volatile int local_e = var_e;
    volatile int local_f = var_f;
    
    /* Loop to create more complex control flow */
    for (int i = 0; i < 3; i++) {
        /* Inline assembly to prevent optimizations */
        asm volatile ("" : : : "memory");
        
        if (local_cond) {
            goto target_label2;
        }
        
        /* Different computation path */
        local_d = ext_func3(local_d);
        if (i % 2 == 0) {
            local_e = local_d * 2;
        }
    }
    
    /* Unreachable code to create dead branch */
    local_f = 999;
    return;
    
target_label2:
    /* Safe non-trapping operation */
    local_f = local_d - local_e;
    
    /* Multiple external calls for resource separation */
    ext_side_effect();
    ext_func1(local_f);
    
    result += local_f;
}

/* Function with switch statement */
__attribute__((optimize("O2")))
void test_pattern3(void) {
    volatile int local_cond = cond3;
    volatile int local_g = var_g;
    volatile int local_h = var_h;
    volatile int local_i = var_i;
    
    /* Switch creates multiple jump targets */
    switch (local_cond) {
        case 0:
            local_g = 1;
            break;
        case 1:
            /* Jump to label from within switch case */
            if (local_g > 0) {
                goto target_label3;
            }
            local_h = local_g * 3;
            break;
        default:
            local_i = 100;
    }
    
    /* Some computation */
    local_h = ext_func2(local_g);
    
    /* Another jump opportunity */
    if (local_h < 50) {
        goto skip_label;
    }
    
    local_i = local_g + local_h;
    
skip_label:
    /* More code to prevent optimization */
    asm volatile ("" : : : "memory");
    return;
    
target_label3:
    /* Simple assignment - good delay slot candidate */
    local_i = local_g + 10;
    
    result += local_i;
}

/* Function using computed goto */
__attribute__((optimize("O2")))
void test_pattern4(void) {
    volatile int local_cond = cond4;
    volatile int local_j = var_j;
    volatile int local_k = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label1, &&label2, &&target_label4 };
    
    /* Inline assembly for resource tracking */
    asm volatile ("" : : : "memory");
    
    if (local_cond) {
        /* Direct goto */
        goto target_label4;
    }
    
    /* Computed goto */
    goto *labels[local_cond % 3];
    
label1:
    local_j = ext_func1(local_j);
    goto end;
    
label2:
    local_j = ext_func2(local_j);
    goto end;
    
target_label4:
    /* Simple arithmetic with no resource conflicts */
    local_k = local_j + 25;
    
    /* External call separates resources */
    ext_func3(local_k);
    
    result += local_k;
    return;
    
end:
    local_k = local_j * 2;
    result += local_k;
}

/* Nested control flow pattern */
__attribute__((optimize("O3")))
void test_pattern5(void) {
    volatile int local_cond = cond5;
    volatile int local_m = 15;
    volatile int local_n = 25;
    volatile int local_o = 35;
    
    /* Nested loops */
    for (int i = 0; i < 2; i++) {
        asm volatile ("" : : : "memory");
        
        for (int j = 0; j < 2; j++) {
            /* Multiple conditional jumps */
            if (local_cond && (i == j)) {
                goto target_label5;
            }
            
            local_m = ext_func1(local_m + j);
        }
        
        /* Switch inside loop */
        switch (i) {
            case 0:
                local_n = local_m + 1;
                break;
            case 1:
                if (local_n > 20) {
                    /* Another jump opportunity */
                    goto intermediate_label;
                }
                break;
        }
        
        continue;
        
    intermediate_label:
        local_o = local_n * 2;
    }
    
    return;
    
target_label5:
    /* Safe operation - addition with constants */
    local_o = local_m + local_n + 5;
    
    /* Multiple external calls to create resource barriers */
    ext_side_effect();
    ext_func1(local_o);
    ext_func2(local_o);
    
    result += local_o;
}

/* Function with multiple jump-to-label patterns */
__attribute__((optimize("O2")))
void test_pattern6(void) {
    volatile int local_p = 100;
    volatile int local_q = 200;
    volatile int local_r = 300;
    
    /* Pattern A */
    asm volatile ("" : : : "memory");
    if (cond1) {
        goto target_a;
    }
    local_p = ext_func1(local_p);
    
target_a:
    local_q = local_p + 50;
    ext_func2(local_q);
    
    /* Pattern B */
    asm volatile ("" : : : "memory");
    if (cond2) {
        goto target_b;
    }
    local_r = ext_func3(local_r);
    
target_b:
    local_p = local_q - local_r;
    
    /* Pattern C - inside if-else */
    asm volatile ("" : : : "memory");
    if (cond3) {
        if (cond4) {
            goto target_c;
        }
        local_q = local_p * 2;
    } else {
        local_r = local_q / 2;
    }
    
target_c:
    local_r = local_p + local_q + 10;
    
    result += local_p + local_q + local_r;
}

/* External function definitions (simulated) */
int ext_func1(int x) {
    /* Simulate side effects */
    asm volatile ("" : : : "memory");
    return x + 1;
}

int ext_func2(int x) {
    asm volatile ("" : : : "memory");
    return x * 2;
}

int ext_func3(int x) {
    asm volatile ("" : : : "memory");
    return x - 3;
}

void ext_side_effect(void) {
    asm volatile ("" : : : "memory");
}

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile variables with non-deterministic values */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    cond5 = rand() % 2;
    
    var_a = rand() % 100;
    var_b = rand() % 100;
    var_c = rand() % 100;
    var_d = rand() % 100;
    var_e = rand() % 100;
    
    /* Execute test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    test_pattern6();
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
