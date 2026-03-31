#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) barrier_func1(int x) {
    return x ^ 0x55AA;
}

static int __attribute__((noinline)) barrier_func2(int x) {
    return x * 3 + 1;
}

/* Test function for MIPS target */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int arg1, int arg2) {
    /* Local temporaries - independent from jump condition */
    int temp_a = arg1 + 7;
    int temp_b = arg2 * 2;
    int temp_c = 0;
    int temp_d = 0;
    
    /* Create a non-trivial condition using barrier functions */
    int cond = barrier_func1(arg1) > barrier_func2(arg2);
    
    /* Simple conditional jump to label */
    if (cond) {
        /* This should compile to a simple jump to target_label */
        goto target_label;
    }
    
    /* Some other code to create basic blocks */
    temp_a = temp_b + 5;
    
    /* This is the target label - next instruction must be safe and movable */
target_label:
    /* Safe, non-jump instruction using independent temporaries */
    temp_c = temp_a + temp_b;  /* Simple arithmetic */
    
    /* Use result to prevent dead code elimination */
    return temp_c + (cond ? 1 : 0);
}

/* Test function for SPARC target */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int arg1, int arg2, int arg3) {
    /* Multiple independent temporary variables */
    int t1 = arg1 & 0xFF;
    int t2 = arg2 | 0x80;
    int t3 = arg3 ^ 0x40;
    int t4 = 0;
    int t5 = 0;
    
    /* Volatile read to prevent constant folding */
    volatile int v = arg1;
    int cond = (v > 0) && (arg2 < arg3);
    
    /* Additional basic blocks before the target jump */
    for (int i = 0; i < 2; i++) {
        t1 += i;
    }
    
    /* Simple conditional jump */
    if (cond) {
        goto sparc_target;
    }
    
    /* Alternative path */
    t2 = t3 - t1;
    
sparc_target:
    /* Safe instruction: bitwise operation on locals */
    t4 = t1 & t2;
    
    /* Another safe operation to create scheduling opportunities */
    t5 = t4 << 2;
    
    return t5 + (cond ? 100 : 200);
}

/* Generic test function with multiple jumps */
static int test_case_3(int base) {
    int a = base + 1;
    int b = base * 2;
    int c = base / 3;  /* Safe division - base is non-zero from main */
    int d = 0, e = 0, f = 0;
    
    /* Multiple conditions to create complex CFG */
    if (barrier_func1(base) > 100) {
        if (barrier_func2(base) < 200) {
            /* Target jump pattern */
            goto generic_target;
        }
        a = b - c;
    }
    
    /* Another basic block */
    b = a * 3;
    
generic_target:
    /* Multiple safe instructions after label */
    d = a + b;
    e = d ^ c;  /* Bitwise XOR is always safe */
    
    /* Use volatile to prevent reordering */
    volatile int v = e;
    f = v + 1;
    
    return f;
}

/* Test with nested control flow */
static int test_case_4(int x, int y) {
    int tmp1 = x;
    int tmp2 = y;
    int tmp3 = 0;
    int result = 0;
    
    /* Complex condition using volatile */
    volatile int vx = x;
    volatile int vy = y;
    
    if (vx != 0) {
        for (int i = 0; i < 3; i++) {
            tmp1 += i;
            if (vy > i) {
                /* This is our target jump */
                goto nested_target;
            }
        }
        tmp2 = tmp1 * 2;
    }
    
    /* Fall through path */
    tmp3 = tmp2 + 1;
    goto end;
    
nested_target:
    /* Safe instruction using only local temps */
    tmp3 = tmp1 | tmp2;  /* Bitwise OR is trap-free */
    
end:
    result = barrier_func1(tmp3);
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int sum = 0;
    
    /* Use command line args or defaults for variability */
    int arg1 = argc > 1 ? atoi(argv[1]) : 42;
    int arg2 = argc > 2 ? atoi(argv[2]) : 123;
    int arg3 = argc > 3 ? atoi(argv[3]) : 789;
    
    /* Ensure non-zero for safe division */
    if (arg1 == 0) arg1 = 1;
    if (arg2 == 0) arg2 = 1;
    if (arg3 == 0) arg3 = 1;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all test cases */
    sum += test_case_1(arg1, arg2);
    sum += test_case_2(arg1, arg2, arg3);
    sum += test_case_3(arg1);
    sum += test_case_4(arg2, arg3);
    
    /* Update global to prevent optimization */
    global_accumulator += sum;
    
    printf("Result checksum: %d\n", sum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return (sum > 0) ? 0 : 1;
}
