/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int use_result(int x) {
    global_accumulator += x;
    return x;
}

/* Test function 1: MIPS target with simple jump pattern */
__attribute__((target("arch=mips32")))
int test_mips_delay_slot_1(int a, int b) {
    volatile int cond = a > b;  /* Prevent constant propagation */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = 0;
    
    /* Other basic blocks to create scheduling context */
    if (a < 0) {
        temp1 = -temp1;
    }
    
    for (int i = 0; i < 2; i++) {
        temp2 += i;
    }
    
    /* Target jump pattern */
    if (cond) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    temp3 = temp1 - temp2;
    return use_result(temp3);
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 + temp2;  /* Uses only local temps, no traps */
    return use_result(temp3);
}

/* Test function 2: SPARC target with different variable pattern */
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot_2(int x, int y) {
    volatile int flag = (x & 0xFF) != 0;
    int local_a = x | 0x1234;
    int local_b = y & 0xABCD;
    int local_c = 0;
    int local_d = x ^ y;
    
    /* Some additional control flow */
    switch (x % 3) {
        case 0: local_a += 1; break;
        case 1: local_b -= 1; break;
        default: local_d *= 2; break;
    }
    
    /* Target jump with simple condition */
    if (flag && (local_a > 0)) {
        goto sparc_target_label;
    }
    
    local_c = local_a * local_b;
    return use_result(local_c + local_d);
    
sparc_target_label:
    /* Safe arithmetic after label - independent of jump condition */
    local_c = local_b << 2;  /* Simple shift, no trap */
    return use_result(local_c | local_d);
}

/* Test function 3: Generic architecture with multiple temporaries */
int test_generic_delay_slot_3(int val) {
    volatile int check = get_input(val) % 256;
    int t1 = val + 100;
    int t2 = val - 50;
    int t3 = 0;
    int t4 = t1 ^ t2;
    
    /* Loop to create more basic blocks */
    for (int i = 0; i < 3; i++) {
        t4 += i;
        if (i == 1) {
            t2 = t2 * 2;
        }
    }
    
    /* The critical jump pattern */
    if (check > 128) {
        goto generic_label;
    }
    
    t3 = t1 / 3;  /* Division but not in delay slot path */
    return use_result(t3 + t4);
    
generic_label:
    /* Safe bitwise operation after label */
    t3 = t4 & 0x7F;  /* Mask operation - safe, no traps */
    return use_result(t3);
}

/* Test function 4: Another variation with logical operations */
__attribute__((target("arch=mips32")))
int test_mips_delay_slot_4(int p, int q) {
    volatile int decision = (p ^ q) & 1;
    int work1 = p + q;
    int work2 = p - q;
    int work3 = 0;
    int work4 = work1 | work2;
    
    /* Additional basic block */
    if (work1 < work2) {
        work4 = work4 ^ 0xFF;
    }
    
    /* Simple jump to label */
    if (decision) {
        goto mips_label_2;
    }
    
    work3 = work4 + 1000;
    return use_result(work3);
    
mips_label_2:
    /* Safe arithmetic with constants */
    work3 = work1 * 3;  /* Multiplication with constant - safe */
    return use_result(work3);
}

/* Test function 5: Minimal pattern focusing on the exact condition */
int test_minimal_pattern(int a, int b) {
    /* Use volatile to prevent constant folding */
    volatile int cmp = a != b;
    int x = a * 2;
    int y = b + 1;
    int z = 0;
    
    /* Minimal surrounding code */
    if (a > 0) {
        x += 5;
    }
    
    /* The exact pattern we want to trigger */
    if (cmp) {
        goto minimal_target;
    }
    
    z = x - y;
    return use_result(z);
    
minimal_target:
    /* Single safe instruction after label */
    z = y << 1;  /* Simple shift - very likely eligible */
    return use_result(z);
}

/* Main driver that exercises all patterns */
int main() {
    int result = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 0; i < 10; i++) {
        result ^= test_mips_delay_slot_1(i, i * 2);
        result ^= test_sparc_delay_slot_2(i + 1, i * 3);
        result ^= test_generic_delay_slot_3(i * 5);
        result ^= test_mips_delay_slot_4(i, 10 - i);
        result ^= test_minimal_pattern(i, 20 - i);
    }
    
    /* Also test with edge cases */
    result ^= test_mips_delay_slot_1(0, 0);
    result ^= test_mips_delay_slot_1(-100, 100);
    result ^= test_sparc_delay_slot_2(0x7FFFFFFF, 0x80000000);
    
    printf("Accumulated result: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    /* Return non-zero if any test failed (simplified check) */
    return (result == 0 && global_accumulator == 0) ? 1 : 0;
}
