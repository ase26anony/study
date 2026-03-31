/* Test program for triggering delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

static void __attribute__((noinline)) update_accumulator(int x) {
    global_accumulator ^= x;
}

/* Test function 1: Simple conditional jump with safe arithmetic after label */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    /* Local temporaries - independent of jump condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create a non-trivial condition */
    if (a > b && (a % 2) == 0) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = use_value(temp1);
    temp2 = temp2 & 0xFF;
    
    return temp1 + temp2;

target_label_1:
    /* Safe, non-jump instruction that doesn't trap */
    /* Uses independent temporaries not used in the condition */
    temp3 = temp1 + temp2;  /* Simple arithmetic */
    temp4 = temp3 ^ 0x55;   /* Simple logical operation */
    
    /* Use the result to prevent dead code elimination */
    return temp4;
}

/* Test function 2: Another pattern with different operations */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    /* More independent temporaries */
    int local_a = x * 2;
    int local_b = y - 5;
    int local_c = 0;
    int local_d = 0;
    
    /* Different condition */
    volatile int vol = x;  /* Prevent constant propagation */
    if (vol > 10 && y < 20) {
        goto target_label_2;
    }
    
    /* Alternative path */
    local_a = local_a | 0x01;
    local_b = local_b << 1;
    
    return local_a - local_b;

target_label_2:
    /* Safe instruction using only local temporaries */
    local_c = local_a + local_b;
    local_d = local_c * 3;  /* Multiplication is safe with integers */
    
    return local_d;
}

/* Test function 3: Nested control flow with target jump */
static int test_case_3(int n) {
    int t1 = n + 100;
    int t2 = n * 2;
    int t3 = 0;
    int t4 = 0;
    
    /* More complex control flow around the target */
    for (int i = 0; i < 3; i++) {
        t1 = t1 ^ i;
        
        /* The target jump pattern */
        if (n > 0 && (t1 & 1)) {
            goto target_label_3;
        }
        
        t2 = t2 + i;
    }
    
    return t1 + t2;

target_label_3:
    /* Safe operation after label */
    t3 = t1 - t2;
    t4 = t3 & 0x7F;  /* Mask operation - always safe */
    
    return t4;
}

/* Test function 4: Multiple jumps to same label */
static int test_case_4(int a, int b, int c) {
    int x = a + b;
    int y = b + c;
    int z = 0;
    
    /* Multiple conditions leading to same label */
    if (a > b) {
        if (c != 0) {
            goto common_label;
        }
    }
    
    if (b < c) {
        goto common_label;
    }
    
    /* Fall through */
    x = x * 2;
    y = y / 2;  /* Division but divisor is constant 2 - safe */
    
    return x + y;

common_label:
    /* Instruction after label - uses independent variable z */
    z = x ^ y;  /* XOR operation - always safe */
    
    return z;
}

/* Test function 5: Minimal pattern focusing on the exact condition */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_5(int val) {
    /* Minimal set of variables to avoid resource conflicts */
    int r1 = val + 1;
    int r2 = val * 2;
    int r3 = 0;
    
    /* Simple condition that should produce simplejump_p */
    volatile int barrier = val;
    if (barrier > 50) {
        goto minimal_label;
    }
    
    r1 = r1 | 0x01;
    return r1 + r2;

minimal_label:
    /* Single, simple, safe instruction */
    r3 = r1 - r2;  /* Only uses r1 and r2 defined before jump */
    
    return r3;
}

int main(void) {
    int result = 0;
    int checksum = 0;
    
    /* Seed for pseudo-random but reproducible behavior */
    srand(42);
    
    /* Execute all test cases with different inputs */
    result = test_case_1(rand() % 100, rand() % 100);
    checksum ^= result;
    update_accumulator(result);
    
    result = test_case_2(rand() % 100, rand() % 100);
    checksum ^= result;
    update_accumulator(result);
    
    result = test_case_3(rand() % 100);
    checksum ^= result;
    update_accumulator(result);
    
    result = test_case_4(rand() % 100, rand() % 100, rand() % 100);
    checksum ^= result;
    update_accumulator(result);
    
    result = test_case_5(rand() % 100);
    checksum ^= result;
    update_accumulator(result);
    
    /* Print results to ensure code executes */
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return checksum == 0 ? 0 : 1;
}
