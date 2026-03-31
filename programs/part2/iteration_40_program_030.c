/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to prevent optimization */
volatile int vol_int = 42;
volatile float vol_float = 3.14f;
volatile int* volatile vol_ptr = &vol_int;

/* Noinline functions to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + (a ^ b);
}

__attribute__((noinline)) float helper2(float x, float y) {
    asm volatile ("" : : "x"(x), "x"(y) : "memory");
    return x * y - x / y;
}

__attribute__((noinline)) void helper3(volatile int* p) {
    *p = (*p << 3) | (*p >> 5);
    asm volatile ("mfence" ::: "memory");
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed) {
    int result = seed;
    float fresult = seed * 1.5f;
    __m128i vec_result = _mm_set1_epi32(seed);
    
    /* Region 1: Mixed integer operations with volatile accesses */
    for (int i = 0; i < 8; i++) {
        /* Create instruction chains */
        int a = result + vol_int;
        int b = a * (i + 1);
        int c = b ^ result;
        int d = c >> (i & 3);
        
        /* Volatile memory access creates scheduling barrier */
        vol_int = d;
        result = vol_int + c;
        
        /* Inline assembly with clobbers */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "rorl $3, %%eax"
            : "=a"(result)
            : "a"(result), "b"(d)
            : "cc"
        );
    }
    
    /* Region 2: SIMD/vector operations */
    __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
    
    for (int i = 0; i < 4; i++) {
        vec_result = _mm_add_epi32(vec_result, vec1);
        vec_result = _mm_mullo_epi32(vec_result, vec2);
        vec_result = _mm_slli_epi32(vec_result, 2);
        
        /* Memory barrier */
        asm volatile ("" ::: "memory");
        
        /* Function call in loop */
        result += helper1(result, i);
    }
    
    /* Extract SIMD result */
    int vec_arr[4];
    _mm_storeu_si128((__m128i*)vec_arr, vec_result);
    result += vec_arr[0] + vec_arr[1] + vec_arr[2] + vec_arr[3];
    
    /* Region 3: Switch with multiple cases creating complex CFG */
    switch (result & 7) {
        case 0: {
            /* Case with many independent instructions */
            int t1 = result * 3;
            int t2 = result / 2;
            int t3 = result ^ 0xAAAA;
            int t4 = t1 + t2;
            int t5 = t3 - t4;
            result = t5 & 0xFFFF;
            
            /* Volatile float operation */
            fresult = vol_float * 2.0f;
            result += (int)fresult;
            break;
        }
        case 1: {
            /* Another instruction-dense case */
            for (int j = 0; j < 3; j++) {
                result = (result << j) | (result >> (32 - j));
                result ^= j * 0x12345678;
                
                /* Call to volatile helper */
                helper3(&vol_int);
                result += vol_int;
            }
            break;
        }
        case 2:
        case 3: {
            /* Shared case with goto to create interesting flow */
            int tmp = result;
            result = 0;
            
            computed_jump:
            for (int k = 0; k < 4; k++) {
                /* Mixed operations */
                float ftmp = helper2(fresult, k + 1.0f);
                result += (int)ftmp;
                
                /* Builtin operations with varying latency */
                result += __builtin_popcount(tmp);
                result += __builtin_ctz(tmp | 1);
                
                /* More inline asm with register constraints */
                asm volatile (
                    "imull %%ecx, %%eax\n\t"
                    "addl $0x1234, %%eax"
                    : "+a"(result)
                    : "c"(k)
                    : "cc"
                );
            }
            
            if (result & 1) {
                tmp = result;
                goto computed_jump;
            }
            break;
        }
        default: {
            /* Default case with nested loops */
            int outer = 2;
            while (outer--) {
                int inner = 3;
                do {
                    /* Data-dependent exit condition */
                    if (inner == result % 3) break;
                    
                    /* Pointer arithmetic */
                    int* ptr = &result;
                    for (int m = 0; m < 2; m++) {
                        *(ptr + m) = *(ptr + m) + m * 17;
                    }
                    
                    /* Memory clobber */
                    asm volatile ("" ::: "memory");
                    
                    inner--;
                } while (inner > 0);
                
                result = result * 1103515245 + 12345;
            }
            break;
        }
    }
    
    /* Region 4: Many independent instructions for ready list */
    int r1 = result + 1;
    int r2 = result * 2;
    int r3 = result & 0xFF00FF;
    int r4 = result | 0x00FF00;
    int r5 = result ^ r1;
    int r6 = r2 + r3;
    int r7 = r4 - r5;
    int r8 = r6 * r7;
    int r9 = r8 >> 4;
    int r10 = r9 << 2;
    int r11 = r10 % 97;
    int r12 = ~r11;
    int r13 = r12 + vol_int;
    int r14 = r13 * 3;
    int r15 = r14 / 2;
    
    /* Use all results to prevent elimination */
    result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
    /* Final memory barrier */
    asm volatile ("sfence" ::: "memory");
    
    return result;
}

