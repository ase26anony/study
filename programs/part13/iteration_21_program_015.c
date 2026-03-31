/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Large atomic compare-and-swap on __int128 */
void test_atomic_cmpxchg(void) {
    _Atomic __int128 large_atomic = 0;
    __int128 old_val = 0;
    __int128 new_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 expected = 0;
    
    for (int i = 0; i < 10000; i++) {
        /* This should generate RTL with many operands for 16-byte atomic */
        __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                    0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        sink += (uint64_t)large_atomic;
    }
}

/* Test 2: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    for (int i = 0; i < 10000; i++) {
        /* 5 outputs + 6 inputs = 11 total operands */
        asm volatile (
            "# Dummy 11-operand asm\n\t"
            "add %0, %5, %6\n\t"
            "add %1, %7, %8\n\t"
            "add %2, %9, %10\n\t"
            "mul %3, %0, %1\n\t"
            "sub %4, %2, %3"
            : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=r"(o3), "=r"(o4)
            : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
            : "cc"
        );
        
        sink += o0 + o1 + o2 + o3 + o4;
        i0++; i1++; i2++; i3++; i4++; i5++;
    }
}

/* Test 3: Vector shuffle with complex mask */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 8, 2, 10};  /* Mix elements from both vectors */
    
    for (int i = 0; i < 10000; i++) {
        /* Complex shuffle that may expand to many RTL operands */
        v4si result = __builtin_shuffle(v1, v2, mask);
        
        /* Use result to prevent elimination */
        for (int j = 0; j < 4; j++) {
            sink += result[j];
        }
        
        /* Modify mask slightly each iteration */
        mask[0] = (mask[0] + 1) & 0xF;
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    /* Simulate a fused multiply-add with multiple outputs */
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
    double out1, out2, out3, out4;
    
    for (int i = 0; i < 10000; i++) {
        /* Use inline asm to simulate 4 outputs + 5 inputs = 9 operands */
        /* Add two dummy inputs to reach 11 */
        double dummy1 = i * 0.1, dummy2 = i * 0.2;
        
        asm volatile (
            "# Multi-output operation\n\t"
            "fmadd %0, %4, %5, %6\n\t"
            "fmadd %1, %7, %8, %9\n\t"
            "fadd %2, %0, %1\n\t"
            "fsub %3, %0, %1"
            : "=f"(out1), "=f"(out2), "=f"(out3), "=f"(out4)
            : "f"(a), "f"(b), "f"(c), "f"(d), "f"(e), "f"(dummy1), "f"(dummy2)
            : 
        );
        
        sink += (uint64_t)(out1 + out2 + out3 + out4);
        a += 0.1; b += 0.1; c += 0.1; d += 0.1; e += 0.1;
    }
}

/* Test 5: Complex memory operation with many addressing modes */
void test_complex_memop(void) {
    struct LargeStruct {
        __int128 a;
        __int128 b;
        __int128 c;
        __int128 d;
    } data[4];
    
    for (int i = 0; i < 10000; i++) {
        /* Complex memory copy with overlapping regions */
        /* May generate RTL with many operands for address calculation */
        __int128 temp = data[0].a;
        data[0].a = data[1].b;
        data[1].b = data[2].c;
        data[2].c = data[3].d;
        data[3].d = temp;
        
        sink += (uint64_t)data[i % 4].a;
    }
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run all tests to maximize coverage chances */
    test_atomic_cmpxchg();
    printf("Atomic test complete, sink = %lu\n", (unsigned long)sink);
    
    test_multi_operand_asm();
    printf("Multi-operand asm test complete, sink = %lu\n", (unsigned long)sink);
    
    test_vector_shuffle();
    printf("Vector shuffle test complete, sink = %lu\n", (unsigned long)sink);
    
    test_multi_output_builtin();
    printf("Multi-output builtin test complete, sink = %lu\n", (unsigned long)sink);
    
    test_complex_memop();
    printf("Complex memop test complete, sink = %lu\n", (unsigned long)sink);
    
    printf("All tests completed. Final sink = %lu\n", (unsigned long)sink);
    return 0;
}
