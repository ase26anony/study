/* test_reorg.c - Program to trigger delay slot filling logic in GCC's reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int result = 0;

/* Function with O2 optimization and jump-to-label pattern */
__attribute__((optimize("O2")))
void test_func1(void) {
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_result = 0;
    
    /* Resource setup for parent instruction */
    int temp1 = local_a * 2;
    
    /* External call to create resource separation */
    ext_func1(temp1);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    local_result += ext_func2(local_b);
    
target_label1:
    /* Simple non-jump, non-trapping instruction (delay slot candidate) */
    int temp2 = local_b + 3;  /* Does not reference/set parent's resources */
    
    /* Use the result */
    local_result += temp2;
    
    /* Another external call */
    ext_func3(local_result);
    
    result += local_result;
}

/* Function with switch statement and multiple labels */
__attribute__((optimize("O2"), noinline))
void test_func2(int mode) {
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    int compute = 0;
    
    /* Parent instruction computation */
    compute = x * y;
    
    switch (mode) {
        case 1:
            if (cond2) {
                goto target_label2;
            }
            compute += ext_func1(x);
            break;
            
        case 2:
            /* Another jump pattern */
            if (cond3) {
                goto target_label3;
            }
            compute += ext_func2(y);
            break;
            
        default:
            compute += 100;
    }
    
    /* Dead code that won't be reached due to jumps */
    compute += 999;
    
target_label2:
    /* Simple arithmetic - delay slot candidate */
    int simple_op = z - 5;  /* Uses different variable than parent */
    
    compute += simple_op;
    goto end;
    
target_label3:
    /* Another simple operation */
    int another_op = d + 7;  /* Different variable set */
    
    compute += another_op;
    
end:
    result += compute;
}

/* Function with loop and computed goto */
__attribute__((optimize("O3"), noinline))
void test_func3(int iterations) {
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Labels for computed goto */
    static void* targets[] = { &&label_a, &&label_b, &&label_c };
    
    /* Parent instruction with inline assembly for resource constraints */
    int base = a;
    asm volatile ("" : "+r" (base) : : "memory");
    
    for (int i = 0; i < iterations; i++) {
        counter++;
        
        /* Conditional jump inside loop */
        if (cond4 && (i % 2 == 0)) {
            goto loop_target;
        }
        
        /* Some computation */
        sum += ext_func1(i);
        continue;
        
    loop_target:
        /* Simple assignment - delay slot candidate */
        int loop_temp = b + i;  /* Different resources than parent */
        
        sum += loop_temp;
    }
    
    /* Computed goto pattern */
    int choice = iterations % 3;
    
    /* External call before jump */
    ext_func2(sum);
    
    goto *targets[choice];
    
label_a:
    /* Simple operation after label */
    int val_a = c * 2;
    sum += val_a;
    goto finish;
    
label_b:
    /* Another simple operation */
    int val_b = d / 2;  /* Division but with constant divisor (non-trapping) */
    sum += val_b;
    goto finish;
    
label_c:
    /* Memory operation */
    int val_c = a;
    val_c += 5;
    sum += val_c;
    
finish:
    result += sum;
}

/* Function with nested control flow */
__attribute__((optimize("O2"), noinline))
void test_func4(void) {
    volatile int p = a;
    volatile int q = b;
    volatile int r = c;
    int accum = 0;
    
    /* Complex parent instruction with inline assembly */
    int parent_res = p * q;
    asm volatile ("# Resource marker" : : "r" (parent_res) : "memory");
    
    /* Nested if statements */
    if (cond1) {
        ext_func1(p);
        
        if (cond2) {
            ext_func2(q);
            
            if (cond3) {
                goto deep_label;
            }
            
            accum += 50;
        }
        
        accum += 100;
    }
    
    /* This should be skipped by the jump */
    accum += 9999;
    
deep_label:
    /* Simple, safe operation - ideal delay slot candidate */
    int deep_temp = r + 42;  /* Completely separate from parent_res */
    
    accum += deep_temp;
    
    /* More operations to prevent tail merging */
    for (int i = 0; i < 3; i++) {
        accum += ext_func3(i);
    }
    
    result += accum;
}

/* Function that mimics delay slot patterns */
__attribute__((optimize("O2"), noinline))
void test_func5(void) {
    volatile int var1 = a;
    volatile int var2 = b;
    volatile int var3 = c;
    volatile int var4 = d;
    
    /* Parent instruction setup */
    int parent_op = var1 * var2;
    ext_func1(parent_op);
    
    /* Multiple jump possibilities */
    if (cond1) {
        if (cond2) {
            goto target_a;
        } else {
            goto target_b;
        }
    }
    
    /* Fall-through path */
    parent_op += var3;
    goto end;
    
target_a:
    /* Candidate instruction - uses var4, not var1/var2 */
    int candidate_a = var4 - 10;
    parent_op += candidate_a;
    goto end;
    
target_b:
    /* Another candidate */
    int candidate_b = var3 + var4;
    parent_op += candidate_b;
    
end:
    result += parent_op;
}

/* External function implementations */
int ext_func1(int x) {
    return x + 1;
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x - 3;
}

int main(void) {
    printf("Starting reorg pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 1;
    a = 5; b = 10; c = 15; d = 20;
    result = 0;
    
    /* Run test functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        test_func1();
        test_func2(i);
        test_func3(2 + i);
        test_func4();
        test_func5();
        
        /* Change conditions to exercise different paths */
        cond1 = !cond1;
        cond2 = !cond2;
        cond3 = !cond3;
        cond4 = !cond4;
    }
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
