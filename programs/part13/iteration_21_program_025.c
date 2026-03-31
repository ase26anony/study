/* test_11_operands.c */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Test 1: Inline assembly with exactly 11 operands */
void test_multi_operand_asm(void) {
    uint64_t o0, o1, o2, o3, o4;
    uint64_t i0 = 1, i1 = 2, i2 = 3, i3 = 4, i4 = 5, i5 = 6;
    
    /* 5 outputs + 6 inputs = 11 operands total */
    asm volatile (
        "# 11-operand asm\n\t"
        "mov %0, %5\n\t"
        "mov %1, %6\n\t"
        "mov %2, %7\n\t"
        "mov %3, %8\n\t"
        "mov %4, %9\n\t"
        : "=&r"(o0), "=&r"(o1), "=&r"(o2), "=&r"(o3), "=&r"(o4)
        : "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5)
        : "cc"
    );
    
    sink += o0 + o1 + o2 + o3 + o4;
}

/* Test 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
void test_large_atomic_cmpxchg(void) {
    typedef __int128 int128_t;
    _Atomic int128_t large_atomic = 0;
    int128_t old_val = 0;
    int128_t new_val = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t expected = 0;
    
    /* __atomic_compare_exchange_n expands to complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink += (uint64_t)large_atomic + (uint64_t)(large_atomic >> 64);
}
#endif

/* Test 3: Vector operations with complex shuffles */
void test_vector_shuffle(void) {
    typedef int v8si __attribute__((vector_size(32)));
    typedef int v4si __attribute__((vector_size(16)));
    
    v8si v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si v2 = {9, 10, 11, 12, 13, 14, 15, 16};
    v4si mask = {0, 2, 4, 6};
    
    /* Complex shuffle that may generate multi-operand RTL */
    v4si result = __builtin_shuffle(v1, v2, mask);
    
    for (int i = 0; i < 4; i++) {
        sink += result[i];
    }
}

/* Test 4: Multiple output builtin simulation */
void test_multi_output_builtin(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t x, y, z, w, v, u;
    
    /* Simulate a 6-input, 5-output operation = 11 operands */
    asm volatile (
        "# Multi-output operation\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "mul %3, %5, %7\n\t"
        "mul %4, %6, %8\n\t"
        : "=&r"(x), "=&r"(y), "=&r"(z), "=&r"(w), "=&r"(v)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(sink)
        : "cc"
    );
    
    sink += x + y + z + w + v;
}

/* Test 5: Memory barrier with many register clobbers */
void test_memory_barrier(void) {
    uint64_t r0, r1, r2, r3, r4, r5, r6;
    
    /* Memory barrier that clobbers many registers */
    asm volatile (
        "# Memory barrier with many operands\n\t"
        "mfence\n\t"
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4), "=r"(r5), "=r"(r6)
        : 
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "cc"
    );
    
    sink += r0 + r1 + r2 + r3 + r4 + r5 + r6;
}

int main(void) {
    printf("Testing 11-operand RTL generation...\n");
    
    /* Run each test many times to ensure RTL generation */
    for (int i = 0; i < 100000; i++) {
        test_multi_operand_asm();
        
#ifdef __SIZEOF_INT128__
        if (i % 1000 == 0) {
            test_large_atomic_cmpxchg();
        }
#endif
        
        if (i % 500 == 0) {
            test_vector_shuffle();
        }
        
        if (i % 200 == 0) {
            test_multi_output_builtin();
        }
        
        if (i % 1000 == 0) {
            test_memory_barrier();
        }
    }
    
    printf("Final sink value: %lu\n", (unsigned long)sink);
    return 0;
}
