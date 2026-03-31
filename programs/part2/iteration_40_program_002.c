/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -fno-omit-frame-pointer */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline, target("no-sse"))) 
int helper1(int a, int b) {
    volatile int v = a;
    return v * b + (v >> 3);
}

__attribute__((noinline, target("default"))) 
float helper2(float a, float b) {
    volatile float v = a;
    asm volatile ("" : "+x" (v));
    return v * b + 1.0f;
}

__attribute__((noinline))
void helper3(volatile int* p, int val) {
    *p = (*p & 0xFF) + val;
    asm volatile ("mfence" ::: "memory");
}

/* Complex function with mixed operations */
__attribute__((noinline))
int complex_region(int* arr, float* farr, volatile int* varr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    /* Region 1: Mixed integer operations with volatile */
    for (int i = 0; i < n; i++) {
        volatile int vi = arr[i];
        int a = vi + i;
        int b = a * 3;
        int c = b >> 2;
        int d = c ^ 0x55AA55AA;
        int e = d & 0x00FF00FF;
        int f = e | (i << 16);
        sum += f;
        
        /* Memory barrier to split scheduling regions */
        asm volatile ("" ::: "memory");
        
        /* SIMD operations when available */
        if (i % 8 == 0) {
            __m128i vec = _mm_set_epi32(arr[i], arr[i+1], arr[i+2], arr[i+3]);
            vec = _mm_add_epi32(vec, _mm_set1_epi32(i));
            int tmp[4];
            _mm_storeu_si128((__m128i*)tmp, vec);
            sum += tmp[0] + tmp[3];
        }
    }
    
    return sum;
}

