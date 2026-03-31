/* Test program to trigger 11-operand RTL generation in GCC's optabs */
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Strategy 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm() {
    int o0, o1, o2, o3;
    int i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6, i6 = 7, i7 = 8;
    
    /* 11 operands: 4 outputs + 8 inputs - 1 reused (i0) = 11 total operands in constraints */
    asm volatile (
        "# Multi-operand asm with 11 constraint operands\n\t"
        "add %0, %4, %5\n\t"
        "add %1, %6, %7\n\t"
        "add %2, %8, %9\n\t"
        "add %3, %10, %11"
        : "=&r" (o0), "=&r" (o1), "=&r" (o2), "=&r" (o3)  /* 4 outputs */
        : "r" (i0), "r" (i1), "r" (i2), "r" (i3),         /* 8 inputs */
          "r" (i4), "r" (i5), "r" (i6), "r" (i7)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3;
}

/* Strategy 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg() {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    for (int i = 0; i < 100; i++) {
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (int)(expected & 0xFFFFFFFF);
    }
}
#endif

/* Strategy 3: Vector operations with complex shuffles */
void test_vector_shuffle() {
    typedef int v8si __attribute__((vector_size(32)));
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex shuffle that may generate many operands */
    int mask[8] = {1, 3, 5, 7, 0, 2, 4, 6};
    
    for (int i = 0; i < 100; i++) {
        /* __builtin_shufflevector with runtime indices can be complex */
        v8si result = __builtin_shufflevector(a, b, 
            mask[0], mask[1], mask[2], mask[3],
            mask[4], mask[5], mask[6], mask[7]);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 8; j++) {
            sink += result[j];
        }
        
        /* Modify mask slightly */
        mask[i % 8] = (mask[i % 8] + 1) & 7;
    }
}

/* Strategy 4: Custom multi-output operation via inline asm */
void test_custom_multi_output() {
    int out0, out1, out2, out3, out4;
    int in0 = 1, in1 = 2, in2 = 3, in3 = 4, in4 = 5, in5 = 6;
    
    /* 11 operands: 5 outputs + 6 inputs = 11 */
    asm volatile (
        "# Custom operation with 5 outputs, 6 inputs\n\t"
        "mov %0, %5\n\t"
        "add %1, %5, %6\n\t"
        "add %2, %6, %7\n\t"
        "add %3, %7, %8\n\t"
        "add %4, %8, %9"
        : "=&r" (out0), "=&r" (out1), "=&r" (out2), 
          "=&r" (out3), "=&r" (out4)                /* 5 outputs */
        : "r" (in0), "r" (in1), "r" (in2),          /* 6 inputs */
          "r" (in3), "r" (in4), "r" (in5)
        : "cc"
    );
    
    sink += out0 + out1 + out2 + out3 + out4;
}

/* Strategy 5: Complex builtin with many arguments */
void test_complex_builtin() {
    /* __builtin_cpu_supports with many features checked */
    int features = 0;
    
    /* Each string argument becomes an operand during expansion */
    features |= __builtin_cpu_supports("mmx");
    features |= __builtin_cpu_supports("sse");
    features |= __builtin_cpu_supports("sse2");
    features |= __builtin_cpu_supports("sse3");
    features |= __builtin_cpu_supports("ssse3");
    features |= __builtin_cpu_supports("sse4.1");
    features |= __builtin_cpu_supports("sse4.2");
    features |= __builtin_cpu_supports("avx");
    features |= __builtin_cpu_supports("avx2");
    
    sink += features;
}

int main() {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 10000; i++) {
        test_multi_operand_asm();
        test_custom_multi_output();
        test_vector_shuffle();
        test_complex_builtin();
        
        #ifdef __SIZEOF_INT128__
        if (i % 100 == 0) {
            test_large_atomic_cmpxchg();
        }
        #endif
        
        /* Prevent loop unrolling from simplifying things */
        if (i % 7 == 0) {
            sink += i;
        }
    }
    
    printf("Final sink value: %d\n", sink);
    printf("Test completed.\n");
    
    return 0;
}
