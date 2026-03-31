/* Test program for triggering delay slot filling logic in GCC reorg.cc */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Barrier functions to prevent optimization */
int __attribute__((noinline)) get_input(int x) {
    return x ^ global_seed;
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
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    
    /* Simple conditional jump to label */
    if (a > b) {
        /* This should compile to a simple jump to label */
        goto target_label1;
    }
    
    /* Some other code to create CFG complexity */
    temp3 = temp3 * 2;
    
target_label1:
    /* Safe, non-jump instruction immediately after label */
    temp3 = temp3 | 0x1F;  /* Simple bitwise operation */
    
    /* Use result to prevent elimination */
    return use_result(temp3);
}

/* Test function 2: Different variable pattern for SPARC */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_pattern2(int x, int y) {
    /* Independent temporaries */
    int local_a = x + y;
    int local_b = x - y;
    int local_c = local_a * local_b;
    
    /* Volatile read to prevent constant folding */
    volatile int v = global_seed;
    
    /* Conditional jump with non-trivial condition */
    if ((x & 0xF) != (y & 0xF)) {
        goto target_label2;
    }
    
    /* Alternative path */
    local_c = local_c >> 2;
    
target_label2:
    /* Safe arithmetic after label */
    local_c = (local_c + 1) & 0xFF;
    
    return use_result(local_c);
}

/* Test function 3: Multiple basic blocks */
int test_pattern3(int val) {
    int t1 = val * 3;
    int t2 = val + 5;
    int t3 = 0;
    
    /* Create multiple basic blocks */
    for (int i = 0; i < 3; i++) {
        t1 += i;
    }
    
    /* Jump condition using function argument */
    if (get_input(val) % 2 == 0) {
        goto target_label3;
    }
    
    /* Another basic block */
    t2 = t2 << 1;
    
target_label3:
    /* Simple safe operation */
    t3 = t1 ^ t2;
    
    return use_result(t3);
}

/* Test function 4: Nested control flow */
int test_pattern4(int a, int b, int c) {
    int tmp1 = a;
    int tmp2 = b;
    int tmp3 = c;
    
    /* Outer condition */
    if (a > 0) {
        /* Inner condition for the target jump */
        if (b != c) {
            goto target_label4;
        }
        tmp1 = tmp1 * 2;
    }
    
    tmp2 = tmp2 + 10;
    
target_label4:
    /* Safe logical operation */
    tmp3 = tmp3 & 0x7F;
    
    return use_result(tmp1 + tmp2 + tmp3);
}

/* Test function 5: More complex but still safe */
int test_pattern5(int base) {
    int x = base;
    int y = base + 1;
    int z = base + 2;
    
    /* Multiple independent calculations */
    int sum = x + y + z;
    int prod = x * y;
    
    /* Condition using volatile */
    volatile int check = global_seed;
    if ((check & 1) == (base & 1)) {
        goto target_label5;
    }
    
    prod = prod / 2;  /* This is safe division by constant 2 */
    
target_label5:
    /* Safe bit manipulation */
    sum = sum ^ prod;
    
    return use_result(sum);
}

/* Main driver */
int main() {
    int result = 0;
    
    /* Call test functions with different inputs */
    result += test_pattern1(10, 5);
    result += test_pattern1(5, 10);
    
    result += test_pattern2(15, 7);
    result += test_pattern2(7, 15);
    
    result += test_pattern3(20);
    result += test_pattern3(21);
    
    result += test_pattern4(1, 2, 3);
    result += test_pattern4(3, 2, 2);
    
    result += test_pattern5(8);
    result += test_pattern5(9);
    
    /* Print checksum */
    printf("Accumulator: %d\n", global_accumulator);
    printf("Result checksum: %d\n", result);
    
    /* Verify execution */
    if (global_accumulator != 0) {
        printf("Test functions executed successfully\n");
    }
    
    return 0;
}
