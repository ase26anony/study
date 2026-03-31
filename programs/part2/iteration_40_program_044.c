/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Non-inlinable functions to create scheduling barriers */
__attribute__((noinline, cold)) 
int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline, cold)) 
float helper2(float a, float b) {
    volatile float tmp = a * b;
    asm volatile ("" : "+x" (tmp) : : "memory");
    return tmp + 1.0f;
}

__attribute__((noinline, cold))
void* helper3(void* ptr, int offset) {
    volatile char* p = (char*)ptr;
    asm volatile ("" : "+r" (p) : : "memory");
    return p + offset;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = seed;
    int a = barrier + 1;
    int b = barrier * 2;
    int c = barrier >> 3;
    
    /* Region 1: Integer operation chains with volatile accesses */
    volatile int* volatile_ptr = &barrier;
    for (int i = 0; i < 8; i++) {
        *volatile_ptr = *volatile_ptr + a;
        a = a * b + c;
        b = b ^ a;
        c = c + (b >> 2);
        asm volatile ("nop\n\tnop\n\tnop" : : : "memory", "eax", "ebx", "ecx");
    }
    
    /* Region 2: Mixed integer/float with SIMD */
    float f1 = (float)a / 3.14f;
    float f2 = (float)b * 2.718f;
    __m128i v1 = _mm_set_epi32(a, b, c, barrier);
    __m128i v2 = _mm_set_epi32(b, c, a, barrier);
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_mullo_epi32(v3, v1);
    
    int vresult[4];
    _mm_storeu_si128((__m128i*)vresult, v4);
    
    /* Region 3: Switch with multiple cases creating instruction queues */
    int switch_val = barrier & 0xF;
    int switch_result = 0;
    
    switch (switch_val) {
        case 0:
            switch_result = helper1(a, b) + __builtin_popcount(c);
            asm volatile ("cpuid" : : "a"(0) : "rbx", "rcx", "rdx", "memory");
            break;
        case 1:
            switch_result = helper1(b, c) * __builtin_ctz(a | 1);
            f1 = helper2(f1, f2);
            break;
        case 2:
            switch_result = helper1(c, a) ^ __builtin_clz(b);
            /* Inline assembly with register constraints */
            asm volatile ("movl %1, %%eax\n\t"
                         "imull %%eax, %%eax\n\t"
                         "movl %%eax, %0"
                         : "=r" (switch_result)
                         : "r" (switch_result)
                         : "%eax", "memory");
            break;
        case 3:
            switch_result = vresult[0] + vresult[1] - vresult[2];
            /* Memory barrier */
            asm volatile ("mfence" : : : "memory");
            break;
        default:
            switch_result = helper1(a, c) | helper1(b, barrier);
            for (int j = 0; j < 4; j++) {
                switch_result += vresult[j] * j;
            }
            break;
    }
    
    /* Region 4: Nested loops with data-dependent exits */
    int loop_result = 0;
    int outer = (barrier % 5) + 3;
    
    for (int i = 0; i < outer; i++) {
        int inner = (switch_result % 4) + 2;
        volatile int inner_counter = inner;
        
        while (inner_counter > 0) {
            loop_result += helper1(loop_result, inner_counter);
            loop_result ^= __builtin_ia32_crc32si(loop_result, inner_counter);
            
            /* Conditional break with arithmetic */
            if ((loop_result & 0x3) == 0) {
                inner_counter -= 2;
            } else {
                inner_counter--;
            }
            
            /* Memory access pattern */
            volatile_ptr = (volatile int*)helper3((void*)&barrier, loop_result & 0x3);
            *volatile_ptr += 1;
        }
        
        /* Vector operation in loop */
        __m128 vf1 = _mm_set_ps(f1, f2, (float)i, (float)loop_result);
        __m128 vf2 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 vf3 = _mm_mul_ps(vf1, vf2);
        
        float farr[4];
        _mm_storeu_ps(farr, vf3);
        f1 += farr[0];
        f2 += farr[1];
    }
    
    /* Region 5: Many independent instructions for ready list */
    int r1 = barrier * 3;
    int r2 = barrier / 7;
    int r3 = barrier ^ 0xABCD;
    int r4 = barrier + 0x1234;
    int r5 = barrier - 999;
    int r6 = barrier << 4;
    int r7 = barrier >> 2;
    int r8 = barrier | 0xF0F0;
    int r9 = barrier & 0x0F0F;
    int r10 = ~barrier;
    
    /* Force all to be used */
    int final = r1 + r2 - r3 * r4 / (r5 | 1) + (r6 ^ r7) & r8 | r9 ^ r10;
    final += switch_result * 2;
    final += loop_result * 3;
    final += (int)(f1 * 100.0f);
    final += (int)(f2 * 200.0f);
    
    /* Another scheduling barrier */
    asm volatile ("lfence" : : : "memory");
    
    return final;
}

