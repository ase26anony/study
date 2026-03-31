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

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 0;
    volatile int x = 5, y = 3, z = 0;
    
    /* Call external function before jump */
    ext_func1(a);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    c = a + b;
    ext_func2(c);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Use distinct variables to avoid resource conflicts */
    z = x + y;  /* Simple arithmetic - delay slot candidate */
    
    /* Call external function after target */
    ext_func3(z);
    
    result += z;
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int m = 100, n = 200;
    volatile int p = 30, q = 40, r = 0;
    volatile int counter = 0;
    
    /* Loop with nested conditional jumps */
    for (counter = 0; counter < 3; counter++) {
        ext_func1(counter);
        
        /* Switch statement to create complex control flow */
        switch (counter) {
            case 0:
                if (cond2) {
                    goto target_label2;
                }
                m = m * 2;
                break;
            case 1:
                if (cond3) {
                    goto target_label2;
                }
                n = n / 2;
                break;
            case 2:
                if (cond4) {
                    goto target_label2;
                }
                m = m + n;
                break;
        }
        
        /* Continue normal execution */
        ext_func2(m + n);
        continue;
        
    target_label2:
        /* Target instruction - simple assignment */
        r = p - q;  /* Different operation, different variables */
        ext_func3(r);
        result += r;
    }
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    volatile int i = 0, j = 0, k = 0;
    volatile int u = 8, v = 2, w = 0;
    
    static void *labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* Use inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    for (i = 0; i < 3; i++) {
        ext_func1(i);
        
        /* Conditional jump based on volatile */
        if (cond1 && !cond2) {
            goto *labels[i];
        }
        
        j = i * 10;
        ext_func2(j);
        continue;
        
    label_a:
        w = u * v;  /* Multiplication - safe operation */
        goto end_loop;
        
    label_b:
        w = u / (v + 1);  /* Division but safe (no divide by zero) */
        goto end_loop;
        
    label_c:
        w = u - v;  /* Subtraction */
        goto end_loop;
        
    end_loop:
        ext_func3(w);
        result += w;
        k++;
    }
}

/* Function with multiple jump targets */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    volatile int a1 = 15, b1 = 25;
    volatile int a2 = 35, b2 = 45;
    volatile int res1 = 0, res2 = 0;
    
    /* Multiple conditional jumps in sequence */
    ext_func1(a1);
    
    if (cond3) {
        goto target_a;
    }
    
    res1 = a1 + b1;
    ext_func2(res1);
    
    if (cond4) {
        goto target_b;
    }
    
    res2 = a2 - b2;
    goto finish;
    
target_a:
    /* First target - simple assignment */
    res1 = a1 * 2;  /* Different operation from parent */
    ext_func2(res1);
    
    /* Another conditional jump */
    if (cond1) {
        goto target_b;
    }
    
    res2 = b1 + 10;
    goto finish;
    
target_b:
    /* Second target - different simple operation */
    res2 = a2 / 5;  /* Safe division */
    ext_func3(res2);
    
finish:
    result += res1 + res2;
}

/* Function with mixed operations to avoid trapping */
__attribute__((optimize("O3")))
void test_pattern5(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int idx = 0;
    volatile int sum = 0;
    
    /* Complex control flow with safe operations */
    while (idx < 4) {
        ext_func1(idx);
        
        /* Conditional jump to label */
        if (cond1 || cond3) {
            goto compute_label;
        }
        
        /* Alternative path */
        sum += arr[idx] * 2;
        idx++;
        continue;
        
    compute_label:
        /* Safe computation at target - no trapping */
        sum += arr[idx] + 5;  /* Addition is always safe */
        ext_func2(sum);
        idx++;
    }
    
    result += sum;
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

int main(void) {
    printf("Starting delay slot pattern tests...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;
    cond2 = 0;
    cond3 = 1;
    cond4 = 0;
    result = 0;
    
    /* Run all test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    printf("Result: %d\n", result);
    printf("Tests completed.\n");
    
    return 0;
}
