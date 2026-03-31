/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    *p = *p * 2 + 1;
    return *p;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a / (b + 1.0f);
    return res;
}

/* Complex function with multiple scheduling regions */
int complex_scheduler_test(int seed) {
    volatile int* volatile_ptr = (volatile int*)&seed;
    int arr_int[256];
    float arr_float[256];
    __m128i vec_data[16];
    int checksum = 0;
    
    /* Initialize arrays with mixed patterns */
    for (int i = 0; i < 256; i++) {
        arr_int[i] = i * seed + (i ^ seed);
        arr_float[i] = (float)i / (seed + 1.0f);
        if (i < 16) {
            vec_data[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
        }
    }
    
    /* REGION 1: Mixed integer operations with volatile and inline asm */
    {
        int a = *volatile_ptr;
        int b = arr_int[a & 0xFF];
        int c = helper1(a, b);
        
        /* Inline assembly with clobbers to create scheduling barriers */
        asm volatile ("# Region 1 start\n\t"
                      "mov %0, %%eax\n\t"
                      "add %1, %%eax\n\t"
                      : : "r"(a), "r"(b) : "eax", "memory");
        
        int d = a + b * c;
        int e = (d << 3) | (d >> 29);
        int f = helper2(arr_int, e & 0xFF);
        
        /* Memory barrier */
        asm volatile ("# Barrier\n\t" : : : "memory");
        
        int g = f ^ c ^ d;
        checksum += g;
        
        /* Vector operations */
        __m128i v1 = vec_data[0];
        __m128i v2 = vec_data[1];
        __m128i v3 = _mm_add_epi32(v1, v2);
        vec_data[2] = v3;
        
        /* More arithmetic chains */
        for (int i = 0; i < 8; i++) {
            arr_int[i] = arr_int[i] * 3 + arr_int[i+1];
            arr_int[i+1] = arr_int[i+1] / 2 - arr_int[i];
        }
    }
    
    /* REGION 2: Nested loops with data-dependent exits */
    {
        int counter = checksum;
        for (int i = 0; i < 32; i++) {
            int inner = arr_int[i] & 0xF;
            while (inner > 0) {
                /* Mixed operations */
                float fval = helper3(arr_float[i], arr_float[i+1]);
                arr_float[i] = fval * 2.0f - 1.0f;
                
                /* More inline asm with register constraints */
                asm volatile ("# Inner loop\n\t"
                              "imull %0, %0\n\t"
                              : "+r"(inner) : : "cc");
                
                inner >>= 1;
                
                /* Conditional with many operations */
                if (inner & 1) {
                    arr_int[i] = helper1(arr_int[i], inner);
                    checksum += arr_int[i];
                } else {
                    arr_int[i] = helper2(arr_int, inner & 0xFF);
                    checksum -= arr_int[i];
                }
            }
            
            /* SIMD operations in loop */
            if (i < 12) {
                __m128i v = vec_data[i % 4];
                v = _mm_slli_epi32(v, 2);
                v = _mm_add_epi32(v, _mm_set1_epi32(i));
                vec_data[(i+1) % 4] = v;
            }
        }
    }
    
    /* REGION 3: Switch statement with multiple cases */
    {
        int selector = checksum & 0x7;
        
        switch (selector) {
            case 0: {
                /* Long chain of dependent operations */
                int x = arr_int[0];
                int y = helper1(x, arr_int[1]);
                int z = y * x - (x >> 2);
                int w = helper2(arr_int, z & 0xFF);
                checksum = checksum * 3 + w;
                
                /* Vector shuffle */
                __m128i v = _mm_set_epi32(1, 2, 3, 4);
                v = _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 3, 2, 1));
                vec_data[5] = v;
                break;
            }
            case 1: {
                /* Floating point intensive */
                float f1 = arr_float[0];
                float f2 = arr_float[1];
                for (int i = 0; i < 16; i++) {
                    f1 = helper3(f1, f2);
                    f2 = f1 * 1.5f - f2;
                    arr_float[i] = f1 + f2;
                }
                checksum += (int)f1;
                break;
            }
            case 2: {
                /* Memory intensive with volatile */
                volatile int* p = &arr_int[100];
                for (int i = 0; i < 20; i++) {
                    *p = *p + arr_int[i];
                    p = &arr_int[100 + ((i * 17) & 0x3F)];
                }
                checksum ^= *p;
                break;
            }
            case 3: {
                /* Many independent instructions */
                int a1 = arr_int[10] + 1;
                int a2 = arr_int[11] * 2;
                int a3 = arr_int[12] & 0xAAAA;
                int a4 = arr_int[13] | 0x5555;
                int a5 = arr_int[14] ^ arr_int[15];
                int a6 = helper1(a1, a2);
                int a7 = helper2(arr_int, a3 & 0xFF);
                checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7;
                break;
            }
            case 4: {
                /* Builtin operations */
                checksum += __builtin_popcount(arr_int[50]);
                checksum += __builtin_ctz(arr_int[51] | 1);
                checksum += __builtin_clz(arr_int[52] | 1);
                break;
            }
            case 5: {
                /* Complex asm with multiple clobbers */
                int val = arr_int[60];
                asm volatile ("# Complex asm\n\t"
                              "mov %1, %%eax\n\t"
                              "bsf %1, %%ecx\n\t"
                              "imul %%ecx, %%eax\n\t"
                              "mov %%eax, %0\n\t"
                              : "=r"(val) : "r"(val) : "eax", "ecx", "cc", "memory");
                checksum *= val;
                break;
            }
            default: {
                /* Mixed everything */
                __m128i v = vec_data[selector % 4];
                v = _mm_add_epi32(v, _mm_srli_epi32(v, 8));
                int* vi = (int*)&v;
                for (int i = 0; i < 4; i++) {
                    checksum += helper1(vi[i], arr_int[i]);
                }
                break;
            }
        }
    }
    
    /* REGION 4: Computed goto with scheduling regions */
    {
        static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
        int idx = checksum & 0x3;
        
        goto *labels[idx];
        
        L0: {
            /* Chain of dependent FP ops */
            float f = arr_float[0];
            for (int i = 1; i < 20; i++) {
                f = helper3(f, arr_float[i]);
                arr_float[i] = f * (i + 1);
            }
            checksum += (int)f;
            goto END_REGION;
        }
        
        L1: {
            /* Integer SIMD */
            __m128i vsum = _mm_setzero_si128();
            for (int i = 0; i < 8; i += 2) {
                __m128i v1 = vec_data[i];
                __m128i v2 = vec_data[i+1];
                vsum = _mm_add_epi32(vsum, _mm_mullo_epi32(v1, v2));
            }
            int* vs = (int*)&vsum;
            checksum += vs[0] + vs[1] + vs[2] + vs[3];
            goto END_REGION;
        }
        
        L2: {
            /* Volatile memory dance */
            volatile int* p1 = &arr_int[64];
            volatile int* p2 = &arr_int[128];
            for (int i = 0; i < 16; i++) {
                *p1 = *p1 + *p2;
                *p2 = *p2 - *p1;
                asm volatile ("# Memory op\n\t" : : : "memory");
                p1++;
                p2--;
            }
            checksum ^= *p1 ^ *p2;
            goto END_REGION;
        }
        
        L3: {
            /* Mixed type operations */
            int sum = 0;
            for (int i = 0; i < 32; i++) {
                sum += arr_int[i];
                arr_float[i] = arr_float[i] + (float)sum;
                if (i & 1) {
                    __m128i v = _mm_set1_epi32(sum);
                    vec_data[i/8] = _mm_add_epi32(vec_data[i/8], v);
                }
            }
            checksum = sum;
            goto END_REGION;
        }
        
        END_REGION:;
    }
    
    /* REGION 5: Final reduction with many parallel operations */
    {
        /* Independent chains */
        int chain1 = 0, chain2 = 0, chain3 = 0, chain4 = 0;
        
        for (int i = 0; i < 64; i += 4) {
            chain1 = chain1 * 3 + arr_int[i];
            chain2 = chain2 ^ helper1(arr_int[i+1], i);
            chain3 = chain3 + (int)arr_float[i+2];
            chain4 = chain4 - helper2(arr_int, arr_int[i+3] & 0xFF);
            
            /* Memory barrier every 8 iterations */
            if ((i & 0x7) == 0) {
                asm volatile ("# Parallel barrier\n\t" : : : "memory");
            }
        }
        
        /* Vector horizontal sum */
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 4; i++) {
            vsum = _mm_add_epi32(vsum, vec_data[i]);
        }
        vsum = _mm_hadd_epi32(vsum, vsum);
        vsum = _mm_hadd_epi32(vsum, vsum);
        int vec_sum;
        _mm_store_ss((float*)&vec_sum, _mm_castsi128_ps(vsum));
        
        checksum = checksum + chain1 + chain2 + chain3 + chain4 + vec_sum;
    }
    
    return checksum;
}

int main() {
    int total = 0;
    
    /* Test multiple seeds to explore different scheduling paths */
    for (int seed = 0; seed < 100; seed++) {
        total += complex_scheduler_test(seed);
    }
    
    return total & 0xFF; /* Prevent overflow, ensure return varies */
}
