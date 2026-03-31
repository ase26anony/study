/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-schedule-insns -fselective-scheduling2 -march=native -mtune=native */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Volatile globals to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int* g_volatile_ptr = NULL;
volatile float g_volatile_float = 0.0f;

/* Noinline functions to force calls */
NOINLINE int helper1(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return (a * b) ^ (a + b);
}

NOINLINE float helper2(float x, float y) {
    asm volatile ("" : : "x"(x), "x"(y) : "memory", "xmm0", "xmm1");
    return x * y - x / y;
}

NOINLINE void helper3(volatile int* p) {
    *p = (*p * 37) ^ 0xDEADBEEF;
    asm volatile ("mfence" ::: "memory");
}

/* Always inline to create scheduling regions */
ALWAYS_INLINE int complex_region1(int seed) {
    int a = seed;
    volatile int* vp = &g_volatile_counter;
    
    /* Mixed operations creating dependencies */
    a = a + *vp;
    a = a * 3;
    a = a >> 2;
    a = a | 0xFF;
    a = a ^ (a << 3);
    
    /* Inline assembly with clobbers */
    asm volatile ("nop; nop; nop" : "+r"(a) : : "memory", "eax", "ebx");
    
    /* Memory barrier */
    asm volatile ("" ::: "memory");
    
    return a;
}

ALWAYS_INLINE __m128i vector_ops(__m128i v1, __m128i v2) {
    /* Vector operations for target-specific scheduling */
    __m128i r1 = _mm_add_epi32(v1, v2);
    __m128i r2 = _mm_mullo_epi32(r1, v2);
    __m128i r3 = _mm_slli_epi32(r2, 3);
    __m128i r4 = _mm_xor_si128(r3, _mm_set1_epi32(0xAAAAAAAA));
    
    /* Memory clobber to prevent reordering */
    asm volatile ("" ::: "memory");
    
    return _mm_add_epi32(r4, _mm_set1_epi32(1));
}

/* Function with switch creating multiple basic blocks */
NOINLINE int switch_scheduler(int mode, int value) {
    int result = value;
    
    switch (mode & 7) {
        case 0:
            result = (result * 3) >> 1;
            /* Fall through */
        case 1:
            result ^= 0x12345678;
            result = __builtin_popcount(result);
            break;
        case 2:
            result = __builtin_ctz(result | 1);
            result = result * 7 + 3;
            break;
        case 3:
            result = (result << 4) | (result >> 28);
            /* Fall through */
        case 4:
            result = helper1(result, result + 1);
            asm volatile ("pause" ::: "memory");
            break;
        case 5:
            result = complex_region1(result);
            /* Fall through */
        case 6:
            result = (result & 0x55555555) + ((result >> 1) & 0x55555555);
            result = (result & 0x33333333) + ((result >> 2) & 0x33333333);
            break;
        case 7:
            result = (result * 0xCCCCCCCD) >> 35;  /* Division by 10 */
            result = result * 10;
            break;
    }
    
    /* Memory barrier between switch cases */
    asm volatile ("" ::: "memory");
    
    return result;
}

/* Nested loops with data-dependent exits */
NOINLINE int nested_loop_scheduler(int limit) {
    int sum = 0;
    volatile int* vp = &g_volatile_counter;
    
    for (int i = 0; i < limit; i++) {
        int inner = i * 2;
        
        /* Inner loop with data-dependent exit */
        for (int j = 0; j < 8; j++) {
            if (inner > *vp) {
                inner = helper1(inner, j);
                break;
            }
            inner = (inner * 3 + 1) ^ j;
            
            /* Inline assembly with specific register constraints */
            asm volatile ("addl %%ebx, %%eax" 
                         : "+a"(inner) 
                         : "b"(j) 
                         : "cc", "memory");
        }
        
        sum += inner;
        
        /* Volatile access creating scheduling barrier */
        *vp = (*vp + 1) & 0xFF;
    }
    
    return sum;
}

/* Main function with multiple complex scheduling regions */
int main() {
    int result = 0x12345678;
    float fresult = 3.14159f;
    
    /* Initialize volatile pointer */
    int local_volatile = 42;
    g_volatile_ptr = &local_volatile;
    
    /* Region 1: Mixed integer operations with volatile */
    for (int i = 0; i < 100; i++) {
        result = complex_region1(result);
        result ^= g_volatile_counter;
        
        /* Function call creating scheduling boundary */
        result = helper1(result, i);
        
        /* Memory clobber */
        asm volatile ("" ::: "memory");
    }
    
    /* Region 2: Vector operations */
    __m128i vec1 = _mm_set_epi32(result, result + 1, result + 2, result + 3);
    __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
    
    for (int i = 0; i < 50; i++) {
        vec1 = vector_ops(vec1, vec2);
        vec2 = _mm_add_epi32(vec2, _mm_set1_epi32(1));
        
        /* Extract and use result to prevent elimination */
        int arr[4];
        _mm_storeu_si128((__m128i*)arr, vec1);
        result ^= arr[0] ^ arr[1] ^ arr[2] ^ arr[3];
    }
    
    /* Region 3: Switch-based scheduling */
    for (int i = 0; i < 200; i++) {
        result = switch_scheduler(result & 7, result);
        
        /* Interleave with float operations */
        fresult = helper2(fresult, (float)(result & 0xFF));
        
        /* Volatile memory operation */
        helper3(&local_volatile);
        result ^= local_volatile;
    }
    
    /* Region 4: Nested loops */
    result += nested_loop_scheduler(25);
    
    /* Region 5: Independent instruction parallelism */
    int a = result, b = result + 1, c = result + 2, d = result + 3;
    
    /* Many independent operations */
    a = (a * 3) >> 1;
    b = (b ^ 0xAAAAAAAA) + 1;
    c = __builtin_popcount(c);
    d = (d << 4) | (d >> 28);
    
    asm volatile ("" ::: "memory");
    
    a = helper1(a, b);
    b = helper1(b, c);
    c = helper1(c, d);
    d = helper1(d, a);
    
    asm volatile ("" ::: "memory");
    
    a = (a + b) * (c - d);
    b = (b | c) & (d ^ a);
    c = __builtin_ctz(c | 1) * 7;
    d = (d * 0xCCCCCCCD) >> 35;
    
    result = a ^ b ^ c ^ d;
    
    /* Region 6: Computed goto to create complex CFG */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    int idx = result & 3;
    
    goto *labels[idx];
    
L0:
    result = (result * 11) >> 3;
    /* Fall through */
L1:
    result ^= 0xF0F0F0F0;
    result = __builtin_bswap32(result);
    goto L4;
L2:
    result = (result & 0x33333333) << 2;
    result |= (result >> 2) & 0x33333333;
    goto L4;
L3:
    result = helper1(result, result >> 16);
    /* Fall through */
L4:
    result = result * 3 + 1;
    
    /* Final volatile sync */
    g_volatile_counter = result;
    asm volatile ("mfence" ::: "memory");
    
    /* Return checksum to prevent dead code elimination */
    return (result ^ (int)fresult) & 0x7FFFFFFF;
}
