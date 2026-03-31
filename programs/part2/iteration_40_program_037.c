/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -mtune=native */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int* mem = (volatile int*)&a;
    *mem = *mem + b;
    asm volatile ("" : : "r"(*mem) : "memory");
    return *mem;
}

__attribute__((noinline)) int helper2(int a, int b) {
    /* Mixed operations to prevent combining */
    int r1 = a + b;
    int r2 = a * b;
    int r3 = r1 ^ r2;
    int r4 = r3 << 2;
    int r5 = r4 >> 1;
    asm volatile ("nop; nop" : : : "eax", "memory");
    return r5;
}

__attribute__((noinline)) float helper3(float a, float b) {
    /* Floating point operations */
    float r1 = a + b;
    float r2 = a * b;
    float r3 = r1 / r2;
    asm volatile ("" : : : "xmm0", "xmm1", "memory");
    return r3;
}

/* Function with vector operations */
__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i r1 = _mm_add_epi32(a, b);
    __m128i r2 = _mm_mullo_epi32(a, b);
    __m128i r3 = _mm_slli_epi32(r1, 2);
    __m128i r4 = _mm_srli_epi32(r2, 1);
    __m128i r5 = _mm_xor_si128(r3, r4);
    
    /* Memory barrier to split scheduling regions */
    asm volatile ("" : : : "memory");
    
    return r5;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    int result = seed;
    volatile int* volatile_ptr = &result;
    
    /* Region 1: Integer operations with dependencies */
    int a = result + 1;
    int b = a * 2;
    int c = b >> 3;
    int d = c ^ 0xABCD;
    int e = d & 0xFF;
    
    /* Volatile access creates scheduling barrier */
    *volatile_ptr = *volatile_ptr + e;
    
    /* Region 2: Mixed operations */
    float f1 = (float)a * 1.5f;
    float f2 = (float)b / 2.0f;
    float f3 = helper3(f1, f2);
    result += (int)f3;
    
    /* Region 3: Vector operations */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, result, seed, 0x1234);
    __m128i v3 = vector_op(v1, v2);
    
    /* Extract results from vector */
    int varr[4];
    _mm_storeu_si128((__m128i*)varr, v3);
    result += varr[0] + varr[1] + varr[2] + varr[3];
    
    /* Region 4: Inline assembly with clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(result)
        : "eax", "memory"
    );
    
    return result;
}

/* Function with switch creating multiple basic blocks */
int switch_scheduler(int mode, int input) {
    int output = input;
    
    switch(mode) {
        case 0: {
            /* Long chain of dependent operations */
            int t1 = output + 1;
            int t2 = t1 * 2;
            int t3 = t2 - 3;
            int t4 = t3 >> 1;
            int t5 = t4 ^ 0xFF;
            int t6 = t5 & 0x7F;
            int t7 = t6 << 2;
            int t8 = t7 + 5;
            int t9 = t8 * 3;
            int t10 = t9 - 7;
            output = helper1(t10, output);
            break;
        }
        case 1: {
            /* Many independent instructions */
            int a1 = output + 1;
            int a2 = output * 2;
            int a3 = output ^ 0x1234;
            int a4 = output & 0xFF00;
            int a5 = output >> 4;
            int a6 = output << 3;
            
            /* Force them to be used */
            output = a1 + a2 + a3 + a4 + a5 + a6;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            /* More operations */
            output = helper2(output, 42);
            break;
        }
        case 2: {
            /* Nested loops with data-dependent exit */
            int sum = 0;
            for (int i = 0; i < 10; i++) {
                int inner = output;
                for (int j = 0; j < (i % 3) + 1; j++) {
                    inner = helper1(inner, j);
                    asm volatile ("nop" : : : "memory");
                }
                sum += inner;
            }
            output = sum;
            break;
        }
        case 3: {
            /* Mixed types and operations */
            float f = (float)output;
            for (int i = 0; i < 5; i++) {
                f = helper3(f, 1.1f + i);
                output += (int)f;
                
                /* Architecture-specific builtins */
                output += __builtin_popcount(output);
                output += __builtin_ctz(output | 1);
            }
            break;
        }
        default: {
            /* Complex region with gotos */
            int x = output;
            if (x > 100) goto label1;
            x = x * 2;
            goto label2;
        label1:
            x = x / 2;
        label2:
            output = x;
            break;
        }
    }
    
    return output;
}

/* Main function with multiple scheduling regions */
int main() {
    int checksum = 0;
    
    /* Array of different data types */
    int int_array[100];
    volatile int volatile_array[50];
    float float_array[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_array[i] = i * 3;
        if (i < 50) {
            volatile_array[i] = i * 2;
            float_array[i] = (float)i * 1.5f;
        }
    }
    
    /* Region 1: Complex scheduling with vector ops */
    checksum += complex_scheduling_region(42);
    
    /* Region 2: Switch with multiple cases */
    for (int mode = 0; mode < 5; mode++) {
        checksum += switch_scheduler(mode, checksum);
    }
    
    /* Region 3: Pointer arithmetic and memory ops */
    int* ptr = int_array;
    for (int i = 0; i < 100; i += 4) {
        /* Multiple independent memory accesses */
        int val1 = ptr[i];
        int val2 = ptr[i + 1];
        int val3 = ptr[i + 2];
        int val4 = ptr[i + 3];
        
        /* Operations on loaded values */
        val1 = val1 * 2;
        val2 = val2 + 3;
        val3 = val3 ^ 0xAA;
        val4 = val4 & 0x55;
        
        /* Store back with volatile */
        volatile_array[i % 50] = val1 + val2 + val3 + val4;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Region 4: Nested loops with function calls */
    for (int outer = 0; outer < 10; outer++) {
        int temp = checksum;
        for (int inner = 0; inner < (outer % 3) + 2; inner++) {
            temp = helper1(temp, inner);
            temp = helper2(temp, outer);
            
            /* Mix in floating point */
            float ftemp = (float)temp;
            ftemp = helper3(ftemp, 2.5f);
            temp += (int)ftemp;
        }
        checksum ^= temp;
    }
    
    /* Region 5: SIMD operations on arrays */
    for (int i = 0; i < 96; i += 4) {
        __m128i vec1 = _mm_loadu_si128((__m128i*)&int_array[i]);
        __m128i vec2 = _mm_set1_epi32(checksum);
        __m128i vec3 = vector_op(vec1, vec2);
        
        /* Extract and use results */
        int vec_result[4];
        _mm_storeu_si128((__m128i*)vec_result, vec3);
        checksum += vec_result[0] + vec_result[1] + 
                   vec_result[2] + vec_result[3];
    }
    
    /* Final mixing */
    checksum = __builtin_bswap32(checksum);
    checksum ^= __builtin_popcount(checksum);
    
    /* Prevent dead code elimination */
    volatile int final_result = checksum;
    
    return final_result % 256;
}
