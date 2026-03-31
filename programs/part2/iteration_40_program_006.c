/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-schedule-insns -fselective-scheduling2 -march=native */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline, target("no-sse"))) 
int helper1(int a, int b) {
    volatile int barrier = a + b;
    asm volatile ("" : : "r"(barrier) : "memory");
    return barrier * 2;
}

__attribute__((noinline, target("sse2")))
float helper2(float a, float b) {
    volatile float sum = a + b;
    asm volatile ("nop; nop" : : : "memory", "xmm0", "xmm1");
    return sum * 3.14f;
}

__attribute__((noinline))
void helper3(volatile int* p, int val) {
    *p = (*p & 0xFF) | (val << 8);
    asm volatile ("mfence" : : : "memory");
}

/* Complex SIMD operations for target-specific scheduling */
__attribute__((target("avx2")))
__m256i simd_chain(__m256i a, __m256i b, __m256i c) {
    __m256i t1 = _mm256_add_epi32(a, b);
    __m256i t2 = _mm256_mullo_epi32(t1, c);
    __m256i t3 = _mm256_slli_epi32(t2, 3);
    __m256i t4 = _mm256_srai_epi32(t3, 1);
    asm volatile ("" : : "x"(t1), "x"(t2), "x"(t3), "x"(t4) : "memory");
    return _mm256_xor_si256(t4, _mm256_set1_epi32(0x55555555));
}

