/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to create memory barriers */
volatile int g_volatile_counter = 0;
volatile int* volatile g_volatile_ptr = 0;

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int local = a + b;
    asm volatile ("" : "+r" (local) : : "memory");
    return local * 2;
}

__attribute__((noinline)) int helper2(int a, int b) {
    volatile int local = a - b;
    asm volatile ("" : "+r" (local) : : "memory");
    return local / 2;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float local = a * b;
    asm volatile ("" : "+r" (local) : : "memory");
    return local + 1.0f;
}

__attribute__((noinline)) void helper4(__m128i* vec) {
    volatile __m128i local = _mm_add_epi32(*vec, _mm_set1_epi32(1));
    asm volatile ("" : "+x" (local) : : "memory");
    *vec = local;
}

/* Function with mixed operations to create complex scheduling regions */
int complex_scheduling_region(int seed) {
    int result = seed;
    int i, j, k;
    float f1, f2, f3;
    volatile int* volatile_ptr = &g_volatile_counter;
    
    /* Region 1: Integer operations with dependencies */
    int a = seed + 1;
    int b = a * 2;
    int c = b >> 3;
    int d = c & 0xFF;
    int e = d | 0x1000;
    int f = e ^ 0x5555;
    result += f;
    
    /* Volatile access creates scheduling barrier */
    *volatile_ptr = *volatile_ptr + 1;
    
    /* Region 2: Floating point mixed with integer */
    f1 = (float)seed * 1.5f;
    f2 = f1 + 3.14159f;
    f3 = f2 * f1;
    result += (int)f3;
    
    /* Inline assembly with clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (result)
        : "eax", "memory"
    );
    
    /* Region 3: SIMD operations (target-specific scheduling) */
    __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_slli_epi32(vec3, 2);
    
    /* Extract results from vector */
    int vec_results[4];
    _mm_storeu_si128((__m128i*)vec_results, vec4);
    result += vec_results[0] + vec_results[1] + vec_results[2] + vec_results[3];
    
    /* Call noinline helper */
    result = helper1(result, seed);
    
    return result;
}

/* Function with nested loops and switch for instruction queue creation */
int nested_control_flow(int seed) {
    int result = seed;
    int i, j;
    
    /* Outer loop with multiple iterations */
    for (i = 0; i < 10; i++) {
        /* Switch statement creates multiple basic blocks */
        switch (i % 4) {
            case 0: {
                /* Case with arithmetic chain */
                int a = result + i;
                int b = a * 2;
                int c = b - i;
                int d = c >> 1;
                result = d;
                
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                
                /* Vector operation */
                __m128i v1 = _mm_set1_epi32(result);
                __m128i v2 = _mm_set1_epi32(i);
                __m128i v3 = _mm_mullo_epi32(v1, v2);
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v3);
                result += temp[0];
                break;
            }
            case 1: {
                /* Different operations mix */
                float f = (float)result * 0.5f;
                int int_part = (int)f;
                result = int_part | 0x1;
                
                /* Call helper */
                result = helper2(result, i);
                
                /* Another memory barrier */
                volatile int barrier = result;
                (void)barrier;
                break;
            }
            case 2: {
                /* Nested loop */
                for (j = 0; j < 3; j++) {
                    result += (i * j) + (result % 17);
                    
                    /* Conditional break with data dependency */
                    if (result > 1000) {
                        result >>= 1;
                        break;
                    }
                }
                
                /* Builtin operations */
                result += __builtin_popcount(result);
                result += __builtin_ctz(result | 1);
                break;
            }
            case 3: {
                /* Mixed operations with pointer arithmetic */
                int array[4] = {1, 2, 3, 4};
                int* ptr = array;
                result += *(ptr + (i % 4));
                result += *(ptr + ((i + 1) % 4));
                
                /* SIMD load/store */
                __m128i v = _mm_loadu_si128((__m128i*)array);
                v = _mm_add_epi32(v, _mm_set1_epi32(result));
                _mm_storeu_si128((__m128i*)array, v);
                result += array[0];
                break;
            }
        }
        
        /* Loop-carried dependency */
        result = result * 1103515245 + 12345;
    }
    
    return result;
}

