/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Force no-inline to create scheduling barriers */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    int val = *p;
    *p = val + (idx * 7);
    asm volatile ("nop" : : : "memory", "eax");
    return val;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile ("" : : "r"(res) : "memory");
    return res;
}

/* Vector operations to trigger target-specific scheduling */
__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi32(t1, _mm_set1_epi32(7));
    __m128i t3 = _mm_slli_epi32(t2, 3);
    __m128i t4 = _mm_xor_si128(t3, _mm_set1_epi32(0xAAAAAAAA));
    asm volatile ("" : : "x"(t4) : "memory");
    return t4;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int* volatile_ptr = &seed;
    int result = 0;
    int i, j, k;
    
    /* Region 1: Mixed operations with volatile and inline asm */
    {
        int a = seed * 3;
        int b = helper1(a, seed);
        volatile int c = *volatile_ptr + b;
        int d = c << 2;
        
        /* Inline assembly with explicit clobbers */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "imull $0x7, %%eax\n\t"
            : "=a"(d)
            : "a"(d), "b"(a)
            : "memory", "cc"
        );
        
        int e = helper2(&seed, d & 0xF);
        result += d + e;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Region 2: SIMD operations */
    {
        __m128i v1 = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
        __m128i v2 = _mm_set_epi32(seed*2, seed*3, seed*4, seed*5);
        
        for (i = 0; i < 8; i++) {
            v1 = vector_op(v1, v2);
            v2 = _mm_add_epi32(v2, _mm_set1_epi32(i));
            
            /* Extract and use results to prevent optimization */
            int arr[4];
            _mm_storeu_si128((__m128i*)arr, v1);
            result += arr[0] + arr[1] + arr[2] + arr[3];
        }
    }
    
    /* Region 3: Nested loops with data-dependent exits */
    {
        int counter = seed;
        for (i = 0; i < 20; i++) {
            int inner_sum = 0;
            for (j = 0; j < 15; j++) {
                if (j > (counter & 0x7)) break;
                
                inner_sum += helper1(i, j);
                inner_sum ^= (i * j) << (j & 0x3);
                
                /* Volatile access in loop */
                *volatile_ptr = inner_sum;
                
                for (k = 0; k < 3; k++) {
                    inner_sum += (k * (*volatile_ptr)) >> 1;
                    asm volatile ("nop" : : : "memory");
                }
            }
            result += inner_sum;
            counter = helper2(&counter, i);
        }
    }
    
    /* Region 4: Switch with multiple cases creating complex basic blocks */
    {
        int switch_val = result & 0xF;
        switch (switch_val) {
            case 0: {
                float f1 = (float)result;
                float f2 = helper3(f1, f1 * 0.5f);
                int i1 = (int)f2;
                result += i1 * 3;
                result ^= __builtin_popcount(i1);
                /* Fall through */
            }
            case 1:
            case 2: {
                result = helper1(result, switch_val);
                result += __builtin_ctz(result | 1);
                asm volatile (
                    "rorl $5, %0\n\t"
                    : "+r"(result)
                    : 
                    : "cc"
                );
                break;
            }
            case 3:
            case 4:
            case 5: {
                __m128i v = _mm_set1_epi32(result);
                int arr[4];
                for (i = 0; i < 100; i++) {
                    v = _mm_add_epi32(v, _mm_set1_epi32(i));
                    if (i % 7 == 0) {
                        _mm_storeu_si128((__m128i*)arr, v);
                        result += arr[0];
                    }
                }
                break;
            }
            default: {
                /* Many independent instructions for parallel scheduling */
                int t1 = result * 3;
                int t2 = result / 2;
                int t3 = result ^ 0x12345678;
                int t4 = result + t1;
                int t5 = t2 - t3;
                int t6 = t4 * t5;
                int t7 = t6 >> 4;
                int t8 = t7 & 0xFF;
                int t9 = helper1(t8, t7);
                int t10 = __builtin_popcount(t9);
                
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "=r"(result)
                    : "r"(t10), "r"(t9)
                    : "eax", "memory"
                );
                
                *volatile_ptr = result;
                break;
            }
        }
    }
    
    /* Region 5: Instruction-level parallelism stress test */
    {
        int vars[16];
        for (i = 0; i < 16; i++) vars[i] = result + i;
        
        /* Many independent operations */
        int r0 = vars[0] + vars[1];
        int r1 = vars[2] * vars[3];
        int r2 = vars[4] ^ vars[5];
        int r3 = vars[6] >> (vars[7] & 0x3);
        int r4 = helper1(vars[8], vars[9]);
        int r5 = __builtin_ctz(vars[10] | 1);
        int r6 = vars[11] * 7;
        int r7 = vars[12] + helper2(vars, 13);
        float r8 = helper3((float)vars[14], (float)vars[15]);
        
        /* Memory barrier between independent sections */
        asm volatile ("" : : : "memory");
        
        /* Combine results with more operations */
        result += r0 * r1;
        result ^= r2 + r3;
        result += (int)r8;
        result = helper1(result, r4);
        result += r5 * r6;
        result ^= r7;
        
        /* Final volatile store */
        *volatile_ptr = result;
    }
    
    return result;
}

/* Another complex function to increase scheduling opportunities */
__attribute__((noinline)) int second_scheduler_test(int base) {
    int total = base;
    
    #pragma GCC optimize("O2")
    {
        /* Different optimization level can trigger different scheduling */
        for (int i = 0; i < 50; i++) {
            if (__builtin_expect(i % 13 == 0, 0)) {
                /* Unlikely path */
                total += complex_scheduling_region(i);
            } else {
                /* Likely path with different operations */
                total -= helper1(i, base);
                total ^= __builtin_popcount(total);
            }
            
            /* Inline assembly with memory clobber every 7 iterations */
            if (i % 7 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
    }
    
    return total;
}

int main() {
    int final_result = 0;
    
    /* Multiple calls with different seeds to exercise scheduler */
    for (int iteration = 0; iteration < 10; iteration++) {
        int seed = iteration * 12345;
        
        /* Profile-guided optimization hint */
        if (__builtin_expect(iteration < 8, 1)) {
            final_result += complex_scheduling_region(seed);
        } else {
            final_result += second_scheduler_test(seed);
        }
        
        /* Occasionally add memory barriers */
        if (iteration % 3 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    volatile int output = final_result;
    return output & 0x7FFFFFFF; /* Ensure positive return */
}
