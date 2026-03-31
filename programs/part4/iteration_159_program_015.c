#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;
volatile int global_seed = 42;

/* Optimization barrier functions */
__attribute__((noinline)) int get_value(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int barrier(int x) {
    volatile int temp = x;
    return temp;
}

/* Test function 1: Simple arithmetic after label (MIPS target) */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Local temporaries independent of condition */
    int temp1 = a + 100;
    int temp2 = b * 2;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create condition that's not trivially predictable */
    int condition = (a & 0xF) > (b & 0x7);
    condition = barrier(condition);
    
    /* Target jump pattern */
    if (condition) {
        /* simplejump_p should be true for this */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = get_value(temp1);
    temp2 = temp2 + 1;
    
target_label_1:
    /* This instruction should be eligible for delay slot */
    temp3 = temp1 + temp2;  /* Safe arithmetic, no traps */
    
    /* Use result to prevent elimination */
    return temp3 + (a & 1);
}

/* Test function 2: Logical operations after label (SPARC target) */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int mask = 0xFF;
    int result = 0;
    int tmp_a = x | 0x1000;
    int tmp_b = y & 0x0FFF;
    
    /* Non-trivial condition using volatile read */
    volatile int vol = global_seed;
    int cond = (x ^ vol) < (y ^ vol);
    
    if (cond) {
        goto target_label_2;
    }
    
    /* Other basic blocks */
    mask = mask << 2;
    tmp_a = barrier(tmp_a);
    
target_label_2:
    /* Safe logical operation - no division or memory access */
    result = (tmp_a & mask) | (tmp_b & ~mask);
    
    return result ^ (y & 1);
}

/* Test function 3: Multiple temporaries with shift operations */
int test_case_3(int p, int q) {
    int t1 = p + q;
    int t2 = p - q;
    int t3 = 0;
    int t4 = 0;
    
    /* Complex enough condition to avoid constant folding */
    int cond = ((p * q) & 0xFF) != 0;
    cond = cond && (p != q);
    
    if (cond) {
        goto target_label_3;
    }
    
    /* Different path */
    t1 = t1 >> 1;
    t2 = t2 << 1;
    
target_label_3:
    /* Safe shift operation */
    t3 = (t1 << 2) | (t2 >> 2);
    
    /* Use in computation */
    t4 = t3 + (p & q);
    return t4;
}

/* Test function 4: Nested control flow with safe operation */
int test_case_4(int val) {
    int a = val * 3;
    int b = val + 7;
    int c = 0;
    int d = 0;
    
    /* Loop to create more complex CFG */
    for (int i = 0; i < 3; i++) {
        a += i;
    }
    
    /* Condition based on computation */
    int cond = (a & 0x3) == 0;
    
    if (cond) {
        goto target_label_4;
    }
    
    /* Alternative path */
    b = b ^ 0x1234;
    
target_label_4:
    /* Safe arithmetic with constants */
    c = a + b + 5;
    
    return c;
}

/* Test function 5: Multiple jumps to same label */
int test_case_5(int x, int y, int z) {
    int t1 = x * y;
    int t2 = y * z;
    int t3 = z * x;
    int result = 0;
    
    /* First conditional jump */
    if (x > y) {
        goto common_label;
    }
    
    /* Some intermediate code */
    t1 = t1 + 10;
    
    /* Second conditional jump to same label */
    if (y < z) {
        goto common_label;
    }
    
    t2 = t2 - 5;
    goto end;
    
common_label:
    /* Instruction after label - simple assignment */
    result = t1 + t2 + t3;
    
end:
    return result;
}

/* Main driver that calls all test cases */
int main() {
    int results[5];
    int checksum = 0;
    
    /* Initialize with non-constant values */
    int a = global_seed;
    int b = global_seed * 2 + 1;
    int c = global_seed / 3;
    
    /* Run all test cases */
    results[0] = test_case_1(a, b);
    results[1] = test_case_2(b, c);
    results[2] = test_case_3(a, c);
    results[3] = test_case_4(b);
    results[4] = test_case_5(a, b, c);
    
    /* Compute checksum */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
        global_accumulator += results[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Accumulator: %d\n", global_accumulator);
    
    /* Also print individual results for debugging */
    for (int i = 0; i < 5; i++) {
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    return 0;
}
