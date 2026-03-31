#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int barrier(int x) {
    volatile int v = x;
    return v;
}

/* Test function for MIPS target */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of condition */
    int temp1 = a + 100;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    
    /* Use volatile to prevent constant folding */
    volatile int cond = a > b;
    
    /* Simple conditional jump to label */
    if (cond) {
        /* This should compile to a simple jump */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 + 5;
    return temp3;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    /* Uses independent temporary variables */
    temp3 = temp1 - temp2;  /* Simple arithmetic */
    
    /* Use result to prevent dead code elimination */
    return barrier(temp3);
}

/* Test function for SPARC target */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 * 3;
    int t4 = t2 + 7;
    
    /* Dynamic condition */
    volatile int flag = (x + y) & 1;
    
    if (flag) {
        /* Should generate simplejump_p */
        goto target_label_2;
    }
    
    /* Alternative path */
    t4 = t4 * 2;
    return t4;
    
target_label_2:
    /* Safe instruction: bitwise operation on locals */
    t4 = t3 ^ t4;  /* No trapping possible */
    
    return barrier(t4);
}

/* Generic test function */
int test_case_3(int seed) {
    /* Multiple independent variables */
    int v1 = seed;
    int v2 = seed * 3;
    int v3 = seed + 17;
    int v4 = seed - 5;
    
    /* Complex enough condition to not be optimized away */
    int condition = (v1 * v2) > (v3 * v4);
    
    if (condition) {
        /* Simple jump to label */
        goto target_label_3;
    }
    
    /* Other basic blocks */
    for (int i = 0; i < 3; i++) {
        v1 += i;
    }
    return v1;
    
target_label_3:
    /* Safe arithmetic on local variables */
    v1 = v2 + v3 - v4;  /* All local, no side effects */
    
    return barrier(v1);
}

/* Test with more complex control flow around the target pattern */
int test_case_4(int a, int b, int c) {
    int x = a;
    int y = b;
    int z = c;
    
    /* Multiple basic blocks before */
    if (a > 0) {
        x = x * 2;
    } else {
        x = x / 2;  /* Safe division by constant 2 */
    }
    
    /* Intermediate computation */
    y = y + x;
    z = z - x;
    
    /* Dynamic jump condition */
    volatile int jump_flag = (x ^ y ^ z) & 1;
    
    if (jump_flag) {
        /* Target jump */
        goto delay_slot_target;
    }
    
    /* Alternative path with loop */
    for (int i = 0; i < 2; i++) {
        y += i;
    }
    return y;
    
delay_slot_target:
    /* Instruction to potentially fill delay slot */
    /* Uses variables not involved in jump condition */
    int result = (y & 0xF) | (z & 0xF0);
    
    return barrier(result);
}

/* Test with multiple jumps and labels */
int test_case_5(int val) {
    int a = val;
    int b = val + 10;
    int c = val * 2;
    
    /* First conditional jump */
    if (a & 1) {
        goto label_a;
    }
    
    b = b + 5;
    
    /* Second conditional jump - target for delay slot filling */
    volatile int cond2 = (b > c);
    if (cond2) {
        /* This is the jump we want to optimize */
        goto target_for_delay;
    }
    
    c = c - 3;
    return c;
    
label_a:
    a = a * 2;
    return a;
    
target_for_delay:
    /* Safe instruction after label */
    b = c + a;  /* All local variables */
    
    return barrier(b);
}

int main() {
    int results[5];
    int checksum = 0;
    
    /* Initialize with non-constant values */
    int base = 42;
    
    /* Run all test cases */
    results[0] = test_case_1(base, base + 1);
    results[1] = test_case_2(base * 2, base / 2);
    results[2] = test_case_3(base ^ 0x1234);
    results[3] = test_case_4(base, base + 7, base - 3);
    results[4] = test_case_5(base * 3);
    
    /* Compute checksum */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        global_accumulator += results[i];
        printf("Test %d: %d\n", i + 1, results[i]);
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return checksum == 0 ? 0 : 1;
}
