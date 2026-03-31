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
int test_mips_delay_slot(int arg1, int arg2) {
    /* Create independent temporary variables */
    int temp_a = arg1 * 3;
    int temp_b = arg2 + 7;
    int temp_c = 0;
    int temp_d = arg1 ^ arg2;
    
    /* Create a non-trivial condition using barrier */
    if (barrier(arg1) > barrier(arg2)) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_mips;
    }
    
    /* Other basic blocks to create CFG complexity */
    temp_a = temp_b * 2;
    if (temp_a < 100) {
        temp_c = temp_d | 0xFF;
    }
    
    /* This should never be reached if the goto is taken */
    return temp_a + temp_c;

target_label_mips:
    /* Safe, non-jump instruction using independent temporaries */
    temp_c = temp_b & 0x0F;  /* Simple bitwise operation */
    
    /* Use the result to prevent elimination */
    g_accumulator += temp_c;
    return temp_c;
}

/* SPARC-specific variant */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_sparc_delay_slot(int x, int y, int z) {
    /* Multiple independent variables */
    int local1 = x + y;
    int local2 = y * z;
    int local3 = x ^ z;
    int local4 = 0;
    
    /* Volatile read to prevent constant folding */
    int cond = g_volatile;
    
    /* Two-stage condition to avoid trivial optimization */
    if (x > 0 && cond != 0) {
        if (local1 < local2) {
            /* Simple jump to label */
            goto sparc_target;
        }
    }
    
    /* Alternative path with computations */
    for (int i = 0; i < 3; i++) {
        local3 += i;
    }
    return local3;

sparc_target:
    /* Safe arithmetic on independent variables */
    local4 = local1 + local3;  /* Simple addition */
    
    g_accumulator += local4;
    return local4;
}

/* Generic variant for any delay slot architecture */
int test_generic_delay(int a, int b, int c) {
    /* Create a web of independent variables */
    int t1 = a + b;
    int t2 = b - c;
    int t3 = a * c;
    int t4 = 0;
    int t5 = b ^ c;
    
    /* Complex enough condition to not be optimized away */
    int condition = (a & 1) | (b & 2) | (c & 4);
    
    if (condition != 0 && t1 > t2) {
        /* Multiple basic blocks before the jump */
        t3 = barrier(t3);
        if (t3 > 0) {
            /* The target simple jump */
            goto generic_label;
        }
    }
    
    /* Different computation path */
    t4 = t5 << 2;
    return t4;

generic_label:
    /* Safe logical operation - no traps, no memory access */
    t4 = t2 | t5;  /* Only uses variables defined before jump */
    
    g_accumulator += t4;
    return t4;
}

/* Test with multiple jumps in same function */
int test_multiple_jumps(int val) {
    int x = val * 2;
    int y = val + 5;
    int z = val ^ 0x55;
    int result = 0;
    
    /* First potential jump */
    if ((val & 1) && x > 10) {
        goto first_target;
    }
    
    /* Intermediate computation */
    y = barrier(y);
    
    /* Second potential jump */
    if ((val & 2) && y < 20) {
        goto second_target;
    }
    
    z = x + y;
    return z;

first_target:
    /* Safe instruction after first label */
    result = y & 0x0F;
    g_accumulator += result;
    return result;

second_target:
    /* Safe instruction after second label */
    result = z + 1;  /* Simple increment */
    g_accumulator += result;
    return result;
}

/* Test with nested control flow */
int test_nested_control(int p, int q) {
    int a = p * q;
    int b = p + q;
    int c = p - q;
    int d = 0;
    
    /* Outer loop */
    for (int i = 0; i < 2; i++) {
        a += i;
        
        /* Inner condition */
        if (i == 1 && a > b) {
            /* Jump from inside loop */
            goto nested_target;
        }
        
        b = barrier(b);
    }
    
    c = a * b;
    return c;

nested_target:
    /* Safe operation after label */
    d = b ^ a;  /* XOR of two integers */
    
    g_accumulator += d;
    return d;
}

int main() {
    int results[5];
    int checksum = 0;
    
    /* Initialize volatile to prevent constant propagation */
    g_volatile = time(NULL) & 0xFF;
    
    /* Call all test functions with different arguments */
    results[0] = test_mips_delay_slot(5, 10);
    results[1] = test_sparc_delay_slot(3, 7, 11);
    results[2] = test_generic_delay(2, 6, 9);
    results[3] = test_multiple_jumps(8);
    results[4] = test_nested_control(4, 12);
    
    /* Compute checksum */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        checksum = (checksum << 3) | (checksum >> 29);  /* Simple rotation */
    }
    
    checksum ^= g_accumulator;
    
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", results[i]);
    }
    printf("\nAccumulator: %d\n", g_accumulator);
    printf("Checksum: 0x%08X\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
