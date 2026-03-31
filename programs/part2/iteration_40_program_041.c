/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to create memory barriers */
volatile int volatile_counter = 0;
volatile int* volatile_ptr = &volatile_counter;

/* Arrays for data mixing */
int int_array[256] = {0};
float float_array[256] = {0.0f};
double double_array[256] = {0.0};

/* Noinline functions to prevent optimization */
__attribute__((noinline)) int noinline_add(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b;
}

__attribute__((noinline)) float noinline_mul(float a, float b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b;
}

__attribute__((noinline)) void noinline_side_effect(int* p) {
    *p = (*p * 1103515245 + 12345) & 0x7fffffff;
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed, int iterations) {
    int result = seed;
    float f_result = (float)seed;
    double d_result = (double)seed;
    
    /* Mixed data type operations to prevent instruction combining */
    for (int i = 0; i < iterations; i++) {
        /* Integer arithmetic chain */
        int a = result * 1664525 + 1013904223;
        int b = a ^ (a >> 16);
        int c = b * 1103515245 + 12345;
        
        /* Floating point operations */
        f_result = f_result * 1.6180339f + (float)c;
        d_result = d_result * 1.41421356 + (double)b;
        
        /* Volatile memory access - creates scheduling barrier */
        *volatile_ptr = *volatile_ptr + 1;
        
        /* Inline assembly with explicit clobbers */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "imull %%ecx, %%eax\n\t"
            : "=a"(result)
            : "a"(a), "b"(b), "c"(c)
            : "cc", "memory"
        );
        
        /* SIMD operations when available */
        #ifdef __SSE2__
        __m128i vec_a = _mm_set_epi32(i, a, b, c);
        __m128i vec_b = _mm_set_epi32(c, b, a, i);
        __m128i vec_c = _mm_add_epi32(vec_a, vec_b);
        __m128i vec_d = _mm_mullo_epi32(vec_c, vec_a);
        
        int simd_results[4];
        _mm_storeu_si128((__m128i*)simd_results, vec_d);
        result += simd_results[0] + simd_results[1];
        #endif
        
        /* Function call with side effects */
        noinline_side_effect(&result);
    }
    
    return result + (int)f_result + (int)d_result;
}

/* Another scheduling region with different patterns */
__attribute__((noinline, optimize("O2")))
int nested_control_flow(int base) {
    int acc = base;
    
    /* Nested loops with data-dependent exit conditions */
    for (int i = 0; i < 8; i++) {
        int inner_limit = (acc & 0xF) + 3;
        
        for (int j = 0; j < inner_limit; j++) {
            /* Mixed operations */
            acc = acc * 6364136223846793005ULL + 1442695040888963407ULL;
            
            /* Conditional break with probability */
            if ((acc & 0xFF) < 32) {
                /* Inline assembly with memory clobber */
                asm volatile ("" : : : "memory");
                break;
            }
            
            /* More arithmetic */
            acc ^= acc >> 21;
            acc ^= acc << 35;
            acc ^= acc >> 4;
            
            /* Volatile access */
            volatile_counter++;
        }
        
        /* Switch statement creating multiple basic blocks */
        switch (i & 0x3) {
            case 0:
                acc = noinline_add(acc, i * 100);
                /* Fall through */
            case 1:
                acc = acc * 3 + 1;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 2:
                acc = acc ^ (acc >> 1);
                /* Function call */
                noinline_side_effect(&acc);
                break;
            case 3:
                acc = (acc << 5) | (acc >> 27);
                /* Another volatile access */
                *volatile_ptr = acc;
                break;
        }
    }
    
    return acc;
}

