/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory", "eax", "ebx");
    return a * b + barrier;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float barrier = 0.0f;
    asm volatile ("" : "+x"(a), "+x"(b) : : "memory", "xmm0", "xmm1");
    return a * b + barrier;
}

/* Function with many independent instructions to populate ready list */
__attribute__((noinline)) static int parallel_ops(int a, int b, int c, int d, int e, int f) {
    int r1 = a + b;
    int r2 = c * d;
    int r3 = e & f;
    int r4 = a ^ c;
    int r5 = b | d;
    int r6 = e << 2;
    int r7 = f >> 1;
    int r8 = r1 + r2;
    int r9 = r3 - r4;
    int r10 = r5 * r6;
    
    /* Memory barrier to split scheduling region */
    asm volatile ("" : : : "memory");
    
    int r11 = r7 + r8;
    int r12 = r9 ^ r10;
    int r13 = __builtin_popcount(r11);
    int r14 = __builtin_ctz(r12 | 1);
    
    return r13 + r14;
}

/* Function with vector operations for target-specific scheduling */
__attribute__((noinline)) static __m128i vector_ops(__m128i a, __m128i b) {
    __m128i r1 = _mm_add_epi32(a, b);
    __m128i r2 = _mm_sub_epi32(a, b);
    __m128i r3 = _mm_mullo_epi16(a, b);
    __m128i r4 = _mm_slli_epi32(a, 3);
    __m128i r5 = _mm_srli_epi32(b, 2);
    
    /* Inline assembly with register constraints */
    asm volatile (
        "movdqa %1, %%xmm0\n\t"
        "paddd %%xmm0, %0\n\t"
        : "+x"(r1)
        : "x"(r2)
        : "xmm0"
    );
    
    return _mm_add_epi32(r3, _mm_add_epi32(r4, r5));
}

/* Complex control flow to create multiple basic blocks */
__attribute__((noinline)) static int complex_control_flow(int seed) {
    volatile int* volatile_ptr = &seed;
    int result = 0;
    
    /* Nested loops with data-dependent exit conditions */
    for (int i = 0; i < 10; i++) {
        int temp = *volatile_ptr;
        asm volatile ("" : "+r"(temp) : : "memory");
        
        for (int j = 0; j < (temp & 3) + 2; j++) {
            result += helper1(i, j);
            
            /* Mixed data types */
            float f = (float)(i * j) * 0.5f;
            result += (int)helper2(f, f + 1.0f);
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Update volatile pointer */
        *volatile_ptr = result;
    }
    
    return result;
}

/* Switch statement with multiple cases */
__attribute__((noinline)) static int switch_ops(int val) {
    int result = 0;
    
    switch (val & 7) {
        case 0:
            result = __builtin_bswap32(val);
            result += parallel_ops(val, val+1, val+2, val+3, val+4, val+5);
            /* Fall through */
        case 1:
            result ^= __builtin_popcount(val);
            result *= 0x9e3779b9; /* Mixing constant */
            break;
        case 2:
            result = val * 3;
            result += helper1(val, val >> 1);
            result -= helper2((float)val, (float)(val * 2));
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                result += (val << i) | (val >> (32 - i));
            }
            break;
        case 4:
            result = parallel_ops(val, ~val, val ^ 0x55555555, 
                                 val ^ 0xaaaaaaaa, val * 13, val * 17);
            /* Fall through */
        case 5:
            result = __builtin_ctz(val | 1) * 7;
            break;
        case 6:
            result = complex_control_flow(val);
            break;
        case 7:
            result = val * val * val;
            result += __builtin_ffs(val) * 11;
            break;
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    volatile int checksum = 0;
    int array_int[256];
    float array_float[256];
    volatile int* volatile_array = (volatile int*)array_int;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array_int[i] = i * 3;
        array_float[i] = i * 0.25f;
    }
    
    /* Region 1: Vector operations */
    __m128i vec_a = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_b = _mm_set_epi32(5, 6, 7, 8);
    __m128i vec_result = vector_ops(vec_a, vec_b);
    
    int vec_sum = 0;
    int* vec_ptr = (int*)&vec_result;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_ptr[i];
    }
    checksum += vec_sum;
    
    /* Region 2: Complex control flow with volatile accesses */
    for (int i = 0; i < 100; i++) {
        int idx = i & 255;
        volatile_array[idx] = volatile_array[idx] + 1;
        
        if (__builtin_expect((i & 31) == 0, 0)) {
            /* Unlikely path with many operations */
            checksum += complex_control_flow(i);
        } else {
            /* Likely path */
            checksum += switch_ops(i);
        }
        
        /* Memory barrier every 16 iterations */
        if ((i & 15) == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Region 3: Instruction-level parallelism */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    for (int i = 0; i < 50; i++) {
        /* Many independent operations */
        int r1 = a + b * c;
        int r2 = d ^ e | f;
        int r3 = __builtin_popcount(a);
        int r4 = __builtin_ctz(b | 1);
        float r5 = array_float[i & 255] * 2.0f;
        int r6 = (int)r5 + array_int[i & 255];
        
        /* Update variables with data dependencies */
        a = r1 + r2;
        b = r3 * r4;
        c = r6 ^ a;
        d = helper1(b, c);
        e = (int)helper2((float)d, (float)e);
        f = parallel_ops(a, b, c, d, e, f);
        
        checksum += a + b + c + d + e + f;
    }
    
    /* Region 4: Nested loops with mixed operations */
    for (int outer = 0; outer < 10; outer++) {
        int temp = checksum;
        
        for (int inner = 0; inner < 20; inner++) {
            /* Mixed integer/float operations */
            float fval = (float)temp * 0.1f;
            int ival = (int)(fval * 10.0f);
            
            /* SIMD-like operations using builtins */
            ival = __builtin_bswap32(ival);
            ival = (ival << 4) | (ival >> 28);
            
            /* Function call with side effects */
            ival = helper1(ival, inner);
            
            /* Volatile memory access */
            volatile_array[inner & 255] = ival;
            
            temp += ival + __builtin_ffs(inner | 1);
        }
        
        checksum = temp ^ 0x12345678;
        
        /* Inline assembly with explicit clobbers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "ror $7, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r"(checksum)
            : 
            : "eax", "memory"
        );
    }
    
    /* Region 5: Switch with computed goto-like behavior */
    int state = checksum & 3;
    for (int i = 0; i < 40; i++) {
        switch (state) {
            case 0:
                checksum += array_int[i & 255] * 3;
                state = 1;
                break;
            case 1:
                checksum -= (int)array_float[i & 255];
                state = 2;
                break;
            case 2:
                checksum ^= volatile_array[i & 255];
                state = 3;
                break;
            case 3:
                checksum = __builtin_bswap32(checksum);
                state = 0;
                break;
        }
        
        /* Every 8 iterations, force a complex operation */
        if ((i & 7) == 0) {
            checksum = switch_ops(checksum);
        }
    }
    
    /* Final mixing */
    checksum = __builtin_bswap32(checksum);
    checksum ^= __builtin_popcount(checksum);
    checksum = checksum * 0x9e3779b9;
    
    return checksum & 0x7fffffff; /* Ensure positive return */
}
