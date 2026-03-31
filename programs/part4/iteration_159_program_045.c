#include <stdio.h>
#include <stdlib.h>

volatile int g_volatile = 0;
static int g_accumulator = 0;

__attribute__((noinline)) int barrier(int x) {
    return x + (g_volatile & 1);
}

/* MIPS-specific variant */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_mips_pattern(int arg1, int arg2) {
    /* Create independent temporaries to avoid resource conflicts */
    int temp_a = arg1 * 3;
    int temp_b = arg2 + 7;
    int temp_c = arg1 ^ arg2;
    int temp_d = 0;
    
    /* Create other basic blocks first */
    if (arg1 > 100) {
        temp_a = barrier(temp_a);
    }
    
    /* Loop to create more CFG complexity */
    for (int i = 0; i < 3; i++) {
        temp_b += i;
    }
    
    /* The critical pattern: simple conditional jump to label */
    if (arg1 != arg2) {
        /* This should compile to a simple jump to target_label */
        goto target_label;
    }
    
    /* Fall-through path */
    temp_d = temp_a - temp_b;
    return temp_d;
    
target_label:
    /* Safe, non-jump instruction immediately after label */
    /* Uses independent temporaries not involved in jump condition */
    temp_c = temp_b & 0xFF;  /* Simple logical operation */
    
    /* Use result to prevent elimination */
    return temp_c + 1;
}

/* SPARC-specific variant */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_sparc_pattern(int x, int y, int z) {
    /* Multiple independent variable sets */
    int set1_a = x * 2;
    int set1_b = y + 5;
    int set2_a = z - 3;
    int set2_b = x ^ y ^ z;
    int result = 0;
    
    /* Pre-jump computation using first set */
    if (x > 0) {
        set1_a = barrier(set1_a);
    }
    
    /* Another basic block */
    if (y < 0) {
        set1_b = set1_b * 2;
    }
    
    /* The target jump pattern */
    if ((x + y) > z) {
        /* Simple jump to label */
        goto compute_result;
    }
    
    /* Alternative path */
    result = set1_a + set1_b;
    return result;
    
compute_result:
    /* Safe instruction after label using second variable set */
    /* No trapping operations, no memory access */
    set2_a = set2_b | 0x7F;  /* Safe bitwise operation */
    
    /* Use in computation */
    result = set2_a * 3;
    
    /* Additional basic block to prevent tail merging */
    if (result > 100) {
        result = result / 2;  /* Division by constant is safe */
    }
    
    return result;
}

/* Generic architecture pattern */
int test_generic_pattern(int a, int b, int c) {
    /* Three independent temporary sets */
    int t1 = a + b;
    int t2 = b * c;
    int t3 = a ^ c;
    int t4 = 0;
    int t5 = 0;
    
    /* Create control flow before the target pattern */
    switch (a & 0x3) {
        case 0: t1 = t1 + 1; break;
        case 1: t1 = t1 - 1; break;
        case 2: t1 = barrier(t1); break;
        default: t1 = t1 * 2; break;
    }
    
    /* The critical simple jump */
    if ((b & 1) && (c > 0)) {
        goto process_data;
    }
    
    /* Fall-through uses t1 and t2 */
    t4 = t1 - t2;
    return t4;
    
process_data:
    /* Safe instruction after label using t3 and t5 */
    /* t5 is uninitialized here but will be set - safe for delay slot */
    t5 = t3 << 2;  /* Simple shift operation */
    
    /* Use result */
    return t5 + a;
}

/* Another variant with different operation types */
int test_variant_4(int p1, int p2) {
    int v1 = p1 + p2;
    int v2 = p1 * p2;
    int v3 = p1 - p2;
    int v4 = 0;
    
    /* Some pre-processing */
    for (int i = 0; i < 2; i++) {
        v1 = v1 + i;
    }
    
    /* Jump based on input-dependent condition */
    if (p1 != 0 && p2 != 0) {
        goto do_calc;
    }
    
    v4 = v2 / 2;  /* Division by constant in fall-through */
    return v4;
    
do_calc:
    /* Multiple safe instructions (compiler may schedule one into delay slot) */
    v3 = v3 & 0x3F;    /* Bitwise AND - very safe */
    
    /* Use v3 in computation */
    return v3 * 3 + 1;
}

/* Test with more complex surrounding code */
int test_complex_context(int base) {
    int array[4] = {base, base+1, base+2, base+3};
    int sum = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    /* Compute something */
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    
    tmp1 = sum;
    tmp2 = base * 2;
    tmp3 = base ^ 0x55;
    
    /* Multiple conditional jumps */
    if (base > 50) {
        tmp1 = barrier(tmp1);
    }
    
    /* The target pattern */
    if ((sum & 1) == 0) {
        goto finalize;
    }
    
    /* Alternative path */
    tmp2 = tmp2 + tmp1;
    return tmp2;
    
finalize:
    /* Safe operation after label */
    tmp3 = tmp3 + 5;  /* Simple addition */
    
    return tmp3;
}

int main() {
    int result = 0;
    
    /* Call test functions with different inputs */
    result += test_mips_pattern(10, 20);
    result += test_mips_pattern(30, 30);  /* Different branch taken */
    
    result += test_sparc_pattern(5, -3, 8);
    result += test_sparc_pattern(1, 2, 3);
    
    result += test_generic_pattern(7, 8, 9);
    result += test_generic_pattern(0, 5, 10);
    
    result += test_variant_4(12, 6);
    result += test_variant_4(0, 15);
    
    result += test_complex_context(25);
    result += test_complex_context(60);
    
    /* Store to global to prevent optimization */
    g_accumulator = result;
    
    printf("Result checksum: %d\n", result);
    printf("Accumulator: %d\n", g_accumulator);
    
    return 0;
}
