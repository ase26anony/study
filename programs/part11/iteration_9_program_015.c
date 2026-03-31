/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables for jump conditions */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;

/* Distinct volatile variables for different computations */
volatile int var_a = 10;
volatile int var_b = 20;
volatile int var_c = 30;
volatile int var_d = 40;
volatile int var_e = 50;
volatile int var_f = 60;
volatile int var_g = 70;
volatile int var_h = 80;

/* Result accumulator */
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_function1(void) {
    int temp;
    
    /* Parent instruction computation - uses var_a, var_b */
    temp = var_a + var_b;
    
    /* Call external function to create resource barrier */
    temp = ext_func1(temp);
    
    /* Conditional jump to label */
    if (cond1) {
        /* Inline assembly to create artificial resource constraints */
        __asm__ volatile ("" : : : "memory");
        goto target_label1;
    }
    
    /* Some intermediate code */
    var_c = var_d * 2;
    
target_label1:
    /* Target instruction - uses var_e, var_f (distinct from parent) */
    /* Simple arithmetic, non-trapping, non-jump */
    var_g = var_e + var_f;
    
    /* Another external call */
    result += ext_func2(var_g);
}

__attribute__((optimize("O3")))
void test_function2(void) {
    volatile int local_cond = 1;
    int temp1, temp2;
    
    /* More complex parent computation */
    temp1 = var_a * var_b;
    temp2 = var_c - var_d;
    
    /* Multiple external calls */
    temp1 = ext_func1(temp1);
    temp2 = ext_func2(temp2);
    
    /* Nested control flow */
    for (int i = 0; i < 3; i++) {
        /* Switch statement to create varied control flow */
        switch (i) {
            case 0:
                if (local_cond) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            case 1:
                temp1 = ext_func3(temp1);
                break;
            default:
                break;
        }
        
        /* Intermediate computation */
        var_h = var_g + i;
    }
    
    return;
    
target_label2:
    /* Target instruction - simple assignment */
    var_f = var_e;
    
    /* Safe operation - no division to avoid trapping */
    result += var_f * 2;
}

/* Function with computed goto */
__attribute__((optimize("O2")))
void test_function3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    int index = cond3 ? 0 : 1;
    
    /* Parent computation with inline assembly */
    int parent_val = var_a * var_b;
    __asm__ volatile ("# Resource marker" : : : "memory");
    
    /* Computed goto */
    goto *labels[index];
    
label_a:
    /* Intermediate block */
    var_c = ext_func1(var_c);
    goto end;
    
label_b:
    /* Target label for delay slot candidate */
    /* Simple, safe operation */
    var_d = var_e + 1;
    
    /* External call after target */
    result += ext_func2(var_d);
    goto end;
    
label_c:
    var_f = var_g - 1;
    goto end;
    
end:
    return;
}

/* Function with multiple jump-to-label patterns */
__attribute__((optimize("O2")))
void test_function4(void) {
    int counter = 0;
    
    while (counter < 5) {
        int temp = var_a + counter;
        
        /* First jump pattern */
        if (cond1 && counter % 2 == 0) {
            __asm__ volatile ("" : : : "memory");
            goto target_a;
        }
        
        /* Intermediate computation */
        temp = ext_func1(temp);
        
        /* Second jump pattern */
        if (cond2 || counter > 2) {
            goto target_b;
        }
        
        counter++;
        continue;
        
    target_a:
        /* First target - simple arithmetic */
        var_b = var_c + var_d;
        result += var_b;
        counter++;
        continue;
        
    target_b:
        /* Second target - assignment */
        var_e = var_f;
        result += ext_func3(var_e);
        counter += 2;
        continue;
    }
}

/* Function designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2")))
void test_mips_like(void) {
    volatile int mips_cond = 1;
    int reg1, reg2, reg3;
    
    /* Simulate register operations */
    reg1 = var_a;
    reg2 = var_b;
    
    /* Parent instruction that might have delay slot */
    reg3 = reg1 * reg2;
    
    /* External call as barrier */
    reg3 = ext_func1(reg3);
    
    /* Conditional branch to label */
    if (mips_cond) {
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        goto mips_target;
    }
    
    /* Fall-through path */
    reg1 = ext_func2(reg1);
    return;
    
mips_target:
    /* Instruction immediately after label - delay slot candidate */
    /* Uses different registers than parent */
    var_g = var_h + 100;
    
    /* Safe, non-trapping operation */
    result += var_g;
}

/* Main function that exercises all patterns */
int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile variables with non-zero values */
    var_a = 100;
    var_b = 200;
    var_c = 300;
    var_d = 400;
    var_e = 500;
    var_f = 600;
    var_g = 700;
    var_h = 800;
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 1;
    
    /* Execute test functions multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        test_function1();
        test_function2();
        test_function3();
        test_function4();
        test_mips_like();
        
        /* Vary conditions */
        cond1 = !cond1;
        cond2 = i % 2;
        var_a += 10;
        var_b += 20;
    }
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 1;
}
