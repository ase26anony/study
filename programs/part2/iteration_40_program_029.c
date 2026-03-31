/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -mtune=native */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no-inline to create scheduling boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a + b;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int a, int b) {
    volatile int result = a * b;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper3(int a, int b) {
    volatile int result = a ^ b;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed) {
    volatile int barrier = seed;
    int a = barrier + 1;
    int b = barrier * 2;
    int c = barrier >> 3;
    
    /* Mixed integer operations with dependencies */
    int d = a + b;
    int e = d * c;
    int f = e >> 2;
    int g = f & 0xFF;
    
    /* Volatile memory access creating scheduling barrier */
    volatile int* p = &barrier;
    *p = *p + g;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax"
        : "=a"(d)
        : "a"(d), "b"(e), "c"(f)
        : "cc"
    );
    
    return d;
}

/* Function with SIMD operations */
__attribute__((noinline, target("sse2")))
int simd_scheduling_region(int seed) {
    __m128i v1 = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
    __m128i v2 = _mm_set_epi32(seed+4, seed+5, seed+6, seed+7);
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_mullo_epi32(v3, v1);
    
    /* Extract results with memory barrier */
    volatile int results[4];
    _mm_storeu_si128((__m128i*)results, v4);
    
    asm volatile ("" : : "r"(results) : "memory");
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Main function with multiple complex scheduling regions */
int main() {
    volatile int checksum = 0;
    int array1[256];
    float array2[256];
    volatile int* volatile_ptr = &checksum;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 0.5f;
    }
    
    /* Region 1: Complex integer operations with loops */
    for (int i = 0; i < 100; i++) {
        int a = array1[i];
        int b = array1[i+1];
        int c = array1[i+2];
        
        /* Chain of dependent operations */
        int d = a + b;
        int e = d * c;
        int f = e >> (i & 3);
        int g = f ^ d;
        int h = g & e;
        
        /* Volatile access */
        *volatile_ptr = *volatile_ptr + h;
        
        /* Function call creating scheduling boundary */
        if (i % 3 == 0) {
            h = helper1(h, i);
        } else if (i % 3 == 1) {
            h = helper2(h, i);
        } else {
            h = helper3(h, i);
        }
        
        checksum += h;
    }
    
    /* Region 2: Nested loops with conditional breaks */
    int outer = 0;
    while (outer < 50) {
        int inner = 0;
        int temp = checksum;
        
        do {
            /* Mixed operations to prevent combining */
            temp = temp + (inner * 2);
            temp = temp ^ (inner << 3);
            temp = temp * 1103515245 + 12345;
            
            /* Memory barrier */
            asm volatile ("" : : "r"(temp) : "memory");
            
            /* Conditional break with data dependency */
            if (temp % 1000 > 900) {
                checksum += temp;
                break;
            }
            
            inner++;
        } while (inner < 20);
        
        outer++;
    }
    
    /* Region 3: Switch statement with multiple cases */
    int switch_var = checksum & 7;
    int switch_result = 0;
    
    switch (switch_var) {
        case 0: {
            /* Vector operations in switch case */
            switch_result = simd_scheduling_region(checksum);
            break;
        }
        case 1: {
            int x = checksum;
            x = x + (x << 2);
            x = x ^ (x >> 3);
            x = x * 1664525 + 1013904223;
            switch_result = x;
            break;
        }
        case 2: {
            /* Complex region with inline assembly */
            switch_result = complex_scheduling_region(checksum);
            break;
        }
        case 3: {
            /* Mixed float/int operations */
            float f1 = array2[checksum & 255];
            float f2 = f1 * 3.14159f;
            int i1 = (int)f2;
            int i2 = i1 * 2;
            switch_result = i2;
            break;
        }
        case 4: {
            /* Builtin operations */
            switch_result = __builtin_popcount(checksum) + 
                           __builtin_ctz(checksum | 1);
            break;
        }
        default: {
            /* Multiple independent instructions */
            int a = checksum + 1;
            int b = checksum * 2;
            int c = checksum & 0xFF00;
            int d = checksum >> 8;
            switch_result = a + b + c + d;
            break;
        }
    }
    
    checksum += switch_result;
    
    /* Region 4: Computed goto with multiple targets */
    static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    
    for (int i = 0; i < 4; i++) {
        goto *labels[i];
        
        label1: {
            int x = checksum;
            x = x * 3 + 1;
            checksum = x;
            continue;
        }
        
        label2: {
            int x = checksum;
            x = x ^ 0xAAAAAAAA;
            checksum = x;
            continue;
        }
        
        label3: {
            int x = checksum;
            x = x >> 1;
            checksum = x;
            continue;
        }
        
        label4: {
            int x = checksum;
            x = x + 1000;
            checksum = x;
            continue;
        }
    }
    
    /* Region 5: Multiple scheduling passes simulation */
    #pragma GCC optimize("O2")
    {
        int opt_var = checksum;
        for (int i = 0; i < 10; i++) {
            opt_var = opt_var * 3 - 1;
            /* Force memory clobber */
            asm volatile ("" : : "r"(opt_var) : "memory");
        }
        checksum = opt_var;
    }
    
    #pragma GCC optimize("O3")
    {
        int opt_var = checksum;
        /* Instruction level parallelism */
        int p1 = opt_var + 1;
        int p2 = opt_var * 2;
        int p3 = opt_var & 0xFF;
        int p4 = opt_var >> 4;
        
        /* Combine results */
        checksum = p1 + p2 + p3 + p4;
    }
    
    /* Final computation with profile hints */
    int result = checksum;
    if (__builtin_expect(result > 1000000, 0)) {
        result = result % 1000000;
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result & 0x7FFFFFFF;
}
