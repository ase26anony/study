/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int cond1 = 0;
volatile int cond2 = 1;
volatile int cond3 = 0;
volatile int cond4 = 1;
volatile int x = 5, y = 10, z = 15;
volatile int a = 1, b = 2, c = 3, d = 4;
volatile int result = 0;

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_c = c;
    
    /* Create resource set for parent instruction */
    int parent_compute = local_a * local_b;
    
    /* External call before jump */
    ext_func1(parent_compute);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    local_c = ext_func2(local_c);
    
    /* Target label with simple non-jump instruction */
    target_label1:
    /* This should be the candidate for delay slot filling */
    /* Use different variables to avoid resource conflicts */
    volatile int local_d = d;
    volatile int local_x = x;
    local_d = local_x + local_d;  /* Simple addition, no traps */
    
    /* External call after target */
    ext_func3(local_d);
    
    result += local_d;
}

/* Function with more complex control flow */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int local_x = x;
    volatile int local_y = y;
    volatile int local_z = z;
    
    /* Loop with nested conditional */
    for (int i = 0; i < 3; i++) {
        int loop_compute = local_x * i;
        
        /* Switch statement to create multiple basic blocks */
        switch (i) {
            case 0:
                ext_func1(loop_compute);
                if (cond2) {
                    goto target_label2;
                }
                break;
            case 1:
                ext_func2(loop_compute);
                if (cond3) {
                    goto target_label2;
                }
                break;
            case 2:
                /* Multiple jumps to same label */
                if (cond4) {
                    goto target_label2;
                }
                break;
        }
        
        /* Continue loop if no jump taken */
        local_y += i;
        continue;
        
        target_label2:
        /* Candidate instruction - subtraction, no traps */
        volatile int temp = local_z;
        temp = temp - local_x;
        
        /* Memory barrier via inline assembly */
        __asm__ volatile ("" : : : "memory");
        
        result += temp;
        break;  /* Exit loop after taking jump */
    }
}

/* Function using computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    volatile int selector = cond1 ? 0 : 1;
    
    volatile int var1 = a;
    volatile int var2 = b;
    
    /* Parent instruction computation */
    int compute = var1 * 100 + var2;
    ext_func1(compute);
    
    /* Computed goto */
    goto *labels[selector];
    
    /* Unreachable code */
    var1 = ext_func2(var1);
    
label_a:
    /* Simple assignment candidate */
    volatile int var3 = c;
    var3 = var2;
    result += var3;
    return;
    
label_b:
    /* Another candidate */
    volatile int var4 = d;
    var4 = var1 + 1;
    result += var4;
    return;
    
label_c:
    /* Fallback */
    result += 1;
}

/* Function with multiple jump targets */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    volatile int counter = 0;
    volatile int local_a = a;
    volatile int local_b = b;
    
    /* Multiple conditional jumps in sequence */
    if (cond1) {
        ext_func1(local_a);
        goto target_a;
    }
    
    if (cond2) {
        ext_func2(local_b);
        goto target_b;
    }
    
    if (cond3) {
        goto target_c;
    }
    
    /* Default path */
    local_a = ext_func3(local_a);
    goto exit;
    
target_a:
    /* Candidate 1 - simple bitwise operation */
    volatile int temp1 = local_a;
    temp1 = temp1 & 0xFF;
    result += temp1;
    goto exit;
    
target_b:
    /* Candidate 2 - shift operation */
    volatile int temp2 = local_b;
    temp2 = temp2 << 2;
    result += temp2;
    goto exit;
    
target_c:
    /* Candidate 3 - increment */
    volatile int temp3 = counter;
    temp3 = temp3 + 1;
    result += temp3;
    /* Fall through */
    
exit:
    /* Ensure all paths reach here */
    return;
}

/* Function designed for MIPS-like delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_mips_pattern(void) {
    volatile int mips_var1 = x;
    volatile int mips_var2 = y;
    volatile int mips_var3 = z;
    
    /* Pattern similar to MIPS conditional branch */
    int compare = mips_var1 - mips_var2;
    
    /* Inline assembly to suggest delay slot consideration */
    __asm__ volatile (
        "/* Potential delay slot candidate */"
        : 
        : "r"(compare)
        : "memory"
    );
    
    if (compare > 0) {
        /* External call to separate resources */
        ext_func1(mips_var3);
        goto mips_target;
    }
    
    /* Alternative path */
    mips_var3 = ext_func2(mips_var3);
    return;
    
mips_target:
    /* Ideal delay slot candidate:
       - Simple operation
       - Different variables than parent
       - No traps
       - Not a jump
    */
    volatile int temp = mips_var3;
    temp = temp * 2;  /* Multiplication is safe with integers */
    
    /* Another memory barrier */
    __asm__ volatile ("" : : : "memory");
    
    result += temp;
}

/* Main test driver */
int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    
    /* Run all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_mips_pattern();
    
    /* Additional iterations with different conditions */
    for (int i = 0; i < 5; i++) {
        cond1 = i % 3 == 0;
        cond2 = i % 2 == 0;
        cond3 = i % 4 == 0;
        
        test_pattern1();
        test_pattern2();
    }
    
    printf("Result accumulated: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int arg) {
    return arg + 1;
}

int ext_func2(int arg) {
    return arg * 2;
}

int ext_func3(int arg) {
    return arg - 1;
}