/* Main function with multiple complex scheduling regions */
int main() {
    volatile int checksum = 0;
    int array_int[256] = {0};
    float array_float[256] = {0.0f};
    volatile int* volatile_ptr = &checksum;
    
    /* Region 1: Mixed integer operations with volatile and inline asm */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
        
        /* Chain of dependent operations */
        a = b + c;                    /* 1 */
        d = a * e;                    /* 2 */
        f = d >> 2;                   /* 3 */
        g = f & 0xFF;                 /* 4 */
        h = __builtin_popcount(g);    /* 5 - architecture specific */
        
        /* Volatile access with barrier */
        *volatile_ptr = *volatile_ptr + h;  /* 6 */
        
        /* Inline assembly with explicit clobbers */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "imull %%ecx, %%eax"
            : "=a"(a) : "a"(a), "b"(b), "c"(c) : "memory", "cc"  /* 7 */
        );
        
        /* More operations to fill instruction queue */
        b = a ^ 0x1234;               /* 8 */
        c = b * 3;                    /* 9 */
        d = c / 2;                    /* 10 */
        e = d | 0xFF00;               /* 11 */
        f = e << 3;                   /* 12 */
        g = f - 1;                    /* 13 */
        h = g % 17;                   /* 14 */
        
        checksum += h;                /* 15+ instructions in this block */
    }
    
    /* Region 2: SIMD operations triggering target-specific scheduling */
    #ifdef __AVX2__
    {
        __m256i v1 = _mm256_set_epi32(1,2,3,4,5,6,7,8);
        __m256i v2 = _mm256_set_epi32(8,7,6,5,4,3,2,1);
        __m256i v3 = _mm256_set_epi32(2,3,5,7,11,13,17,19);
        
        /* Complex SIMD chain */
        __m256i result = simd_chain(v1, v2, v3);
        
        /* Extract and accumulate */
        int* res_arr = (int*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += res_arr[i];
        }
    }
    #endif
    
    /* Region 3: Nested loops with data-dependent exits */
    {
        int i = 0, j = 0, k = 0;
        volatile int limit = 100;
        
        while (i < limit) {
            /* Inner loop with complex exit condition */
            for (j = 0; j < 50; j++) {
                k = (i * j) & 0xFF;
                
                /* Data-dependent break */
                if (__builtin_expect(k > 200, 0)) {
                    asm volatile ("" : : : "memory");
                    break;
                }
                
                /* Mixed operations */
                array_int[k] = array_int[k] + i - j;
                array_float[k] = array_float[k] * 1.01f + i;
                
                /* Function call creating scheduling barrier */
                helper3(&array_int[k], j);
                
                /* More operations */
                checksum += __builtin_ctz(k | 1);  /* Builtin with variable latency */
            }
            
            /* Switch statement with multiple cases */
            switch (i % 5) {
                case 0:
                    checksum += helper1(i, k);  /* 1 */
                    checksum *= 2;              /* 2 */
                    break;
                case 1:
                    checksum += helper2(i, k);  /* 3 */
                    checksum /= 3;              /* 4 */
                    /* Fall through */
                case 2:
                    checksum ^= 0xABCD;         /* 5 */
                    checksum |= 0xFF00;         /* 6 */
                    asm volatile ("nop" : : : "memory", "eax"); /* 7 */
                    break;
                case 3:
                    checksum = checksum << 1;   /* 8 */
                    checksum = checksum >> 1;   /* 9 */
                    /* Fall through */
                default:
                    checksum += array_int[i % 256];  /* 10 */
                    checksum -= 1;                   /* 11 */
                    goto common_label;               /* 12 */
            }
            
        common_label:
            /* Common code after switch */
            i += (checksum & 1) + 1;  /* Data-dependent increment */
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Region 4: Many independent instructions for parallel scheduling */
    {
        int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6, a7 = 7, a8 = 8;
        int b1, b2, b3, b4, b5, b6, b7, b8;
        
        /* Independent operations that can be scheduled in parallel */
        b1 = a1 + 1;          /* 1 */
        b2 = a2 * 2;          /* 2 */
        b3 = a3 & 0xFF;       /* 3 */
        b4 = a4 | 0xAA;       /* 4 */
        b5 = a5 ^ 0x55;       /* 5 */
        b6 = a6 << 1;         /* 6 */
        b7 = a7 >> 2;         /* 7 */
        b8 = a8 + a1;         /* 8 */
        
        /* Memory barrier splitting scheduling region */
        asm volatile ("" : : : "memory");
        
        /* More independent operations */
        a1 = b1 + b2;         /* 9 */
        a2 = b3 * b4;         /* 10 */
        a3 = b5 & b6;         /* 11 */
        a4 = b7 | b8;         /* 12 */
        a5 = b1 ^ b8;         /* 13 */
        a6 = b2 << 3;         /* 14 */
        a7 = b3 >> 1;         /* 15 */
        a8 = b4 + b5;         /* 16 */
        
        checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    }
    
    /* Region 5: Computed goto with complex basic blocks */
    {
        static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
        int idx = checksum & 3;
        
        goto *labels[idx];
        
    label0:
        {
            int x = checksum;
            x = x * 3 + 1;                    /* 1 */
            x = x / 2;                        /* 2 */
            x = x | 0x80000000;               /* 3 */
            x = x ^ 0x12345678;               /* 4 */
            asm volatile ("pause" : : : "memory"); /* 5 */
            checksum = x;
            goto end_region;
        }
        
    label1:
        {
            float f = checksum;
            f = f * 1.5f;                     /* 1 */
            f = f / 2.0f;                     /* 2 */
            f = f + 3.14159f;                 /* 3 */
            f = f - 1.0f;                     /* 4 */
            asm volatile ("" : : "x"(f) : "memory", "xmm0"); /* 5 */
            checksum = (int)f;
            goto end_region;
        }
        
    label2:
        {
            long long ll = checksum;
            ll = ll * 7LL;                    /* 1 */
            ll = ll >> 2;                     /* 2 */
            ll = ll & 0xFFFFFFFFLL;           /* 3 */
            ll = ll | 0x100000000LL;          /* 4 */
            asm volatile ("" : : "r"(ll) : "memory", "rax"); /* 5 */
            checksum = (int)ll;
            goto end_region;
        }
        
    label3:
        {
            /* Mixed pointer arithmetic */
            int* p = array_int;
            p += checksum % 128;              /* 1 */
            *p = *p + 1;                      /* 2 */
            p += 64;                          /* 3 */
            *p = *p - 1;                      /* 4 */
            asm volatile ("lfence" : : : "memory"); /* 5 */
            checksum = *p;
            goto end_region;
        }
        
    end_region:
        /* Final barrier */
        asm volatile ("" : : : "memory");
    }
    
    return checksum & 0xFF;
}