/* Function with computed goto for complex control flow */
int computed_goto_region(int seed) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = seed;
    int i;
    
    for (i = 0; i < 8; i++) {
        /* Computed goto */
        goto *labels[i % 4];
        
        label0: {
            /* Integer operations */
            int a = result + 1;
            int b = a * 3;
            int c = b - 5;
            result = c;
            
            /* Volatile access */
            g_volatile_counter++;
            
            /* Continue */
            continue;
        }
        
        label1: {
            /* Floating point */
            float f = (float)result * 1.1f;
            result = (int)f;
            
            /* Inline assembly with multiple clobbers */
            asm volatile (
                "movl %1, %%ecx\n\t"
                "imull $7, %%ecx, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r" (result)
                : "r" (result)
                : "eax", "ecx", "memory"
            );
            continue;
        }
        
        label2: {
            /* SIMD operations */
            __m128i v = _mm_set_epi32(result, i, result + i, result - i);
            v = _mm_srai_epi32(v, 2);
            int temp[4];
            _mm_storeu_si128((__m128i*)temp, v);
            result += temp[0] + temp[1];
            
            /* Call helper */
            result = helper1(result, i);
            continue;
        }
        
        label3: {
            /* Mixed operations */
            result = (result << 3) | (result >> 29); /* rotate */
            result ^= 0xAAAAAAAA;
            
            /* Memory barrier and function call */
            asm volatile ("" : : : "memory");
            result = helper2(result, seed);
            continue;
        }
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    int i;
    
    /* Initialize volatile pointer */
    g_volatile_ptr = &g_volatile_counter;
    
    /* Region 1: Complex scheduling with dependencies */
    final_result += complex_scheduling_region(42);
    
    /* Region 2: Nested control flow */
    final_result += nested_control_flow(final_result);
    
    /* Region 3: Computed goto pattern */
    final_result += computed_goto_region(final_result);
    
    /* Region 4: Additional mixed operations */
    for (i = 0; i < 5; i++) {
        /* Independent instructions for parallel scheduling */
        int a = final_result + i;
        int b = final_result * i;
        int c = final_result >> i;
        int d = final_result & 0xFF;
        
        /* Use all results */
        final_result = a + b + c + d;
        
        /* Memory barrier every few iterations */
        if (i % 2 == 0) {
            asm volatile ("" : : : "memory");
        }
        
        /* SIMD operations */
        __m128i v1 = _mm_set1_epi32(final_result);
        __m128i v2 = _mm_set1_epi32(i);
        __m128i v3 = _mm_add_epi32(v1, v2);
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, v3);
        final_result += temp[0];
        
        /* Call helper functions */
        final_result = helper1(final_result, i);
        float fval = helper3((float)final_result, 1.5f);
        final_result += (int)fval;
        
        /* Vector helper */
        __m128i vec = _mm_set1_epi32(final_result);
        helper4(&vec);
        final_result += ((int*)&vec)[0];
    }
    
    /* Region 5: Switch with many cases */
    for (i = 0; i < 10; i++) {
        switch (final_result % 8) {
            case 0: final_result = __builtin_bswap32(final_result); break;
            case 1: final_result = __builtin_popcount(final_result); break;
            case 2: final_result = final_result * 3 + 1; break;
            case 3: final_result ^= 0xDEADBEEF; break;
            case 4: final_result = (final_result << 1) | (final_result >> 31); break;
            case 5: final_result = helper2(final_result, i); break;
            case 6: {
                __m128i v = _mm_set1_epi32(final_result);
                v = _mm_mullo_epi32(v, _mm_set1_epi32(7));
                final_result = ((int*)&v)[0];
                break;
            }
            case 7: final_result = final_result / (i + 1); break;
        }
        
        /* Create scheduling pressure with many independent ops */
        int x1 = final_result + 1;
        int x2 = final_result * 2;
        int x3 = final_result & 0xFFFF;
        int x4 = final_result | 0x10000;
        int x5 = final_result ^ 0x5555;
        
        final_result = x1 + x2 + x3 + x4 + x5;
    }
    
    /* Final computation to prevent dead code elimination */
    final_result = final_result % 1000000;
    
    /* Use result to prevent optimization */
    volatile int output = final_result;
    
    return output;
}
