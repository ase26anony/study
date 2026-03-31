/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

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
    *p = val + (idx * 37);
    asm volatile ("mfence" : : : "memory");
    return val;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile ("" : "+x" (res) : : "memory");
    return res;
}

/* Vector operations to trigger target-specific scheduling */
__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi16(a, b);
    __m128i t3 = _mm_slli_epi32(t1, 3);
    __m128i t4 = _mm_xor_si128(t2, t3);
    
    /* Inline assembly with specific register constraints */
    asm volatile (
        "pshufd $0x1B, %1, %0\n\t"
        "paddd %0, %1\n\t"
        : "+x" (t4), "+x" (t3)
        :
        : "cc"
    );
    
    return _mm_add_epi32(t4, t3);
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = 0;
    int result = seed;
    int* volatile mem_ptr = &result;
    
    /* Region 1: Mixed operations with dependencies */
    int a = result + 123;
    int b = a * 456;
    volatile int c = b >> 3;
    int d = helper1(a, b);
    float e = helper3((float)a, (float)b);
    int f = (int)e + d;
    
    /* Memory barrier to split scheduling region */
    asm volatile ("" : : : "memory");
    
    /* Region 2: Vector operations */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i vres = vector_op(v1, v2);
    int vsum = _mm_extract_epi32(vres, 0) + 
               _mm_extract_epi32(vres, 1) +
               _mm_extract_epi32(vres, 2) +
               _mm_extract_epi32(vres, 3);
    
    /* Region 3: Complex control flow with many instructions */
    switch (vsum & 7) {
        case 0: {
            int t1 = f + vsum;
            int t2 = t1 * 3;
            int t3 = t2 / 2;
            int t4 = t3 ^ 0xABCD;
            int t5 = t4 << 1;
            int t6 = t5 | 0xFF;
            int t7 = t6 - t1;
            int t8 = t7 & 0xFFFF;
            int t9 = t8 * 17;
            int t10 = t9 % 256;
            result += t10;
            break;
        }
        case 1: {
            for (int i = 0; i < 8; i++) {
                int x = result + i;
                int y = x * (i + 1);
                int z = y ^ x;
                result ^= z;
                if (z > 1000) break;
            }
            break;
        }
        default: {
            int tmp = result;
            tmp = (tmp * 1103515245 + 12345) & 0x7FFFFFFF;
            tmp = __builtin_popcount(tmp);
            tmp = __builtin_ctz(tmp | 1);
            result = tmp;
            break;
        }
    }
    
    return result;
}

/* Main function with multiple complex regions */
int main() {
    int checksum = 0;
    volatile int* volatile ptrs[4];
    int arrays[4][16];
    
    /* Initialize arrays with volatile pointers */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            arrays[i][j] = (i * 16 + j) * 3;
        }
        ptrs[i] = &arrays[i][0];
    }
    
    /* Multiple independent scheduling regions */
    #pragma GCC optimize("O3")
    for (int region = 0; region < 5; region++) {
        int start_val = checksum + region * 100;
        
        /* Region A: Memory-intensive operations */
        for (int i = 0; i < 4; i++) {
            volatile int* p = ptrs[i];
            int val = *p;
            *p = val + helper2(arrays[i], i);
            
            /* Inline assembly with clobbers */
            asm volatile (
                "addl $1, %0\n\t"
                "rorl $3, %0\n\t"
                : "+r" (val)
                :
                : "cc"
            );
            
            checksum ^= val;
        }
        
        /* Region B: Complex arithmetic chains */
        int chain = start_val;
        chain = chain + (chain << 2);
        chain = chain * 3;
        chain = chain ^ (chain >> 4);
        chain = helper1(chain, chain + 1);
        chain = chain | 0xAA55;
        chain = __builtin_bswap32(chain);
        checksum += chain;
        
        /* Region C: Nested loops with data-dependent exits */
        int outer = 3;
        while (outer-- > 0) {
            int inner = 8;
            int acc = checksum;
            
            do {
                acc = acc * 2 + 1;
                acc = acc ^ (acc << 1);
                if (acc & 0x80000000) {
                    acc = __builtin_popcount(acc);
                    break;
                }
                inner--;
            } while (inner > 0 && (acc % 7) != 0);
            
            checksum = acc;
        }
        
        /* Region D: Call complex scheduling function */
        checksum ^= complex_scheduling_region(checksum);
        
        /* Memory barrier between regions */
        asm volatile ("mfence" : : : "memory");
    }
    
    /* Final mixing to prevent optimization */
    checksum = checksum ^ (checksum >> 16);
    checksum = checksum * 0x85EBCA6B;
    checksum = checksum ^ (checksum >> 13);
    
    return checksum & 0x7FFFFFFF; /* Ensure positive return */
}
