#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

__attribute__((noinline)) int barrier(int x) {
    return x ^ 0x55AA55AA;
}

/* MIPS-specific variant */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of jump condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    
    /* Simple condition using arguments - not trivially true/false */
    if (a > b) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = barrier(temp3);
    
    /* This is the instruction after the label - safe and non-jump */
    target_label_1:
    temp3 = (temp3 & 0xFFFF) | ((temp1 + temp2) << 16);
    
    /* Use result to prevent elimination */
    return temp3;
}

/* SPARC-specific variant */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int local_a = x * 2;
    int local_b = y - 5;
    int local_c = local_a | local_b;
    
    /* Volatile read to prevent optimization */
    volatile int v = global_seed;
    
    /* Another simple condition */
    if ((x ^ y) > v) {
        goto target_label_2;
    }
    
    /* Different path with barrier */
    local_c = barrier(local_c);
    
    target_label_2:
    /* Safe arithmetic after label - no trapping */
    local_c = (local_c * 3) + 1;
    
    return local_c;
}

/* Generic variant for delay slot architectures */
int test_case_3(int p, int q) {
    /* More temporaries to work with */
    int t1 = p + q;
    int t2 = p - q;
    int t3 = t1 * t2;
    int t4 = t1 ^ t2;
    
    /* Complex enough condition to not be optimized away */
    if ((p & 0xF) < (q & 0xF)) {
        goto target_label_3;
    }
    
    /* Alternative path */
    t3 = barrier(t3);
    
    target_label_3:
    /* Multiple safe operations that could be moved */
    t4 = (t4 << 2) | (t3 & 3);
    
    return t4;
}

/* Test with nested control flow */
int test_case_4(int n) {
    int acc = n;
    int tmp1 = n * 2;
    int tmp2 = n + 1;
    
    for (int i = 0; i < 3; i++) {
        /* Loop creates more scheduling opportunities */
        tmp1 += i;
        
        if (tmp1 > tmp2) {
            /* Jump from inside loop */
            goto target_label_4;
        }
        
        tmp2 = barrier(tmp2);
    }
    
    target_label_4:
    /* Safe bit manipulation */
    tmp1 = (tmp1 & 0xFF) ^ (tmp2 & 0xFF00);
    
    return tmp1 + acc;
}

/* Test with multiple jumps to same label */
int test_case_5(int a, int b, int c) {
    int x = a + b;
    int y = b + c;
    int z = c + a;
    
    if (a > 0) {
        if (b > 0) {
            goto common_label;
        }
        x = barrier(x);
    }
    
    if (c > 0) {
        goto common_label;
    }
    
    y = barrier(y);
    
    common_label:
    /* Instruction to potentially fill delay slot */
    z = (x & y) | z;
    
    return z;
}

int main() {
    int result = 0;
    
    /* Call test cases with varying inputs */
    for (int i = 0; i < 10; i++) {
        result ^= test_case_1(i, i * 2);
        result ^= test_case_2(i, i + 3);
        result ^= test_case_3(i, i * 3);
        result ^= test_case_4(i);
        result ^= test_case_5(i, i + 1, i + 2);
        
        /* Modify volatile to change jump decisions */
        global_seed++;
    }
    
    global_accumulator = result;
    printf("Result checksum: 0x%08X\n", result);
    
    return 0;
}