/* Function with switch statement creating multiple basic blocks */
__attribute__((noinline))
int switch_region(int x, int y) {
    int result = 0;
    
    switch (x & 0x7) {
        case 0: {
            /* Long chain of dependent operations */
            int a = y + 1;
            int b = a * 2;
            int c = b - y;
            int d = c << 3;
            int e = d | 0xF0F0;
            int f = e ^ a;
            int g = f + b;
            int h = g * c;
            result = h;
            asm volatile ("nop; nop; nop" ::: "eax", "memory");
            break;
        }
        case 1: {
            /* Floating point mixed with integer */
            float fa = (float)y;
            float fb = fa * 1.5f;
            int ia = (int)fb;
            int ib = ia * 3;
            result = ib + helper2(fa, fb);
            break;
        }
        case 2: {
            /* Vector operations */
            __m128i v1 = _mm_set_epi32(x, y, x+y, x-y);
            __m128i v2 = _mm_set1_epi32(0x12345678);
            __m128i v3 = _mm_add_epi32(v1, v2);
            int tmp[4];
            _mm_storeu_si128((__m128i*)tmp, v3);
            result = tmp[0] + tmp[1] + tmp[2] + tmp[3];
            break;
        }
        case 3: {
            /* Many independent operations */
            int r1 = y * 2;
            int r2 = y + 7;
            int r3 = y ^ 0xFF;
            int r4 = y >> 4;
            int r5 = r1 * r2;
            int r6 = r3 & r4;
            int r7 = r5 | r6;
            int r8 = r7 << 1;
            int r9 = r8 - y;
            int r10 = r9 * 3;
            result = r10;
            break;
        }
        case 4: {
            /* Volatile accesses with barriers */
            volatile int v = x;
            result = helper1(v, y);
            helper3(&v, y);
            result += v;
            break;
        }
        default: {
            /* Complex chain with function calls */
            result = x;
            for (int i = 0; i < 4; i++) {
                result = helper1(result, y + i);
                asm volatile ("" ::: "memory");
            }
            break;
        }
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    const int SIZE = 256;
    int array[SIZE];
    float farray[SIZE];
    volatile int varray[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = i * 3 + 1;
        farray[i] = (float)i * 0.5f;
        varray[i] = i;
    }
    
    int total = 0;
    
    /* Region A: Complex loop with mixed operations */
    #pragma GCC optimize("O3")
    {
        total += complex_region(array, farray, varray, SIZE);
    }
    
    /* Region B: Switch-based computation */
    #pragma GCC optimize("O2")
    {
        for (int i = 0; i < 100; i++) {
            total += switch_region(i, total);
            
            /* Insert scheduling barrier */
            asm volatile ("" ::: "memory");
            
            /* Additional independent operations */
            __m128i v = _mm_set_epi32(i, total, i+total, i-total);
            v = _mm_mullo_epi16(v, _mm_set1_epi16(2));
            int tmp[4];
            _mm_storeu_si128((__m128i*)tmp, v);
            total += tmp[0] + tmp[2];
        }
    }
    
    /* Region C: Nested loops with data-dependent exits */
    #pragma GCC optimize("O3")
    {
        int outer = 50;
        while (outer-- > 0) {
            int inner = outer;
            int local_sum = 0;
            
            do {
                /* Mixed-type operations */
                float fval = farray[inner];
                int ival = array[inner];
                volatile int vval = varray[inner];
                
                /* Chain of operations */
                int t1 = ival + vval;
                float t2 = fval * (float)t1;
                int t3 = (int)t2;
                int t4 = t3 ^ ival;
                int t5 = t4 << (inner & 3);
                local_sum += t5;
                
                /* Architecture-specific builtins */
                local_sum += __builtin_popcount(t5);
                local_sum += __builtin_ctz(t5 | 1);
                
                /* Inline assembly with register constraints */
                asm volatile (
                    "addl %%eax, %0\n\t"
                    "rorl $3, %%eax"
                    : "+r" (local_sum)
                    : "a" (t5)
                    : "cc"
                );
                
                inner -= (local_sum & 1) + 1;
            } while (inner > 0 && inner < SIZE);
            
            total += local_sum;
        }
    }
    
    /* Region D: Computed goto to create complex CFG */
    #pragma GCC optimize("Os")
    {
        static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        
        for (int i = 0; i < 5; i++) {
            goto *labels[i % 5];
            
        L0: {
            /* Vector horizontal sum */
            __m128 v = _mm_set_ps(farray[i], farray[i+1], farray[i+2], farray[i+3]);
            v = _mm_mul_ps(v, _mm_set1_ps(2.0f));
            float ftmp[4];
            _mm_storeu_ps(ftmp, v);
            total += (int)(ftmp[0] + ftmp[1] + ftmp[2] + ftmp[3]);
            continue;
        }
        
        L1: {
            /* Integer SIMD */
            __m128i vi = _mm_loadu_si128((__m128i*)&array[i*4]);
            vi = _mm_add_epi32(vi, _mm_set1_epi32(i));
            int itmp[4];
            _mm_storeu_si128((__m128i*)itmp, vi);
            total += itmp[0] * itmp[1] - itmp[2] + itmp[3];
            continue;
        }
        
        L2: {
            /* Volatile chain */
            volatile int chain = total;
            for (int j = 0; j < 8; j++) {
                chain = chain * 3 + j;
                asm volatile ("" : "+r" (chain) :: "memory");
            }
            total = chain;
            continue;
        }
        
        L3: {
            /* Mixed operations with function call */
            total += helper1(total, i);
            float f = helper2((float)total, (float)i);
            total += (int)f;
            continue;
        }
        
        L4: {
            /* Many parallel independent operations */
            int p1 = i * 2;
            int p2 = i + total;
            int p3 = i ^ total;
            int p4 = i << 2;
            int p5 = p1 * p2;
            int p6 = p3 | p4;
            int p7 = p5 + p6;
            int p8 = p7 - i;
            int p9 = p8 >> 1;
            int p10 = p9 & 0x7FFF;
            total = p10;
            continue;
        }
        }
    }
    
    /* Final checksum to prevent optimization */
    volatile int checksum = total;
    return checksum & 0x7FFFFFFF;
}
