/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Volatile variables to create memory barriers */
volatile int global_counter = 0;
volatile int* volatile global_ptr = &global_counter;

/* Helper functions that won't be inlined */
NOINLINE int helper1(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + (a ^ b);
}

NOINLINE float helper2(float a, float b) {
    asm volatile ("" : : "x"(a), "x"(b) : "memory");
    return a * b - a / b;
}

NOINLINE void helper3(volatile int* p) {
    *p = (*p * 1103515245 + 12345) & 0x7fffffff;
    asm volatile ("mfence" ::: "memory");
}

/* Function with complex scheduling requirements */
NOINLINE int complex_scheduling_region(int seed) {
    int result = seed;
    volatile int barrier = 0;
    
    /* Mixed integer operations creating dependency chains */
    int a = seed * 3;
    int b = a + 7;
    int c = b ^ 0xABCD;
    int d = c * 11;
    int e = d >> 3;
    int f = e & 0xFF;
    
    /* Volatile memory accesses creating scheduling barriers */
    barrier = 1;
    global_counter += f;
    *global_ptr = *global_ptr + 1;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull $0x9E3779B9, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(f)
        : "eax", "memory", "cc"
    );
    
    /* Function call creating scheduling boundary */
    result = helper1(result, barrier);
    
    return result;
}

/* Function with vector operations */
NOINLINE __m128i vector_operations(__m128i a, __m128i b) {
    __m128i c, d, e, f;
    
    /* Multiple vector operations */
    c = _mm_add_epi32(a, b);
    d = _mm_mullo_epi32(c, _mm_set1_epi32(7));
    e = _mm_slli_epi32(d, 2);
    f = _mm_xor_si128(e, _mm_set1_epi32(0xFFFFFFFF));
    
    /* Memory barrier */
    asm volatile ("" ::: "memory");
    
    /* More operations */
    c = _mm_sub_epi32(f, a);
    d = _mm_mul_epu32(c, b);
    
    return _mm_add_epi32(c, d);
}

/* Function with nested control flow */
NOINLINE int nested_control_flow(int x) {
    int result = 0;
    volatile int v = x;
    
    /* Switch with multiple cases */
    switch (x & 7) {
        case 0:
            result = x * 2;
            asm volatile ("nop" ::: "memory");
            break;
        case 1:
            result = x + 257;
            /* Fall through */
        case 2:
            result ^= 0x55AA;
            helper3(&global_counter);
            break;
        case 3:
            for (int i = 0; i < 3; i++) {
                result += (x << i);
                if (result > 1000) break;
            }
            break;
        case 4:
            result = helper1(x, x + 1);
            goto common_label;
        case 5:
            result = x | 0xFF00;
            /* Fall through */
        default:
            result = result * 3 - 1;
            common_label:
            result &= 0xFFFF;
            break;
    }
    
    /* Nested loops with data-dependent exit */
    int temp = result;
    for (int i = 0; i < 8; i++) {
        int j = 0;
        while (j < 4) {
            temp = (temp * 1664525 + 1013904223) & 0x7FFFFFFF;
            if (temp & (1 << j)) break;
            j++;
        }
        result ^= temp;
    }
    
    return result;
}

/* Main scheduling stress test */
int main() {
    int checksum = 0;
    float fchecksum = 0.0f;
    
    /* Initialize arrays with different data types */
    int int_array[256];
    float float_array[256];
    volatile int volatile_array[256];
    
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 0.1f;
        volatile_array[i] = i;
    }
    
    /* Region 1: Complex integer operations with volatile accesses */
    for (int i = 0; i < 100; i++) {
        checksum += complex_scheduling_region(i);
        
        /* Volatile access creating scheduling barrier */
        volatile_array[i % 256] = checksum;
        asm volatile ("" ::: "memory");
        
        /* Independent instructions that can execute in parallel */
        int a = checksum + 1;
        int b = checksum * 2;
        int c = checksum & 0xFF;
        int d = checksum ^ 0x1234;
        
        checksum = a + b + c + d;
    }
    
    /* Region 2: Vector/SIMD operations */
    __m128i vec_a = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_b = _mm_set_epi32(5, 6, 7, 8);
    
    for (int i = 0; i < 50; i++) {
        __m128i result = vector_operations(vec_a, vec_b);
        
        /* Extract results */
        int res_array[4];
        _mm_storeu_si128((__m128i*)res_array, result);
        
        for (int j = 0; j < 4; j++) {
            checksum += res_array[j];
        }
        
        /* Modify vectors */
        vec_a = _mm_add_epi32(vec_a, _mm_set1_epi32(1));
        vec_b = _mm_add_epi32(vec_b, _mm_set1_epi32(2));
        
        /* Memory barrier between iterations */
        asm volatile ("sfence" ::: "memory");
    }
    
    /* Region 3: Nested control flow with mixed operations */
    for (int i = 0; i < 200; i++) {
        checksum += nested_control_flow(i);
        
        /* Mix with floating point operations */
        float f1 = float_array[i % 256];
        float f2 = helper2(f1, f1 + 1.0f);
        fchecksum += f2;
        
        /* Convert float to int for checksum */
        int int_part = (int)f2;
        checksum ^= int_part;
        
        /* Architecture-specific builtins */
        checksum += __builtin_popcount(i);
        checksum ^= __builtin_ctz(i | 1);
    }
    
    /* Region 4: Tight loop with function calls and inline assembly */
    int loop_var = 0;
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < 20; inner++) {
            /* Data-dependent computation */
            loop_var = (loop_var * 1103515245 + 12345) & 0x7fffffff;
            
            /* Function call every few iterations */
            if ((inner & 3) == 0) {
                checksum += helper1(loop_var, inner);
            }
            
            /* Inline assembly with register constraints */
            int temp;
            asm volatile (
                "movl %1, %%ecx\n\t"
                "roll $3, %%ecx\n\t"
                "movl %%ecx, %0\n\t"
                : "=r"(temp)
                : "r"(loop_var)
                : "ecx", "cc"
            );
            
            checksum += temp;
            
            /* Memory barrier */
            if ((inner & 7) == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Profile-guided optimization hint */
        if (__builtin_expect(outer < 5, 1)) {
            checksum += 1000;
        } else {
            checksum -= 500;
        }
    }
    
    /* Region 5: Switch statement with computed goto-like behavior */
    int state = checksum & 0xF;
    for (int i = 0; i < 100; i++) {
        switch (state) {
            case 0: case 8:
                checksum = checksum * 3 + 1;
                state = (checksum >> 4) & 0xF;
                break;
            case 1: case 9:
                checksum ^= 0xAAAAAAAA;
                state = (checksum >> 8) & 0xF;
                break;
            case 2: case 10:
                checksum = (checksum << 1) | (checksum >> 31);
                state = (state + 1) & 0xF;
                break;
            case 3: case 11:
                checksum += int_array[checksum & 0xFF];
                state = 5;
                break;
            case 4: case 12:
                checksum -= 777;
                state = 6;
                break;
            case 5: case 13:
                checksum |= 0x55555555;
                state = 7;
                break;
            case 6: case 14:
                checksum &= 0x33333333;
                state = 0;
                break;
            case 7: case 15:
                checksum = ~checksum;
                state = 3;
                break;
        }
        
        /* Volatile write every iteration */
        volatile_array[i & 0xFF] = checksum;
    }
    
    /* Final mixing */
    checksum += (int)fchecksum;
    checksum ^= global_counter;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(checksum));
    
    return checksum & 0x7FFFFFFF;
}
