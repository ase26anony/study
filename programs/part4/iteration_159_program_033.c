#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier */
static int __attribute__((noinline)) get_value(int x) {
    return x ^ global_seed;
}

/* Test function 1: Basic pattern for MIPS */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    /* Create temporaries that are independent of the condition */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    
    /* Dynamic condition using function arguments */
    if (get_value(a) > get_value(b)) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 * 3;
    return temp3;
    
target_label_1:
    /* Safe, non-jump instruction using independent temporaries */
    temp3 = temp1 + temp2;  /* Simple arithmetic, no traps */
    return temp3;
}

/* Test function 2: Different operation pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    /* Independent local variables */
    int local_a = x & 0xFF;
    int local_b = y | 0x55;
    int local_c = local_a - local_b;
    
    /* Volatile read to prevent constant folding */
    volatile int v = global_seed;
    if ((x ^ v) < (y ^ v)) {
        goto target_label_2;
    }
    
    local_c = local_c >> 2;
    return local_c;
    
target_label_2:
    /* Safe bitwise operation */
    local_c = local_a ^ local_b;  /* No memory access, no division */
    return local_c;
}

/* Test function 3: More complex control flow around the target */
static int test_case_3(int n) {
    int i, sum = 0;
    int tmp1 = n + 100;
    int tmp2 = n * 2;
    
    /* Loop to create more scheduling context */
    for (i = 0; i < 3; i++) {
        sum += i;
    }
    
    /* Multiple conditions to create branching */
    if (n % 2 == 0) {
        if (get_value(n) > 20) {
            goto target_label_3;
        }
        sum += 10;
    }
    
    sum += tmp1;
    return sum;
    
target_label_3:
    /* Safe arithmetic with constants only */
    tmp2 = tmp1 * 3;  /* Uses only local temporaries */
    return tmp2 + sum;
}

/* Test function 4: Nested conditions */
static int test_case_4(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b + c;
    int t3 = c + a;
    
    /* Chain of conditions */
    if (a > 0) {
        if (b > 0) {
            if (c > 0) {
                goto target_label_4;
            }
        }
    }
    
    t3 = t1 * t2;
    return t3;
    
target_label_4:
    /* Simple assignment with arithmetic */
    t3 = t1 - t2;  /* All variables defined before the jump */
    return t3;
}

/* Test function 5: Using only volatile variables for condition */
static int test_case_5(void) {
    volatile int v1 = global_seed;
    volatile int v2 = global_seed + 1;
    
    int r1 = v1 * 2;
    int r2 = v2 / 2;  /* Division is safe here - v2 is never 0 */
    int r3 = r1 | r2;
    
    if (v1 != v2) {
        goto target_label_5;
    }
    
    r3 = r1 & r2;
    return r3;
    
target_label_5:
    /* Shift operation - always safe */
    r3 = r1 << 2;
    return r3;
}

/* Main driver that calls all test cases */
int main(int argc, char **argv) {
    int result = 0;
    int i;
    
    /* Use command line arguments or defaults for variability */
    int a = argc > 1 ? atoi(argv[1]) : 10;
    int b = argc > 2 ? atoi(argv[2]) : 20;
    int c = argc > 3 ? atoi(argv[3]) : 30;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 3; i++) {
        result ^= test_case_1(a + i, b - i);
        result ^= test_case_2(b + i, c - i);
        result ^= test_case_3(a * i + b);
        result ^= test_case_4(a + i, b + i, c + i);
        result ^= test_case_5();
        
        /* Modify inputs slightly each iteration */
        a += 1;
        b += 2;
        c += 3;
    }
    
    global_accumulator = result;
    printf("Final checksum: %d\n", result);
    printf("Accumulator: %d\n", global_accumulator);
    
    return result != 0 ? 0 : 1;
}
