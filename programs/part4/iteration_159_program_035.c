#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier to prevent unwanted optimizations */
__attribute__((noinline)) int get_value(int x) {
    return x ^ global_seed;
}

/* Barrier to create CFG complexity */
__attribute__((noinline)) void side_effect(int x) {
    global_accumulator += x;
}

/* MIPS-specific variant */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_pattern1(int a, int b) {
    /* Create temporaries independent of jump condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    
    /* Other basic blocks to create scheduling context */
    if (a > 100) {
        side_effect(a);
    }
    
    /* Simple conditional jump to label */
    if (a != b) {
        /* Use input-dependent condition to prevent optimization */
        if (get_value(a) > get_value(b)) {
            goto target_label1;
        }
    }
    
    /* Fall-through path */
    temp3 = temp3 * 2;
    return temp3;
    
target_label1:
    /* Safe, non-jump instruction after label */
    temp3 = temp3 | 0x1F;  /* Simple bitwise operation */
    
    /* Use result to prevent elimination */
    return temp3 + 1;
}

/* SPARC-specific variant */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_pattern2(int x, int y) {
    /* Independent temporaries */
    int local_a = x & 0xFF;
    int local_b = y << 2;
    int local_c = local_a + local_b;
    
    /* Create some control flow before the jump */
    for (int i = 0; i < 3; i++) {
        local_c += i;
    }
    
    /* Jump with non-trivial condition */
    if (x != 0 && y != 0) {
        if ((x % 17) < (y % 13)) {
            goto target_label2;
        }
    }
    
    /* Alternative path */
    local_c = local_c - 5;
    return local_c;
    
target_label2:
    /* Safe arithmetic operation after label */
    local_c = local_c * 3;
    
    /* Ensure value is used */
    side_effect(local_c);
    return local_c;
}

/* Generic variant for delay slot architectures */
int test_pattern3(int p, int q) {
    /* Multiple independent variables */
    int var1 = p + q;
    int var2 = p - q;
    int var3 = var1 * var2;
    int var4 = var1 ^ var2;
    
    /* Complex enough to avoid early optimization */
    if (p > 0 && q > 0) {
        /* Nested condition for jump */
        if ((p & 3) == (q & 3)) {
            /* Additional computation to prevent trivial jump */
            int diff = abs(p - q);
            if (diff < 10) {
                goto target_label3;
            }
        }
    }
    
    /* Fall-through with different computation */
    var3 = var3 + var4;
    return var3;
    
target_label3:
    /* Simple, safe instruction after label */
    var4 = var4 & 0x7FFF;  /* Mask operation - cannot trap */
    
    /* Use the result */
    return var4 * 2;
}

/* Variant with volatile to prevent optimization */
int test_pattern4(volatile int* input) {
    int v1 = *input;
    int v2 = v1 + 100;
    int v3 = v2 * 2;
    int v4 = v3 ^ 0xABCD;
    
    /* Jump condition based on volatile read */
    if (v1 > 50) {
        if ((v1 & 1) == 0) {
            goto target_label4;
        }
    }
    
    v4 = v4 >> 2;
    return v4;
    
target_label4:
    /* Safe logical operation */
    v3 = v3 | 0x3F;
    
    return v3 + v4;
}

/* Test with multiple labels and jumps */
int test_pattern5(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b + c;
    int t3 = c + a;
    
    /* Multiple basic blocks */
    if (a > b) {
        t1 = t1 * 2;
    } else {
        t1 = t1 / 2;
    }
    
    /* The target jump pattern */
    if (c != 0) {
        if ((a * b) < 1000) {
            goto main_target;
        }
    }
    
    t2 = t2 - t3;
    return t2;
    
main_target:
    /* Instruction to potentially fill delay slot */
    t3 = t3 & 0xFF;  /* Safe bitwise AND */
    
    /* Continue with more code */
    t3 = t3 + t1;
    return t3;
}

int main() {
    int result = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 0; i < 10; i++) {
        result ^= test_pattern1(i, i * 2);
        result ^= test_pattern2(i + 1, i * 3);
        result ^= test_pattern3(i * 5, i * 7);
        
        volatile int vi = i * 11;
        result ^= test_pattern4(&vi);
        result ^= test_pattern5(i, i + 1, i + 2);
    }
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
