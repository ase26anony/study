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
volatile int result = 0;

/* Function with jump-to-label pattern for delay slot candidate */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Create resource set for parent instruction */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        /* This goto creates a simplejump to label */
        goto target_label1;
    }
    
    /* Some intermediate code to prevent block merging */
    z = x * y;
    ext_func2(z);
    
target_label1:
    /* This is next_trial: non-jump, non-sequence, non-trapping instruction
       that doesn't conflict with parent instruction's resources */
    c = a + b;  /* Uses different variables than parent would use */
    
    /* Ensure result is used */
    result += c;
}

/* More complex pattern with nested control flow */
__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int i, j, k;
    volatile int arr[10] = {0};
    
    for (i = 0; i < 10; i++) {
        /* Multiple conditional jumps */
        if (cond2) {
            goto target_label2;
        }
        
        switch (i % 3) {
            case 0:
                if (cond3) {
                    /* Jump to label before simple arithmetic */
                    goto target_label2;
                }
                arr[i] = i * 2;
                break;
            case 1:
                arr[i] = i + 5;
                break;
            default:
                /* Another jump opportunity */
                if (cond4) {
                    goto target_label2;
                }
                arr[i] = i - 1;
        }
        
        /* Continue loop if no jump taken */
        ext_func1(arr[i]);
        continue;
        
    target_label2:
        /* Candidate for delay slot filling: simple assignment */
        j = i * 3;  /* Safe, non-trapping operation */
        k = j + 1;
        result += k;
        
        /* Break to avoid infinite loop in test */
        break;
    }
}

/* Function using computed goto (labels as values) */
__attribute__((optimize("O2"), __noinline__))
void test_pattern3(void) {
    volatile int m = 7, n = 13, p = 0;
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Create resource usage before jump */
    ext_func2(m);
    
    /* Conditional computed goto */
    if (cond1) {
        goto *labels[1];  /* Jump to label_b */
    }
    
    p = m * n;
    goto end;
    
label_a:
    /* Not used in this execution path */
    p = m - n;
    goto end;
    
label_b:
    /* This is the target: simple arithmetic with different vars */
    p = m + n;  /* Doesn't conflict with parent's resources */
    goto end;
    
label_c:
    p = m / (n != 0 ? n : 1);  /* Avoid division by zero */
    goto end;
    
end:
    result += p;
}

/* Function with inline assembly to influence resource analysis */
__attribute__((optimize("O3"), __noinline__))
void test_pattern4(void) {
    volatile int r1 = 100, r2 = 200, r3 = 0;
    
    /* Inline assembly creates memory clobber affecting resource sets */
    __asm__ volatile (
        "nop\n\t"
        "nop\n\t"
        : 
        : 
        : "memory"
    );
    
    /* Multiple jumps in sequence */
    if (cond1) {
        ext_func3(r1);
        goto target_label4;
    }
    
    if (cond2) {
        goto other_label;
    }
    
    r3 = r1 * r2;
    goto finish;
    
target_label4:
    /* Delay slot candidate: uses completely different computation */
    r3 = r2 - r1;  /* Simple subtraction, no traps */
    goto finish;
    
other_label:
    r3 = r1 + 50;
    
finish:
    /* More assembly to prevent optimization */
    __asm__ volatile (
        "nop"
        : 
        : 
        : "memory"
    );
    
    result += r3;
}

/* Function with multiple labels and jumps */
__attribute__((optimize("O2"), __noinline__))
void test_pattern5(int mode) {
    volatile int val1 = 42, val2 = 17, res = 0;
    
    /* External call creates resource barrier */
    val1 = ext_func1(val1);
    
    /* Complex conditional jumps */
    switch (mode) {
        case 1:
            if (cond1 && !cond2) {
                goto compute_label;
            }
            res = val1 * 2;
            break;
        case 2:
            if (cond3 || cond4) {
                goto compute_label;
            }
            res = val1 + val2;
            break;
        default:
            for (int i = 0; i < 3; i++) {
                if (i == 1) {
                    goto compute_label;
                }
                res += i;
            }
    }
    
    goto done;
    
compute_label:
    /* Target instruction for delay slot filling */
    res = val2 * 3;  /* Safe multiplication */
    
done:
    result += res;
}

/* Main function that exercises all patterns */
int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = rand() & 1;
    cond2 = rand() & 1;
    cond3 = rand() & 1;
    cond4 = rand() & 1;
    
    /* Execute test patterns multiple times */
    for (int i = 0; i < 5; i++) {
        test_pattern1();
        test_pattern2();
        test_pattern3();
        test_pattern4();
        test_pattern5(i % 3);
        
        /* Change conditions dynamically */
        cond1 ^= 1;
        cond2 = !cond3;
        cond3 = (cond1 | cond2) & 1;
        cond4 = (i & 1);
    }
    
    printf("Final result: %d\n", result);
    printf("Test completed.\n");
    
    return result != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
int ext_func1(int x) { return x + 1; }
int ext_func2(int x) { return x - 1; }
int ext_func3(int x) { return x * 2; }
