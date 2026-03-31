/* Test program for delay slot filling in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier */
__attribute__((noinline)) int get_value(int x) {
    return x ^ 0x55AA;
}

/* MIPS-specific test function */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_mips_pattern(int a, int b) {
    /* Create temporaries independent of condition variables */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    int result = 0;
    
    /* Dynamic condition to prevent optimization */
    if (a > b) {
        /* This should generate a simple jump to label */
        if ((a - b) > get_value(global_seed) % 10) {
            goto target_label;
        }
        result = temp1;
    } else {
        result = temp2;
    }
    
    /* Some other code to create CFG complexity */
    for (int i = 0; i < 3; i++) {
        temp3 += i;
    }
    
    return result + temp3;

target_label:
    /* Safe, non-jump instruction after label */
    /* Uses independent temporaries not used in condition */
    temp3 = (temp1 & 0xFF) | (temp2 & 0xFF00);
    
    /* Use result to prevent elimination */
    result = temp3 + 1;
    return result;
}

/* SPARC-specific test function */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_sparc_pattern(int x, int y) {
    /* Independent temporaries */
    int local_a = x * 2;
    int local_b = y - 5;
    int local_c = local_a ^ local_b;
    int local_d = 0;
    
    /* Complex enough condition */
    volatile int barrier = global_seed;
    if (x != 0 && (x * y) < 100) {
        if ((x ^ y) > (barrier & 0xF)) {
            goto sparc_target;
        }
        local_d = local_a;
    }
    
    /* Additional basic blocks */
    switch (x % 3) {
        case 0: local_c += 1; break;
        case 1: local_c += 2; break;
        default: local_c += 3;
    }
    
    return local_c + local_d;

sparc_target:
    /* Safe arithmetic after label */
    local_c = (local_a + local_b) * 2;
    
    /* Simple operation, no traps */
    local_d = local_c >> 1;
    return local_d;
}

/* Generic test function for architectures with delay slots */
int test_generic_pattern(int p, int q) {
    /* Multiple independent variables */
    int var1 = p + 1;
    int var2 = q * 2;
    int var3 = var1 | var2;
    int var4 = 0;
    
    /* Prevent trivial optimization */
    int cond = get_value(p) - get_value(q);
    
    if (p > 0 && q > 0) {
        if (cond > 0) {
            goto generic_target;
        }
        var4 = var1;
    }
    
    /* Loop to create scheduling opportunities */
    for (int i = 0; i < 2; i++) {
        var3 += i * 2;
    }
    
    return var3 + var4;

generic_target:
    /* Safe instruction using only local temporaries */
    var3 = var1 * var2 + 5;
    
    /* Ensure value is used */
    var4 = var3 % 100;
    return var4;
}

/* Another variant with different temporary usage */
int test_variant2(int m, int n) {
    int t1 = m & 0xFF;
    int t2 = n | 0x55;
    int t3 = t1 + t2;
    int res = 0;
    
    /* More complex condition */
    volatile int v = global_seed;
    if ((m * n) % 7 == (v % 5)) {
        if (m < n) {
            goto variant_target;
        }
        res = t1;
    }
    
    /* Additional operations */
    t3 = (t3 << 1) ^ 0xAA;
    return res + t3;

variant_target:
    /* Bitwise operation - guaranteed safe */
    t3 = (t1 ^ t2) & 0x7F;
    
    res = t3 * 2;
    return res;
}

int main() {
    int checksum = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 1; i <= 5; i++) {
        for (int j = 1; j <= 5; j++) {
            checksum ^= test_mips_pattern(i, j);
            checksum ^= test_sparc_pattern(i, j);
            checksum ^= test_generic_pattern(i, j);
            checksum ^= test_variant2(i, j);
            
            /* Update volatile to change conditions */
            global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    printf("Checksum: %d\n", checksum);
    global_accumulator = checksum;
    
    /* Return non-zero if any test failed (simplified) */
    return (checksum == 0) ? 1 : 0;
}
