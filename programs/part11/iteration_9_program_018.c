/* test_reorg.c - Program to trigger delay slot filling logic in GCC reorg pass */

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

/* Function with attribute to force specific optimization level */
__attribute__((optimize("O2")))
void test_pattern1(void) {
    volatile int x = 0, y = 0, z = 0;
    
    /* Create resource set for parent instruction */
    x = a + b;  /* This could be the parent instruction needing delay slot */
    
    /* Call external function to create resource barrier */
    ext_func1(x);
    
    /* Conditional jump to label */
    if (cond1) {
        goto target_label1;
    }
    
    /* Some intermediate code */
    y = c * 2;
    ext_func2(y);
    
    /* This should not be executed if cond1 is true */
    z = d / 2;
    
target_label1:
    /* Candidate for delay slot filling - simple arithmetic */
    /* Uses different variables than parent instruction */
    volatile int temp1 = c + d;
    result += temp1;
    
    /* More operations after label */
    volatile int temp2 = a * b;
    result += temp2;
}

__attribute__((optimize("O3")))
void test_pattern2(void) {
    volatile int p = 0, q = 0, r = 0;
    
    /* Different computation pattern */
    p = b * c;
    
    /* Inline assembly to create artificial resource constraints */
    __asm__ volatile ("" : : : "memory");
    
    /* Nested control flow */
    for (volatile int i = 0; i < 3; i++) {
        if (cond2) {
            goto target_label2;
        }
        
        switch (i) {
            case 0:
                q = a + i;
                break;
            case 1:
                q = b + i;
                break;
            default:
                q = c + i;
        }
        
        if (i == 1 && cond3) {
            goto target_label2;
        }
    }
    
    r = d * 2;
    
target_label2:
    /* Another delay slot candidate - safe assignment */
    volatile int temp3 = a - b;
    result += temp3;
    
    ext_func3(temp3);
}

/* Function with computed goto */
__attribute__((optimize("O2"), noinline))
void test_pattern3(void) {
    volatile int m = 0, n = 0;
    
    /* Parent instruction computation */
    m = (a << 2) | b;
    
    /* Static label array for computed goto */
    static void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    /* External call for resource separation */
    ext_func1(m);
    
    /* Computed goto based on volatile condition */
    volatile int idx = cond4 ? 0 : 1;
    goto *labels[idx];
    
label_a:
    n = 100;
    goto end;
    
label_b:
    /* Delay slot candidate position */
    volatile int temp4 = c * d;
    result += temp4;
    
    /* Non-trapping operation */
    volatile int temp5 = a + 1;
    result += temp5;
    goto end;
    
label_c:
    n = 300;
    
end:
    /* Final computation */
    volatile int temp6 = n + 10;
    result += temp6;
}

/* Complex pattern with multiple jumps */
__attribute__((optimize("O2"), noinline))
void test_pattern4(void) {
    volatile int u = 0, v = 0, w = 0;
    
    /* Multiple basic blocks */
    u = ext_func1(a);
    
    if (cond1) {
        v = ext_func2(b);
        
        /* Jump to label with immediate non-jump instruction */
        if (cond3) {
            goto target_label4;
        }
        
        w = u + v;
    } else {
        v = ext_func3(c);
        
        /* Another jump opportunity */
        if (cond2) {
            goto target_label4;
        }
        
        w = u - v;
    }
    
    /* Intermediate computation */
    volatile int temp = w * 2;
    
    /* This shouldn't be reached if jumps are taken */
    result += 999;
    return;
    
target_label4:
    /* Ideal delay slot candidate - simple, non-trapping */
    volatile int temp7 = d + 5;
    result += temp7;
    
    /* Followed by another safe operation */
    volatile int temp8 = temp7 * 2;
    result += temp8;
}

/* Function with loop and label at beginning */
__attribute__((optimize("O2")))
void test_pattern5(void) {
    volatile int sum = 0;
    volatile int count = 5;
    
    /* Parent instruction */
    sum = a;
    
    /* Loop with potential jump to label */
    while (count-- > 0) {
        if (cond1 && count == 2) {
            goto process_label;
        }
        
        sum += count;
        ext_func1(sum);
    }
    
    /* Alternate path */
    sum *= 2;
    goto finish;
    
process_label:
    /* Candidate instruction - doesn't conflict with parent's resources */
    volatile int increment = b;
    result += increment;
    
    /* Continue normal flow */
    sum += 100;
    
finish:
    result += sum;
}

/* External function implementations */
int ext_func1(int x) {
    /* Simulate side effects */
    static int counter = 0;
    return x + (counter++);
}

int ext_func2(int x) {
    return x * 2;
}

int ext_func3(int x) {
    return x / 2;
}

int main(void) {
    printf("Starting reorg test patterns...\n");
    
    /* Initialize volatile conditions */
    cond1 = 1;  /* Will take first jump */
    cond2 = 0;  /* Won't take some jumps */
    cond3 = 1;  /* Will take other jumps */
    cond4 = 0;  /* For computed goto */
    
    /* Run test patterns multiple times with different conditions */
    for (int i = 0; i < 3; i++) {
        test_pattern1();
        test_pattern2();
        test_pattern3();
        test_pattern4();
        test_pattern5();
        
        /* Vary conditions */
        cond1 = !cond1;
        cond2 = (i % 2 == 0);
        a += i;  /* Change values */
    }
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
