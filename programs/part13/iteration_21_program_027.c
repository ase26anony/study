/* test_11_operand_optab.c
 * Comprehensive test to trigger 11-operand RTL generation in GCC's optabs
 * Compile with: gcc -O3 -march=native -fdump-rtl-expand -fdump-rtl-combine -o test test_11_operand_optab.c
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile sink to prevent optimization */
volatile uint64_t sink = 0;

/* Strategy 1: Inline assembly with exactly 11 operands */
__attribute__((noinline))
uint64_t test_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c, 
                                uint64_t d, uint64_t e, uint64_t f,
                                uint64_t g, uint64_t h, uint64_t i,
                                uint64_t j, uint64_t k) {
    uint64_t out0, out1, out2, out3, out4;
    
    /* 
     * Inline asm with 11 operands total:
     * 5 outputs + 6 inputs = 11 operands
     * Using early-clobber (&) constraints to force separate registers
     */
    asm volatile (
        "# 11-operand dummy instruction\n\t"
        "add %0, %5, %6\n\t"
        "add %1, %7, %8\n\t"
        "add %2, %9, %10\n\t"
        "mul %3, %0, %1\n\t"
        "add %4, %2, %3"
        : "=&r" (out0), "=&r" (out1), "=&r" (out2), 
          "=&r" (out3), "=&r" (out4)          /* 5 outputs */
        : "r" (a), "r" (b), "r" (c),          /* 6 inputs */
          "r" (d), "r" (e), "r" (f)
        : "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + g + h + i + j + k;
}

/* Strategy 2: Atomic operations on 128-bit types */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

_Atomic uint128_t large_atomic;

__attribute__((noinline))
uint128_t test_atomic_128bit(uint128_t new_val) {
    uint128_t expected = 0;
    uint128_t desired = new_val;
    
    /* __atomic_compare_exchange_n can generate complex RTL with many operands */
    __atomic_compare_exchange_n(&large_atomic, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Perform exchange which may also generate multi-operand RTL */
    return __atomic_exchange_n(&large_atomic, new_val + 1, __ATOMIC_SEQ_CST);
}
#endif

/* Strategy 3: Vector operations with complex shuffles */
#define VEC_SIZE 32
typedef int v8si __attribute__((vector_size(VEC_SIZE)));

__attribute__((noinline))
v8si test_vector_shuffle(v8si a, v8si b, int mask0, int mask1) {
    /* Create a complex shuffle mask from runtime values */
    int mask[8] = {
        mask0 & 7, (mask0 >> 3) & 7, (mask1 & 7) + 4, ((mask1 >> 3) & 7) + 4,
        (mask0 & 3) + 2, (mask1 & 3) + 6, (mask0 >> 2) & 3, (mask1 >> 2) & 3
    };
    
    /* __builtin_shufflevector with runtime-dependent mask */
    v8si result = __builtin_shufflevector(a, b, 
        mask[0], mask[1], mask[2], mask[3],
        mask[4], mask[5], mask[6], mask[7]);
    
    /* Additional operation to ensure RTL generation */
    return result + a - b;
}

/* Strategy 4: Complex builtin with many arguments */
__attribute__((noinline))
uint64_t test_builtin_many_args(uint64_t a, uint64_t b, uint64_t c,
                                uint64_t d, uint64_t e, uint64_t f,
                                uint64_t g, uint64_t h) {
    /* Simulate a complex operation that might expand to many RTL operands */
    uint64_t result = 0;
    
    /* Chain of operations that might be combined */
    result = __builtin_add_overflow(a, b, &result) ? result : 0;
    result = __builtin_mul_overflow(result, c, &result) ? result : 0;
    result = __builtin_sub_overflow(result, d, &result) ? result : 0;
    result = __builtin_add_overflow(result, e, &result) ? result : 0;
    result = __builtin_mul_overflow(result, f, &result) ? result : 0;
    result = __builtin_sub_overflow(result, g, &result) ? result : 0;
    result = __builtin_add_overflow(result, h, &result) ? result : 0;
    
    return result;
}

/* Strategy 5: Custom multi-output instruction simulation */
__attribute__((noinline))
void test_custom_multi_io(uint64_t in0, uint64_t in1, uint64_t in2,
                          uint64_t in3, uint64_t in4, uint64_t in5,
                          uint64_t* out0, uint64_t* out1, uint64_t* out2,
                          uint64_t* out3, uint64_t* out4) {
    /* 
     * Simulate a custom instruction with 6 inputs and 5 outputs (11 total)
     * This asm statement has exactly 11 operands
     */
    asm volatile (
        "# Custom 6-in, 5-out instruction\n\t"
        "mov %0, %5\n\t"      /* out0 = in0 */
        "add %1, %5, %6\n\t"  /* out1 = in0 + in1 */
        "mul %2, %6, %7\n\t"  /* out2 = in1 * in2 */
        "sub %3, %8, %9\n\t"  /* out3 = in3 - in4 */
        "xor %4, %9, %10\n\t" /* out4 = in4 ^ in5 */
        : "=r" (*out0), "=r" (*out1), "=r" (*out2), 
          "=r" (*out3), "=r" (*out4)          /* 5 outputs */
        : "r" (in0), "r" (in1), "r" (in2),    /* 6 inputs */
          "r" (in3), "r" (in4), "r" (in5)
        : "cc"
    );
}

/* Main test driver */
int main() {
    uint64_t checksum = 0;
    
    /* Initialize test values */
    uint64_t vals[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    
    printf("Testing 11-operand RTL generation patterns...\n");
    
    /* Run tests in a loop to increase coverage probability */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Modify values slightly each iteration */
        for (int i = 0; i < 11; i++) {
            vals[i] = (vals[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Test 1: Multi-operand inline assembly */
        checksum ^= test_multi_operand_asm(
            vals[0], vals[1], vals[2], vals[3], vals[4],
            vals[5], vals[6], vals[7], vals[8], vals[9], vals[10]
        );
        
        /* Test 2: 128-bit atomic operations */
        #ifdef __SIZEOF_INT128__
        uint128_t big_val = ((uint128_t)vals[0] << 64) | vals[1];
        uint128_t atomic_result = test_atomic_128bit(big_val);
        checksum ^= (uint64_t)atomic_result ^ (uint64_t)(atomic_result >> 64);
        #endif
        
        /* Test 3: Vector shuffle operations */
        if (iteration % 100 == 0) { /* Don't do this every iteration */
            v8si vec_a = {vals[0], vals[1], vals[2], vals[3], 
                         vals[4], vals[5], vals[6], vals[7]};
            v8si vec_b = {vals[1], vals[2], vals[3], vals[4],
                         vals[5], vals[6], vals[7], vals[8]};
            v8si vec_result = test_vector_shuffle(vec_a, vec_b, 
                                                 vals[9] & 255, vals[10] & 255);
            
            /* Extract some values from vector */
            int* vec_ptr = (int*)&vec_result;
            for (int i = 0; i < 4; i++) {
                checksum ^= vec_ptr[i];
            }
        }
        
        /* Test 4: Builtin with many arguments */
        checksum ^= test_builtin_many_args(
            vals[0], vals[1], vals[2], vals[3],
            vals[4], vals[5], vals[6], vals[7]
        );
        
        /* Test 5: Custom multi-input/output instruction */
        if (iteration % 50 == 0) {
            uint64_t outputs[5];
            test_custom_multi_io(
                vals[0], vals[1], vals[2], vals[3], vals[4], vals[5],
                &outputs[0], &outputs[1], &outputs[2], &outputs[3], &outputs[4]
            );
            
            for (int i = 0; i < 5; i++) {
                checksum ^= outputs[i];
            }
        }
        
        /* Prevent optimization */
        sink = checksum;
    }
    
    printf("Final checksum: 0x%016lx\n", checksum);
    printf("Sink value: %lu\n", (unsigned long)sink);
    
    return 0;
}
