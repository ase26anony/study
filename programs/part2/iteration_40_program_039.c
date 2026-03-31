/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Noinline functions to prevent optimization and create scheduling barriers */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    return result;
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float result = a * b - a / b;
    return result;
}

__attribute__((noinline)) void helper3(volatile int* p, int val) {
    *p = (*p & 0xFFFF) | (val << 16);
}

/* Function with mixed operations to create complex scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = 0;
    int result = seed;
    
    /* Region 1: Integer operations with dependencies */
    int a = seed + 1;
    int b = a * 3;
    int c = b >> 2;
    int d = c ^ a;
    int e = d * b + c;
    
    /* Volatile memory access creates scheduling barrier */
    barrier = e;
    
    /* Region 2: SIMD/vector operations (x86 specific) */
    __m128i vec1 = _mm_set_epi32(a, b, c, d);
    __m128i vec2 = _mm_set_epi32(e, seed, a, b);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, vec1);
    
    /* Extract results from vector */
    int vec_results[4];
    _mm_storeu_si128((__m128i*)vec_results, vec4);
    
    /* Region 3: Inline assembly with clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (result)
        : "eax", "memory", "cc"
    );
    
    /* Region 4: Function calls in tight loop */
    for (int i = 0; i < 3; i++) {
        result = helper1(result, vec_results[i % 4]);
        barrier = helper2(result * 1.0f, i * 0.5f);
    }
    
    /* Region 5: Mixed operations with memory barriers */
    volatile int* volatile_ptr = &barrier;
    for (int i = 0; i < 2; i++) {
        *volatile_ptr = *volatile_ptr + 1;
        asm volatile("" : : : "memory");
        result ^= (*volatile_ptr << i);
    }
    
    return result;
}

/* Second complex region with different patterns */
int another_scheduling_region(int base) {
    int total = base;
    
    /* Switch statement with multiple cases */
    switch (base % 5) {
        case 0: {
            /* Many independent instructions */
            int t1 = base + 1;
            int t2 = base * 2;
            int t3 = base & 0xFF;
            int t4 = base | 0xAA;
            int t5 = base ^ t1;
            int t6 = t2 + t3;
            int t7 = t4 - t5;
            int t8 = t6 * t7;
            total = t8 >> 2;
            
            /* Architecture-specific builtins */
            total += __builtin_popcount(total);
            total += __builtin_ctz(total | 1);
            break;
        }
        case 1: {
            /* Nested loops with data-dependent exit */
            for (int i = 0; i < 10; i++) {
                int inner = i;
                while (inner > 0) {
                    total ^= inner;
                    inner -= (total & 3);
                }
            }
            break;
        }
        case 2: {
            /* More SIMD operations */
            __m128i v1 = _mm_set1_epi32(base);
            __m128i v2 = _mm_set_epi32(1, 2, 3, 4);
            __m128i v3 = _mm_slli_epi32(v1, 2);
            __m128i v4 = _mm_srai_epi32(v3, 1);
            
            int vres[4];
            _mm_storeu_si128((__m128i*)vres, _mm_xor_si128(v2, v4));
            total = vres[0] + vres[1] + vres[2] + vres[3];
            break;
        }
        case 3: {
            /* Complex pointer arithmetic */
            int array[8];
            for (int i = 0; i < 8; i++) {
                array[i] = base + i * i;
            }
            
            int* ptr = array;
            for (int i = 0; i < 7; i++) {
                total += *(ptr + i) - *(ptr + i + 1);
                ptr += (total & 1);
            }
            break;
        }
        case 4: {
            /* Mixed floating point and integer */
            float f1 = base * 0.5f;
            float f2 = base * 1.5f;
            for (int i = 0; i < 4; i++) {
                f1 = helper2(f1, f2);
                total += (int)f1;
                f2 = helper2(f2, f1);
            }
            break;
        }
    }
    
    return total;
}

/* Function with computed goto to create irregular control flow */
int computed_goto_region(int val) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int result = val;
    int index = val % 5;
    
    goto *labels[index];
    
label0: {
    /* Block with many parallelizable instructions */
    int a1 = result + 1;
    int a2 = result * 2;
    int a3 = result & 0xFF;
    int a4 = result | 0xAA;
    int a5 = result ^ 0x55;
    int a6 = a1 + a2;
    int a7 = a3 - a4;
    int a8 = a5 * a6;
    int a9 = a7 >> 3;
    result = a8 + a9;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* More operations */
    result = helper1(result, a1);
    result ^= helper1(a2, a3);
    goto end;
}

label1: {
    /* Vector operations with horizontal reduction */
    __m128i v = _mm_set_epi32(result, result+1, result+2, result+3);
    v = _mm_madd_epi16(v, _mm_set1_epi16(1));
    
    int varr[4];
    _mm_storeu_si128((__m128i*)varr, v);
    result = varr[0] + varr[1] + varr[2] + varr[3];
    goto end;
}

label2: {
    /* Loop with volatile access */
    volatile int counter = 0;
    for (int i = 0; i < 8; i++) {
        counter = counter + result;
        result ^= counter;
        helper3(&counter, i);
    }
    goto end;
}

label3: {
    /* Mixed operations */
    float f = result * 0.25f;
    for (int i = 0; i < 4; i++) {
        f = helper2(f, f * 2.0f);
        result += (int)f;
        asm volatile("" : : : "memory");
    }
    goto end;
}

label4: {
    /* Complex integer chain */
    int x = result;
    for (int i = 0; i < 6; i++) {
        x = ((x << 3) | (x >> 29)) ^ 0xDEADBEEF;
        x = helper1(x, i);
        x = x * 1103515245 + 12345;
    }
    result = x;
    goto end;
}

end:
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    volatile int memory_barrier = 0;
    
    /* Initialize with some data */
    int data[16];
    for (int i = 0; i < 16; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Execute multiple complex scheduling regions */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Region A */
        int r1 = complex_scheduling_region(iteration + data[iteration % 16]);
        
        /* Memory barrier between regions */
        memory_barrier = r1;
        asm volatile("" : : : "memory");
        
        /* Region B */
        int r2 = another_scheduling_region(r1 ^ iteration);
        
        /* Region C with computed goto */
        int r3 = computed_goto_region(r2 + iteration);
        
        /* Combine results */
        final_result ^= r1 + r2 * 3 - r3;
        
        /* Occasionally call helper functions directly */
        if (iteration % 7 == 0) {
            final_result = helper1(final_result, iteration);
        }
        
        /* More memory barriers */
        asm volatile("" : : : "memory");
        memory_barrier = final_result;
    }
    
    /* Final complex region */
    for (int i = 0; i < 5; i++) {
        /* SIMD reduction */
        __m128i vsum = _mm_setzero_si128();
        for (int j = 0; j < 4; j++) {
            __m128i v = _mm_set_epi32(
                final_result + j,
                final_result - j,
                final_result * j,
                final_result ^ j
            );
            vsum = _mm_add_epi32(vsum, v);
        }
        
        int vres[4];
        _mm_storeu_si128((__m128i*)vres, vsum);
        final_result = vres[0] + vres[1] + vres[2] + vres[3];
        
        /* Inline assembly with register constraints */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "leal (%%ecx, %%ecx, 2), %%eax\n\t"
            "addl $0x4567, %%eax\n\t"
            "rorl $5, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (final_result)
            : "r" (final_result)
            : "eax", "ecx", "memory", "cc"
        );
    }
    
    /* Return final result to prevent dead code elimination */
    return final_result & 0x7FFFFFFF;  /* Ensure positive result */
}
