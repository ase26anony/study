/* Test program for triggering delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

static void __attribute__((noinline)) barrier(void) {
    __asm__ volatile ("" : : : "memory");
}

/* Test function 1: Simple conditional jump with safe arithmetic after label */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create simple condition to prevent optimization */
    if (a > b) {
        barrier();
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = use_value(temp1);
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 + temp2;  /* Uses only local temps defined before jump */
    return temp3;
}

/* Test function 2: Another pattern with different variable usage */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_2(int x, int y) {
    volatile int v = x;  /* Prevent constant propagation */
    int local_a = v + 5;
    int local_b = y * 2;
    int local_c = 0;
    int local_d = 0;
    
    /* Different condition pattern */
    if ((x & 1) == 0) {
        barrier();
        goto target_label_2;
    }
    
    local_a = use_value(local_a);
    local_b = local_a - 3;
    
target_label_2:
    /* Safe bitwise operation after label */
    local_c = local_a ^ local_b;  /* Independent of jump condition */
    return local_c;
}

/* Test function 3: More complex control flow around the target jump */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_3(int n) {
    int i, j, k, result;
    
    /* Initialize temporaries */
    i = n * 2;
    j = n + 10;
    k = 0;
    
    /* Loop to create more scheduling context */
    for (int idx = 0; idx < 3; idx++) {
        barrier();
    }
    
    /* The target simple jump */
    if (n != 0) {
        barrier();
        goto target_label_3;
    }
    
    /* Alternative path */
    i = use_value(i);
    
target_label_3:
    /* Safe arithmetic with constants */
    k = i * 3 + j;  /* Uses only pre-defined locals */
    
    /* Use result to prevent elimination */
    result = k - n;
    return result;
}

/* Test function 4: Multiple basic blocks and jumps */
static int test_case_4(int p, int q) {
    int t1 = p + q;
    int t2 = p - q;
    int t3 = 0;
    int t4 = 0;
    
    /* First conditional */
    if (p > 10) {
        t1 = use_value(t1);
        goto middle_block;
    }
    
    t2 = use_value(t2);
    
middle_block:
    /* Another conditional leading to target label */
    if (q < 20) {
        barrier();
        goto final_label;
    }
    
    t3 = t1 * t2;
    
final_label:
    /* Safe instruction after final label */
    t4 = t1 & 0xFF;  /* Simple mask operation */
    return t4;
}

/* Test function 5: Minimal pattern focusing on the exact condition */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_5(int val) {
    volatile int cond = val;  /* Force runtime evaluation */
    int a = cond + 100;
    int b = 50;
    int c = 0;
    
    /* Very simple jump to label */
    if (cond > 0) {
        /* Empty block with just goto */
        goto exact_target_label;
    }
    
    a = a - 10;
    
exact_target_label:
    /* Single, simple, safe instruction */
    c = a + b;  /* No traps, no jumps, no complex patterns */
    return c;
}

/* Main driver that exercises all test cases */
int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int results[5];
    int checksum = 0;
    
    /* Initialize with some values */
    int vals[5] = {seed, seed + 1, seed + 2, seed + 3, seed + 4};
    
    /* Run all test cases */
    results[0] = test_case_1(vals[0], vals[1]);
    results[1] = test_case_2(vals[1], vals[2]);
    results[2] = test_case_3(vals[2]);
    results[3] = test_case_4(vals[3], vals[4]);
    results[4] = test_case_5(vals[0]);
    
    /* Compute checksum to ensure all code executed */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        global_accumulator += results[i];
    }
    
    printf("Test results: %d, %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    printf("Checksum: 0x%08x\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return (checksum == 0) ? 1 : 0;
}