/* Function with many independent instructions */
__attribute__((noinline))
int instruction_parallelism_test(int start) {
    int a = start + 1;
    int b = start * 2;
    int c = start & 0x55555555;
    int d = start | 0xAAAAAAAA;
    int e = start ^ 0x33333333;
    int f = start << 3;
    int g = start >> 2;
    int h = start % 17;
    
    /* Independent operations that can execute in parallel */
    a = a * 3 + 1;
    b = b ^ (b >> 1);
    c = c * 5 - 1;
    d = d + (d << 2);
    e = e | 0x0F0F0F0F;
    f = f & 0x00FF00FF;
    g = g * 7 + 3;
    h = h ^ 0x12345678;
    
    /* Memory barrier between instruction groups */
    asm volatile ("" : : : "memory");
    
    /* More independent operations */
    int i = a + b;
    int j = c - d;
    int k = e & f;
    int l = g | h;
    int m = i ^ j;
    int n = k + l;
    int o = m * n;
    int p = o >> 4;
    
    /* Architecture-specific builtins */
    int popcnt = __builtin_popcount(p);
    int ctz = __builtin_ctz(p | 1);
    int clz = __builtin_clz(p | 1);
    
    return popcnt + ctz * 2 + clz * 3 + a + b + c + d + e + f + g + h;
}

/* Main function with multiple scheduling regions */
int main() {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3 + 1;
        float_array[i] = (float)i * 1.5f;
        double_array[i] = (double)i * 2.5;
    }
    
    /* Region 1: Complex scheduling with SIMD */
    checksum ^= complex_scheduling_region(0x12345678, 16);
    
    /* Region 2: Nested control flow */
    checksum += nested_control_flow(checksum);
    
    /* Region 3: Instruction parallelism */
    checksum |= instruction_parallelism_test(checksum);
    
    /* Region 4: Another complex region with different optimization level */
    #pragma GCC optimize("O3")
    {
        int temp = checksum;
        for (int i = 0; i < 32; i++) {
            /* Mixed pointer arithmetic */
            int* ptr = int_array + (temp & 0xFF);
            float* fptr = float_array + (temp & 0xFF);
            double* dptr = double_array + (temp & 0xFF);
            
            *ptr = *ptr + temp;
            *fptr = noinline_mul(*fptr, (float)temp);
            *dptr = *dptr * 1.73205080757;
            
            /* More inline assembly with register constraints */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %%eax\n\t"
                "addl %2, %%eax\n\t"
                : "=a"(temp)
                : "r"(temp), "r"(i)
                : "cc"
            );
            
            /* Conditional goto to create interesting CFG */
            if (temp & 0x100) {
                goto compute_jump;
            }
            
            continue;
            
        compute_jump:
            temp = temp ^ 0xAAAAAAAA;
        }
        checksum += temp;
    }
    #pragma GCC reset_options
    
    /* Region 5: Loop with varying latency operations */
    for (int outer = 0; outer < 4; outer++) {
        int local_sum = 0;
        
        /* Unrolled inner loop */
        for (int inner = 0; inner < 8; inner++) {
            /* Different operations with varying "latency" */
            switch ((outer * 8 + inner) & 0x7) {
                case 0:
                    local_sum += int_array[inner] * 2;
                    /* Memory fence */
                    asm volatile ("" : : : "memory");
                    break;
                case 1:
                    local_sum -= __builtin_popcount(int_array[inner]);
                    break;
                case 2:
                    local_sum ^= (int_array[inner] << 1);
                    break;
                case 3:
                    local_sum |= int_array[inner] >> 2;
                    break;
                case 4:
                    local_sum = (local_sum * 3 + 1) & 0x7FFFFFFF;
                    break;
                case 5:
                    local_sum += __builtin_ctz(int_array[inner] | 1);
                    break;
                case 6:
                    local_sum = noinline_add(local_sum, int_array[inner]);
                    break;
                case 7:
                    local_sum = local_sum * 5 - 3;
                    /* Another volatile access */
                    volatile_counter = local_sum;
                    break;
            }
        }
        
        checksum = checksum * 31 + local_sum;
    }
    
    /* Final computation to prevent dead code elimination */
    checksum = checksum ^ (checksum >> 16);
    checksum = checksum * 0x5BD1E995;
    checksum = checksum ^ (checksum >> 15);
    
    return checksum & 0x7FFFFFFF;  /* Return positive value */
}
