#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA;
}

__attribute__((noinline)) int use_result(int x) {
    return x + 1;
}

/* Test function 1: Basic pattern for MIPS */
__attribute__((target("arch=mips32")))
int test_mips_basic(int arg1, int arg2) {
    /* Local temporaries independent of jump condition */
    int temp_a = arg1 * 3;
    int temp_b = arg2 + 7;
    int temp_c = 0;
    int temp_d = 0;
    
    /* Create non-trivial condition using input-dependent values */
    if (arg1 > arg2 && (arg1 & 0xF) != 0) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_1;
    }
    
    /* Alternative path */
    temp_c = temp_a - temp_b;
    return use_result(temp_c);
    
target_label_1:
    /* Safe, non-jump instruction using independent temporaries */
    temp_d = temp_b * 2;  /* Simple arithmetic, no traps */
    
    /* Use result to prevent elimination */
    return use_result(temp_d);
}

/* Test function 2: Multiple temporaries for SPARC */
__attribute__((target("arch=sparc")))
int test_sparc_variant(int base, int mask) {
    volatile int vol = base;  /* Prevent constant propagation */
    int x = vol & 0xFF;
    int y = mask | 0x11;
    int z = 0;
    int w = 0;
    
    /* Different condition pattern */
    if ((x ^ y) > 100 && x < 200) {
        goto sparc_target;
    }
    
    /* Other basic blocks */
    for (int i = 0; i < 2; i++) {
        z += i * x;
    }
    return z;

sparc_target:
    /* Safe bitwise operation - no memory access or division */
    w = y << 2;
    
    /* Use in computation */
    return w + (x & 0xF);
}

/* Test function 3: Generic with more complex CFG */
int test_generic_complex(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b * c;
    int t3 = 0;
    int t4 = 0;
    int t5 = 0;
    
    /* Multiple basic blocks before target */
    if (a > 0) {
        t3 = t1 - t2;
        if (b < c) {
            t4 = t3 >> 1;
        } else {
            t4 = t3 << 1;
        }
    }
    
    /* Dynamic condition */
    if ((t1 ^ t4) != 0 && (c % 3) != 0) {  /* modulo with non-zero divisor is safe */
        goto generic_label;
    }
    
    /* Alternative path with loop */
    for (int i = 0; i < 3; i++) {
        t5 += i * t4;
    }
    return t5;

generic_label:
    /* Safe logical operation using independent temporaries */
    t5 = (t2 & 0x0F0F) | (t1 & 0xF0F0);
    
    return t5 + t4;
}

/* Test function 4: Nested control flow */
int test_nested_pattern(int seed) {
    int local1 = seed * 3;
    int local2 = seed + 5;
    int local3 = 0;
    int result = 0;
    
    /* Outer condition */
    if (seed > 10) {
        /* Inner condition */
        if ((local1 & 0x7) < 4) {
            /* The target simple jump */
            goto nested_target;
        }
        local3 = local1 - local2;
    } else {
        local3 = local1 + local2;
    }
    
    result = local3 * 2;
    return result;

nested_target:
    /* Safe arithmetic with constants */
    result = local2 * 3 + 1;
    
    return result;
}

/* Test function 5: Multiple candidate labels */
int test_multi_label(int val) {
    int a = val;
    int b = a * 2;
    int c = 0;
    int d = 0;
    
    /* First potential jump */
    if ((a & 1) && (b > 10)) {
        goto label_alpha;
    }
    
    /* Second potential jump */
    if ((a & 2) && (b < 100)) {
        goto label_beta;
    }
    
    c = a + b;
    return c;

label_alpha:
    /* First safe instruction */
    d = b - 5;
    return d | 0x01;

label_beta:
    /* Second safe instruction */
    d = a ^ b;
    return d & 0xFF;
}

int main() {
    int checksum = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 1; i <= 10; i++) {
        checksum ^= test_mips_basic(i, i * 2);
        checksum ^= test_sparc_variant(i, i + 3);
        checksum ^= test_generic_complex(i, i + 1, i + 2);
        checksum ^= test_nested_pattern(i);
        checksum ^= test_multi_label(i);
        
        /* Update global to prevent optimization */
        global_acc += checksum;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