/* Second complex function with different patterns */
int another_scheduling_region(int base) {
    volatile int counters[8] = {0};
    int sum = base;
    
    /* Create instruction-level parallelism */
    #pragma GCC unroll 4
    for (int i = 0; i < 8; i++) {
        counters[i] = helper1(sum, i);
        sum += __builtin_popcount(counters[i]);
        
        /* SIMD-style integer operations */
        __m128i va = _mm_set1_epi32(sum);
        __m128i vb = _mm_set1_epi32(i);
        __m128i vc = _mm_add_epi32(va, vb);
        __m128i vd = _mm_xor_si128(vc, va);
        
        int tmp[4];
        _mm_storeu_si128((__m128i*)tmp, vd);
        sum += tmp[0] + tmp[1] - tmp[2] + tmp[3];
        
        /* Memory access with varying addresses */
        volatile int* p = &counters[(i * 17) & 7];
        *p = *p + sum;
        
        /* Conditional with unlikely path */
        if (__builtin_expect((sum & 0xFF) == 0, 0)) {
            asm volatile ("pause" : : : "memory");
            sum = helper1(sum, sum);
        }
    }
    
    /* Mixed float/int operations */
    float fsum = (float)sum;
    for (int i = 0; i < 4; i++) {
        fsum = helper2(fsum, (float)counters[i]);
        fsum = fsum * 1.5f - 0.5f;
        
        /* Vector float operations */
        __m128 vf = _mm_set1_ps(fsum);
        __m128 vconst = _mm_set_ps(1.1f, 2.2f, 3.3f, 4.4f);
        __m128 vres = _mm_mul_ps(vf, vconst);
        
        float fres[4];
        _mm_storeu_ps(fres, vres);
        fsum += fres[i & 3];
    }
    
    return sum + (int)fsum;
}

int main() {
    int total = 0;
    
    /* Create multiple scheduling contexts */
    for (int iteration = 0; iteration < 100; iteration++) {
        int seed = iteration * 1234567;
        
        /* Alternate between different scheduling patterns */
        if (iteration & 1) {
            total ^= complex_scheduling_region(seed);
        } else {
            total += another_scheduling_region(seed);
        }
        
        /* Occasionally add a scheduling barrier */
        if ((iteration % 7) == 0) {
            asm volatile ("mfence\n\t"
                         "lfence\n\t"
                         "sfence" : : : "memory");
        }
        
        /* Force register spilling and reloading */
        volatile int memory_barrier = total;
        asm volatile ("" : "+r" (total) : "r" (memory_barrier) : "memory");
    }
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < 32; i++) {
        final_result = helper1(final_result, total);
        final_result = __builtin_ia32_crc32si(final_result, i);
        
        /* More vector operations */
        __m128i v1 = _mm_set1_epi32(final_result);
        __m128i v2 = _mm_set1_epi32(i);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, v1);
        
        int vtmp[4];
        _mm_storeu_si128((__m128i*)vtmp, v4);
        final_result += vtmp[0] ^ vtmp[1] ^ vtmp[2] ^ vtmp[3];
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    return output;
}
