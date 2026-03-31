#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier to prevent unwanted optimizations */
static int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

/* Barrier to create register pressure */
static void __attribute__((noinline)) dummy_work(int a, int b, int c) {
    global_accumulator += a ^ b ^ c;
}

/* Test function 1: Basic pattern with independent temporaries */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int x, int y) {
    /* Create independent temporary variables */
    int temp1 = x + 1;
    int temp2 = y * 2;
    int temp3 = temp1 ^ temp2;
    int result = 0;
    
    /* Dynamic condition to prevent optimization */
    if (x > y + global_seed % 10) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    dummy_work(temp1, temp2, temp3);
    
    /* This is the instruction that should be considered for delay slot */
    /* It uses independent variables not involved in the jump condition */
target_label_1:
    temp3 = (temp1 & 0xFF) | (temp2 & 0xFF00);
    
    result = use_value(temp3);
    return result;
}

/* Test function 2: Multiple independent operations before label */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int a, int b) {
    /* Create a chain of independent computations */
    int t1 = a + b;
    int t2 = a - b;
    int t3 = a * 3;
    int t4 = b * 7;
    int t5 = t1 ^ t2;
    
    /* More complex condition using volatile */
    volatile int cond_check = global_seed;
    if ((a * b) > (cond_check & 0xFF)) {
        goto target_label_2;
    }
    
    /* Some intervening code */
    for (int i = 0; i < 2; i++) {
        t5 += i;
    }
    
    /* Target label with safe, non-trapping instruction */
target_label_2:
    t4 = (t3 << 2) | (t5 >> 1);  /* Safe bit operations */
    
    return use_value(t4 + t1);
}

/* Test function 3: Nested control flow with simple jump */
static int test_case_3(int x) {
    int var1 = x * 2;
    int var2 = x + 100;
    int var3 = 0;
    int var4 = x ^ 0x55;
    
    /* Create some register pressure */
    int r1 = var1 + 1;
    int r2 = var2 - 1;
    int r3 = r1 * r2;
    int r4 = r3 & 0xFFFF;
    
    /* Dynamic condition based on input */
    if ((x & 1) && (global_seed % 3 == 0)) {
        /* This should be a simple jump to label */
        goto target_label_3;
    }
    
    /* Alternative path */
    r4 = r4 ^ 0xAAAA;
    
target_label_3:
    /* Safe instruction: logical operation on local temps */
    r3 = (r1 | r2) & r4;
    
    return use_value(r3);
}

/* Test function 4: Multiple labels and jumps pattern */
static int test_case_4(int val) {
    int a = val + 10;
    int b = val * 3;
    int c = a ^ b;
    int d = 0;
    
    /* First conditional jump */
    if (val > 50) {
        goto skip_block;
    }
    
    c = c + 5;
    
skip_block:
    d = a + b;
    
    /* Second conditional jump - target of interest */
    if ((c + d) < 200) {
        goto final_label;
    }
    
    /* Some intermediate computation */
    for (int i = 0; i < 3; i++) {
        d += i;
    }
    
final_label:
    /* Safe instruction after label */
    b = (c & 0xF0) | (d & 0x0F);
    
    return use_value(b);
}

/* Test function 5: Minimal pattern focusing on the exact condition */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_5(int p, int q) {
    /* Minimal set of temporaries */
    int t_a = p + q;
    int t_b = p - q;
    int t_c = 0;
    
    /* Very simple condition that should produce simple jump */
    volatile int barrier = global_seed;
    if (p != q + (barrier & 1)) {
        goto minimal_label;
    }
    
    t_c = 99;
    
minimal_label:
    /* Single, safe arithmetic operation */
    t_a = t_b * 2 + 1;
    
    return t_a;
}

/* Main driver that calls all test cases */
int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int result = 0;
    
    /* Initialize with some variation */
    global_seed = seed;
    
    /* Run all test cases with different inputs */
    result ^= test_case_1(seed, seed + 10);
    result ^= test_case_2(seed * 2, seed / 2);
    result ^= test_case_3(seed ^ 0xAA);
    result ^= test_case_4(seed % 100);
    result ^= test_case_5(seed, seed ^ 0x55);
    
    /* Add dummy work to prevent optimization of entire program */
    for (int i = 0; i < 10; i++) {
        global_accumulator += i * result;
    }
    
    printf("Result checksum: %d (Accumulator: %d)\n", 
           result, global_accumulator);
    
    return result != 0 ? 0 : 1;
}