/* Another scheduling region with different characteristics */
__attribute__((noinline, optimize("O2")))
int second_scheduling_region(int base) {
    int sum = base;
    volatile int local_vol = base;
    
    /* Create instruction-level parallelism */
    for (int i = 0; i < 6; i++) {
        /* Independent chains */
        int chain1 = sum + i;
        int chain2 = sum * i;
        int chain3 = sum ^ i;
        int chain4 = sum - i;
        
        /* Force serialization occasionally */
        if (i & 1) {
            asm volatile ("lfence" ::: "memory");
        }
        
        /* Use all chains */
        chain1 = chain1 << (i & 3);
        chain2 = chain2 >> (i & 3);
        chain3 = chain3 * chain4;
        chain4 = chain4 ^ chain1;
        
        sum = chain1 + chain2 + chain3 + chain4;
        
        /* Volatile update */
        local_vol = sum;
        sum += local_vol;
        
        /* Builtin with data dependency */
        sum += __builtin_ffs(sum | 1) - 1;
    }
    
    return sum;
}

/* Main function with multiple scheduling contexts */
int main() {
    int final_result = 0;
    
    /* Initialize arrays for various operations */
    int int_array[64];
    float float_array[64];
    volatile int* ptr_array[16];
    
    for (int i = 0; i < 64; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 0.5f;
        if (i < 16) {
            ptr_array[i] = &int_array[i * 4];
        }
    }
    
    /* Execute multiple complex scheduling regions */
    for (int iter = 0; iter < 3; iter++) {
        /* Region A */
        final_result ^= complex_scheduling_region(final_result + iter);
        
        /* Memory barrier between regions */
        asm volatile ("" ::: "memory");
        
        /* Region B */
        final_result += second_scheduling_region(final_result);
        
        /* Access volatile through pointer array */
        for (int p = 0; p < 8; p++) {
            *ptr_array[p] = *ptr_array[p] + final_result;
            final_result += *ptr_array[p];
        }
        
        /* SIMD operations on arrays */
        for (int i = 0; i < 64; i += 4) {
            __m128i v1 = _mm_loadu_si128((__m128i*)&int_array[i]);
            __m128i v2 = _mm_set1_epi32(final_result & 0xFF);
            __m128i v3 = _mm_add_epi32(v1, v2);
            _mm_storeu_si128((__m128i*)&int_array[i], v3);
            
            /* Mix with floating point */
            float_array[i] = float_array[i] * 1.1f + final_result * 0.01f;
        }
        
        /* Profile-guided optimization hint */
        if (__builtin_expect((final_result & 0xFF) < 128, 1)) {
            /* Likely path with more operations */
            for (int i = 0; i < 32; i++) {
                int_array[i] = (int_array[i] * 3) / 2;
                int_array[i] ^= final_result;
                int_array[i] = __builtin_bswap32(int_array[i]);
            }
        } else {
            /* Unlikely path still has scheduling needs */
            final_result = ~final_result;
        }
    }
    
    /* Final aggregation */
    for (int i = 0; i < 64; i++) {
        final_result += int_array[i];
        final_result += (int)float_array[i];
    }
    
    /* Use volatile one more time */
    vol_int = final_result;
    final_result += vol_int;
    
    return final_result & 0x7FFFFFFF; /* Ensure positive result */
}
