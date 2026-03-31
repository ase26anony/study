/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) use_result(int x) {
    global_accumulator += x;
    return x;
}

/* Test function 1: Basic pattern for MIPS */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_pattern1(int a, int b) {
    /* Create temporaries independent of jump condition */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    
    /* Simple conditional jump using input-dependent variable */
    if (a > 0) {
        goto target_label1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 + 1;
    
target_label1:
    /* Safe, non-jump instruction after label */
    temp3 = temp3 & 0x0F0F0F0F;
    
    /* Use the result to prevent dead code elimination */
    return use_result(temp3);
}

/* Test function 2: Different condition pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_pattern2(int x, int y) {
    volatile int v = x; /* Prevent constant propagation */
    int local1 = y + 5;
    int local2 = x * 3;
    int local3 = local1 | local2;
    
    /* Another simple conditional jump */
    if (v != 0) {
        goto target_label2;
    }
    
    /* Alternative path */
    local3 = local3 - 1;
    
target_label2:
    /* Safe arithmetic operation after label */
    local3 = local3 + 7;
    
    return use_result(local3);
}

/* Test function 3: Nested control flow */
int test_pattern3(int n) {
    int acc = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int tmp1 = i * i;
        int tmp2 = tmp1 + 1;
        
        /* Jump inside loop */
        if (i & 1) {
            goto loop_target;
        }
        
        tmp2 = tmp2 * 2;
        continue;
        
    loop_target:
        /* Safe operation after label */
        tmp2 = tmp2 >> 1;
        
        acc += tmp2;
    }
    
    return use_result(acc);
}

/* Test function 4: Multiple jumps to same label */
int test_pattern4(int a, int b, int c) {
    int val = a + b + c;
    int mask = 0xFF;
    
    if (a > b) {
        goto common_target;
    }
    
    if (b < c) {
        val = val ^ 0x1234;
        goto common_target;
    }
    
    val = val + 100;
    
common_target:
    /* Safe bitwise operation */
    val = val & mask;
    
    return use_result(val);
}

/* Test function 5: With function call barrier */
int __attribute__((noinline)) helper(int x) {
    return x * 2;
}

int test_pattern5(int x) {
    int a = get_input(x);
    int b = helper(x);
    int result = 0;
    
    if (a > 100) {
        goto compute_label;
    }
    
    result = a + b;
    goto end;
    
compute_label:
    /* Safe computation after label */
    result = (a - b) * 3;
    
end:
    return use_result(result);
}

/* Main driver */
int main(int argc, char *argv[]) {
    int i;
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int result = 0;
    
    /* Call test patterns with varying inputs */
    for (i = 0; i < 10; i++) {
        int input = seed + i * 17;
        
        result ^= test_pattern1(input, input + 1);
        result ^= test_pattern2(input, input * 2);
        result ^= test_pattern3(input % 5 + 1);
        result ^= test_pattern4(input, input + 2, input + 3);
        result ^= test_pattern5(input);
    }
    
    /* Also use global accumulator */
    result ^= global_accumulator;
    
    printf("Result checksum: 0x%08X\n", result);
    return 0;
}
