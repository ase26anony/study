/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

#include <stdio.h>
#include <stdlib.h>

/* External functions to create resource barriers */
extern int ext_func1(int);
extern int ext_func2(int);
extern int ext_func3(int);

/* Volatile control variables to prevent optimization */
volatile int cond1 = 1;
volatile int cond2 = 0;
volatile int cond3 = 1;
volatile int cond4 = 0;
volatile int result = 0;

/* Function with O2 optimization attribute to ensure reorg pass runs */
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
    
    /* Some intermediate code to avoid block merging */
    z = x * y;
    ext_func2(z);
    
target_label1:
    /* Non-jump, non-sequence instruction at target */
    /* Uses different variables than parent instruction to avoid resource conflicts */
    c = b + 15;  /* Simple arithmetic, no trapping */
    
    /* Use result to prevent dead code elimination */
    result += c + res1;
}

__attribute__((optimize("O2")))
void test_pattern2(void) {
    volatile int m = 100, n = 200, p = 0;
    volatile int r = 50, s = 25, t = 0;
    
    /* Different computation for parent instruction */
    int res2 = ext_func2(m);
    
    /* Nested control flow */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                if (cond2) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
                break;
            case 1:
                t = r - s;
                break;
            default:
                if (cond3) {
                    __asm__ volatile ("" : : : "memory");
                    goto target_label2;
                }
        }
        
        /* More intermediate code */
        ext_func3(i);
    }
    
    return;
    
target_label2:
    /* Another safe non-jump instruction */
    p = n * 2;  /* Multiplication is safe with these values */
    result += p + res2;
}

__attribute__((optimize("O3")))
void test_pattern3(void) {
    volatile int u = 1000, v = 2000, w = 0;
    volatile int control = 1;
    
    /* Parent instruction with memory operation */
    int res3 = ext_func3(u);
    
    /* Computed goto using && labels */
    static void* labels[] = { &&label_a, &&label_b, &&target_label3 };
    
    if (control) {
        __asm__ volatile ("" : : : "memory");
        goto *labels[2];  /* Jump directly to target */
    }
    
label_a:
    w = v / 10;  /* Safe division */
    goto end;
    
label_b:
    w = v + 100;
    goto end;
    
target_label3:
    /* Simple assignment with no trapping */
    w = v - 500;
    
end:
    result += w + res3;
}

/* Function with loop containing multiple jump patterns */
__attribute__((optimize("O2"), noinline))
void test_pattern4(int iterations) {
    volatile int arr[4] = {10, 20, 30, 40};
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        volatile int temp = arr[i % 4];
        int res = ext_func1(temp);
        
        /* Multiple conditional jumps in loop */
        if (cond4) {
            __asm__ volatile ("" : : : "memory");
            goto target_label4;
        }
        
        if (i % 2 == 0) {
            sum += temp;
            continue;
        }
        
        /* More code to separate blocks */
        ext_func2(sum);
        
        if (i == iterations - 1) {
            __asm__ volatile ("" : : : "memory");
            goto target_label4;
        }
        
        continue;
        
    target_label4:
        /* Safe operation at target */
        sum = sum + res + 5;
        
        /* Break to prevent infinite loop in execution */
        if (i > 0) break;
    }
    
    result += sum;
}

/* Function that mimics delay slot candidate patterns */
__attribute__((optimize("O2"), noinline))
void test_pattern5(void) {
    volatile int alpha = 42, beta = 84, gamma = 0;
    volatile int delta = 7, epsilon = 0;
    
    /* Complex parent instruction simulation */
    int res5 = ext_func1(alpha) + ext_func2(beta);
    
    /* Multiple basic blocks with labels */
    if (cond1 && !cond2) {
        __asm__ volatile ("" : : : "memory");
        goto target_label5a;
    }
    
    epsilon = delta * 3;
    ext_func3(epsilon);
    
    if (cond3 || cond4) {
        __asm__ volatile ("" : : : "memory");
        goto target_label5b;
    }
    
    return;
    
target_label5a:
    /* First target - simple addition */
    gamma = beta + alpha;
    result += gamma + res5;
    return;
    
target_label5b:
    /* Second target - simple subtraction */
    gamma = beta - alpha;
    result += gamma + res5;
}

/* External function declarations (simulated) */
int ext_func1(int x) {
    /* Simulate side effects without trapping */
    static int counter = 0;
    counter++;
    return x + counter;
}

int ext_func2(int x) {
    /* Another non-trapping function */
    return x * 2;
}

int ext_func3(int x) {
    /* Function with memory clobber */
    __asm__ volatile ("" : : : "memory");
    return x / 2;
}

int main(void) {
    printf("Testing reorg delay slot patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = rand() % 2;
    cond2 = rand() % 2;
    cond3 = rand() % 2;
    cond4 = rand() % 2;
    
    /* Execute test patterns */
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4(3);
    test_pattern5();
    
    /* Print result to ensure execution */
    printf("Accumulated result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
