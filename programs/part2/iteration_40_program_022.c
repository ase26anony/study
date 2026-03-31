/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    int val = *p;
    *p = val + (idx * 7);
    asm volatile ("nop; nop" : : : "eax", "memory");
    return val;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile ("" : "+x" (res) : : "memory");
    return res;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = seed;
    int result = 0;
    
    /* Region 1: Mixed integer operations with dependencies */
    int a = barrier + 1;
    int b = a * 3;
    int c = b >> 2;
    int d = c ^ a;
    int e = d * 7 + 1;
    
    /* Volatile access creates scheduling barrier */
    volatile int* vp = &barrier;
    *vp = *vp + e;
    
    /* Inline assembly with clobbers */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "rorl $3, %%eax"
        : "=a" (a)
        : "a" (a), "b" (b)
        : "cc"
    );
    
    /* Function call */
    result += helper1(a, b);
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(a, b, c, d);
    __m128i vec2 = _mm_set_epi32(e, seed, barrier, result);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, _mm_set1_epi32(7));
    
    int simd_results[4];
    _mm_storeu_si128((__m128i*)simd_results, vec4);
    
    /* More arithmetic chains */
    for (int i = 0; i < 4; i++) {
        simd_results[i] = (simd_results[i] * 3) >> 1;
        simd_results[i] ^= (simd_results[i] << 3);
    }
    
    /* Another volatile barrier */
    asm volatile ("" : : : "memory");
    
    /* Builtins with potential specialized scheduling */
    result += __builtin_popcount(simd_results[0]);
    result += __builtin_ctz(simd_results[1] | 1);
    
    return result;
}

/* Function with nested control flow creating complex basic blocks */
int nested_control_flow(int* data, int size) {
    int sum = 0;
    volatile int check = 0;
    
    /* Outer loop */
    for (int i = 0; i < size; i++) {
        /* Switch with multiple cases */
        switch (data[i] & 7) {
            case 0: {
                /* Case with many independent instructions */
                int t1 = data[i] * 2;
                int t2 = data[i] + 5;
                int t3 = t1 ^ t2;
                int t4 = t3 << 1;
                int t5 = t4 - t2;
                int t6 = t5 * 3;
                sum += t6;
                
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                
                /* Function call */
                sum += helper2(data, i);
                break;
            }
            case 1: {
                /* Different mix of operations */
                float f1 = data[i] * 0.5f;
                float f2 = data[i] * 1.5f;
                float f3 = helper3(f1, f2);
                sum += (int)(f3 * 100);
                
                /* More integer ops */
                int x = data[i];
                x = (x * 13) ^ (x >> 3);
                x = __builtin_bswap32(x);
                sum += x;
                break;
            }
            case 2: {
                /* SIMD in switch case */
                __m128i v = _mm_set1_epi32(data[i]);
                v = _mm_slli_epi32(v, 2);
                v = _mm_xor_si128(v, _mm_set1_epi32(0x55AA55AA));
                int r[4];
                _mm_storeu_si128((__m128i*)r, v);
                sum += r[0] + r[1];
                break;
            }
            default: {
                /* Default case with loop */
                int val = data[i];
                for (int j = 0; j < 3; j++) {
                    val = (val * 11 + j) & 0xFFF;
                    if (val & 1) {
                        val ^= 0xABCD;
                    } else {
                        val += 0x1234;
                    }
                }
                sum += val;
                break;
            }
        }
        
        /* Conditional break based on computation */
        if (sum > 1000000) {
            /* Force early exit with more computation */
            sum = sum >> 2;
            check = sum;
            if (check & 0x100) {
                break;
            }
        }
    }
    
    return sum;
}

/* Another complex region with gotos and labels */
int computed_jumps(int base) {
    int r = base;
    
    /* Use goto to create interesting control flow */
    if (r & 1) goto label1;
    if (r & 2) goto label2;
    if (r & 4) goto label3;
    
    /* Default path */
    r = r * 3 + 1;
    asm volatile ("nop" : : : "memory");
    goto end;
    
label1:
    r = (r << 4) | 0xF;
    /* Multiple independent instructions */
    int t1 = r + 1;
    int t2 = r - 1;
    int t3 = t1 * t2;
    int t4 = t3 ^ r;
    r = t4;
    goto end;
    
label2:
    r = __builtin_popcount(r) * 7;
    /* Memory operation chain */
    volatile int mem = r;
    for (int i = 0; i < 4; i++) {
        mem = mem * 2 + i;
    }
    r = mem;
    goto end;
    
label3:
    /* Mixed float/int operations */
    float f = (float)r;
    f = f * 1.618034f;
    f = helper3(f, f * 0.5f);
    r = (int)f;
    
    /* More integer chain */
    r = (r * 13) >> 2;
    r = r ^ (r << 16);
    r = r + 0xDEADBEEF;
    
end:
    /* Final computation */
    r = (r & 0xFFFF) | ((r & 0xFFFF0000) >> 16);
    return r;
}

/* Main function with multiple scheduling regions */
int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    volatile int* volatile_data = (volatile int*)malloc(SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 1;
        volatile_data[i] = i * 5 + 2;
    }
    
    int result = 0;
    
    /* Region 1: Complex scheduling region */
    result += complex_scheduling_region(data[0]);
    
    /* Region 2: Nested control flow */
    result += nested_control_flow(data, SIZE / 4);
    
    /* Region 3: Computed jumps */
    for (int i = 0; i < 16; i++) {
        result += computed_jumps(data[i]);
    }
    
    /* Region 4: More mixed operations */
    for (int i = 0; i < SIZE; i += 8) {
        /* SIMD load and process */
        __m128i v1 = _mm_loadu_si128((__m128i*)&data[i]);
        __m128i v2 = _mm_loadu_si128((__m128i*)&data[i + 4]);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, _mm_set1_epi32(3));
        
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, v4);
        
        /* Process results */
        for (int j = 0; j < 4; j++) {
            temp[j] = helper1(temp[j], i + j);
            result += temp[j];
        }
        
        /* Volatile access */
        volatile_data[i] = result & 0xFF;
    }
    
    /* Region 5: Final complex block with many instructions */
    {
        int a = result;
        int b = a * 2;
        int c = b + 0x1234;
        int d = c ^ a;
        int e = d << 3;
        int f = e | 0xF0F0;
        int g = f - b;
        int h = g * 7;
        int i = h >> 4;
        int j = i & 0xFFFF;
        int k = j + __builtin_popcount(j);
        int l = k * 13;
        int m = l ^ (l >> 8);
        int n = m + helper2(data, m & 0xF);
        int o = n * 3;
        int p = o - result;
        
        /* Multiple asm barriers */
        asm volatile ("" : : : "memory");
        asm volatile ("nop; nop; nop" : : : "eax", "memory");
        
        result = p;
    }
    
    /* Cleanup */
    free(data);
    free((void*)volatile_data);
    
    /* Return checksum to prevent optimization */
    return result & 0x7FFFFFFF;
}
