/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-schedule-insns -fno-schedule-insns2 test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) MixedData {
    char c;
    short s;
    int i;
    long long ll;
};

/* Function with high register pressure and complex dataflow */
int complex_remat_candidate(int x1, int x2, int x3, int x4, int x5,
                           int x6, int x7, int x8, int x9, int x10) {
    /* Many initial live values */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4;
    int d = c + x5 * 2;
    int e = d - x6 / 3;
    int f = e | x7;
    int g = f & x8;
    int h = g + x9 << 2;
    int i = h - x10 >> 1;
    int j = i * 3 + v3;
    
    /* Mixed-type arithmetic causing mode conversions */
    short s1 = (short)a;
    short s2 = (short)b;
    char c1 = (char)c;
    char c2 = (char)d;
    
    /* More intermediate values - creating register pressure */
    int k = j + (int)s1 * 2;
    int l = k - (int)s2 / 4;
    int m = l | (int)c1;
    int n = m & (int)c2;
    int o = n + a * b;
    int p = o - c * d;
    int q = p ^ e * f;
    int r = q | g * h;
    int s = r & i * j;
    int t = s + k * l;
    int u = t - m * n;
    int v = u ^ o * p;
    int w = v | q * r;
    int x = w & s * t;
    int y = x + u * v;
    int z = y - w * x;
    
    /* Complex control flow with switch */
    int switch_val = (z & 0xF) + v1;
    int result = 0;
    
    for (int iter = 0; iter < 100; iter++) {
        /* Loop increases register pressure further */
        int loop_var = iter * 2 + v2;
        
        switch (switch_val % 7) {
            case 0:
                result += a + b + (short)c + loop_var;
                /* Mode mixing */
                result = (result << 3) | (result & 0xFF);
                break;
            case 1:
                result += c + d + (char)e + loop_var;
                result = (result >> 2) ^ (result & 0xFFFF);
                break;
            case 2:
                result += e + f + (int)s1 + loop_var;
                result = (result * 3) & 0xFFFFFF;
                break;
            case 3:
                result += g + h + (int)c1 + loop_var;
                result = (result + 0x1234) | 0xAA;
                break;
            case 4:
                result += i + j + s2 + loop_var;
                result = (result - 0x5678) ^ 0x55;
                break;
            case 5:
                result += k + l + c2 + loop_var;
                result = (result << 1) + (result >> 31);
                break;
            case 6:
                result += m + n + (short)o + loop_var;
                result = (result & 0xCC) | (result & 0x33);
                break;
        }
        
        /* Address calculations that are rematerialization candidates */
        int* ptr1 = &a + iter;
        int* ptr2 = &b + iter;
        int* ptr3 = &c + iter;
        
        /* Use the pointers in computations */
        result += *ptr1 + *ptr2 + *ptr3;
        
        /* More mixed-type operations */
        long long ll1 = (long long)a * b;
        long long ll2 = (long long)c * d;
        result += (int)(ll1 + ll2) >> 4;
        
        /* Bit-field like operations */
        struct MixedData md;
        md.c = (char)(result & 0xFF);
        md.s = (short)(result & 0xFFFF);
        md.i = result;
        md.ll = (long long)result * result;
        
        result += md.c + md.s + md.i + (int)(md.ll & 0xFFFFFFFF);
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r" (result)
            : "r" (loop_var), "r" (iter)
            : "cc"
        );
        
        /* Update switch value for next iteration */
        switch_val = (switch_val * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final mixing to prevent optimization */
    result = result ^ v1 ^ v2 ^ v3;
    result = (result << 16) | (result >> 16);
    
    return result;
}

/* Another function with vector operations */
#ifdef __SSE2__
#include <emmintrin.h>
void vector_operations(int* data, int size) {
    __m128i vec1, vec2, vec3;
    
    for (int i = 0; i < size; i += 4) {
        vec1 = _mm_loadu_si128((__m128i*)(data + i));
        vec2 = _mm_set1_epi32(42);  /* Constant for rematerialization */
        vec3 = _mm_add_epi32(vec1, vec2);
        _mm_storeu_si128((__m128i*)(data + i), vec3);
    }
}
#endif

/* Main function with multiple calls to create register pressure */
int main() {
    int result = 0;
    
    /* Call with many different arguments to create diverse dataflow */
    for (int i = 0; i < 50; i++) {
        result += complex_remat_candidate(
            i * 1, i * 2, i * 3, i * 4, i * 5,
            i * 6, i * 7, i * 8, i * 9, i * 10
        );
        
        /* Additional computations between calls */
        int temp = result;
        temp = (temp << 5) | (temp >> 27);
        temp = temp ^ (i * 0x5A5A5A5A);
        result = temp + v1;
    }
    
#ifdef __SSE2__
    int data[64];
    for (int i = 0; i < 64; i++) {
        data[i] = i + result;
    }
    vector_operations(data, 64);
    
    /* Use vector results */
    for (int i = 0; i < 64; i++) {
        result += data[i];
    }
#endif
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
