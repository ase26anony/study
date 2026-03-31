/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + barrier;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float barrier = 1.0f;
    asm volatile("" : "+x"(a), "+x"(b) : : "memory");
    return a / b + barrier;
}

__attribute__((noinline)) static void helper3(volatile int* p, int n) {
    for (int i = 0; i < n; i++) {
        *p = *p + i;
        asm volatile("" : : : "memory");
    }
}

/* Complex function with multiple scheduling regions */
__attribute__((noinline)) static uint64_t complex_scheduling_region(
    int* arr_int, float* arr_float, volatile int* vol_arr,
    __m128i* vec_arr, int mode) {
    
    uint64_t checksum = 0;
    
    /* Region 1: Mixed integer operations with volatile barriers */
    {
        int a = arr_int[0];
        int b = arr_int[1];
        volatile int* vp = &vol_arr[0];
        
        /* Chain of dependent operations */
        a = a + b;
        *vp = *vp + 1;  /* Volatile access creates scheduling barrier */
        b = a * 2;
        a = b >> 3;
        *vp = *vp + a;  /* Another volatile barrier */
        int c = helper1(a, b);
        
        /* Independent operations that can fill instruction queue */
        int d = arr_int[2] + arr_int[3];
        int e = arr_int[4] * arr_int[5];
        int f = arr_int[6] & arr_int[7];
        int g = arr_int[8] | arr_int[9];
        
        /* Memory barrier to split scheduling region */
        asm volatile("" : : : "memory");
        
        checksum += a + b + c + d + e + f + g;
    }
    
    /* Region 2: SIMD operations with inline assembly */
    {
        __m128i v1 = vec_arr[0];
        __m128i v2 = vec_arr[1];
        
        /* Vector operations that may use target-specific scheduling */
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi16(v1, v2);
        
        /* Inline assembly with register constraints */
        asm volatile(
            "movdqa %1, %%xmm0\n\t"
            "paddd %%xmm0, %0\n\t"
            : "+x"(v3) : "x"(v4) : "xmm0"
        );
        
        /* Extract results */
        int results[4];
        _mm_store_si128((__m128i*)results, v3);
        
        for (int i = 0; i < 4; i++) {
            checksum += results[i];
        }
    }
    
    /* Region 3: Floating point with control flow */
    {
        float x = arr_float[0];
        float y = arr_float[1];
        
        /* Dependent FP chain */
        x = x + y;
        y = x * 2.0f;
        x = helper2(x, y);
        
        /* Switch with multiple cases creating different scheduling paths */
        switch (mode & 0x3) {
            case 0:
                x = x * 3.14159f;
                asm volatile("" : : : "memory");
                break;
            case 1:
                x = x / 2.71828f;
                /* Memory clobber */
                asm volatile("" : : : "memory");
                break;
            case 2:
                x = x + x;
                /* Explicit register clobber */
                asm volatile("" : : : "eax", "memory");
                break;
            default:
                x = x - 1.0f;
                break;
        }
        
        /* Use unlikely branch to affect scheduling */
        if (__builtin_expect(x > 1000.0f, 0)) {
            x = 1000.0f;
        }
        
        checksum += (uint64_t)x;
    }
    
    /* Region 4: Nested loops with volatile accesses */
    {
        volatile int counter = 0;
        int sum = 0;
        
        for (int i = 0; i < 8; i++) {
            /* Inner loop with data-dependent exit */
            for (int j = 0; j < (i + 2); j++) {
                sum += arr_int[i] * j;
                *(&counter) = *(&counter) + 1;  /* Volatile access */
                
                /* Architecture-specific builtin */
                sum += __builtin_popcount(arr_int[j % 8]);
            }
            
            /* Conditional break that affects instruction queue */
            if (sum > 1000) {
                asm volatile("" : : : "memory");
                break;
            }
        }
        
        helper3(&counter, 2);
        checksum += sum + counter;
    }
    
    /* Region 5: Parallel independent operations */
    {
        /* Many independent instructions to fill ready list */
        int r1 = arr_int[10] + 1;
        int r2 = arr_int[11] * 2;
        int r3 = arr_int[12] & 0xFF;
        int r4 = arr_int[13] | 0xAA;
        int r5 = arr_int[14] ^ 0x55;
        int r6 = arr_int[15] << 2;
        int r7 = arr_int[16] >> 1;
        int r8 = __builtin_ctz(arr_int[17]);
        int r9 = helper1(r1, r2);
        float r10 = helper2(r3, r4);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* More independent ops */
        int r11 = r1 + r2;
        int r12 = r3 * r4;
        int r13 = r5 & r6;
        int r14 = r7 | r8;
        
        checksum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + 
                   (int)r10 + r11 + r12 + r13 + r14;
    }
    
    return checksum;
}

/* Main function with multiple complex scheduling regions */
int main() {
    /* Initialize arrays with different data types */
    int arr_int[32];
    float arr_float[32];
    volatile int vol_arr[32];
    __m128i vec_arr[4];
    
    /* Fill arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr_int[i] = i * 3 + 1;
        arr_float[i] = i * 1.5f;
        vol_arr[i] = i * 2;
    }
    
    /* Initialize vector array */
    for (int i = 0; i < 4; i++) {
        int vals[4] = {i*4, i*4+1, i*4+2, i*4+3};
        vec_arr[i] = _mm_loadu_si128((__m128i*)vals);
    }
    
    uint64_t total_checksum = 0;
    
    /* Call scheduling region multiple times with different modes */
    for (int mode = 0; mode < 8; mode++) {
        /* Change optimization pragma to potentially trigger different scheduling */
        #pragma GCC optimize("O3")
        total_checksum += complex_scheduling_region(
            arr_int, arr_float, vol_arr, vec_arr, mode);
        
        #pragma GCC optimize("O2")
        total_checksum += complex_scheduling_region(
            arr_int + 8, arr_float + 8, vol_arr + 8, vec_arr, mode ^ 1);
    }
    
    /* Use computed checksum to prevent dead code elimination */
    volatile uint64_t result = total_checksum;
    
    /* Additional complex region in main */
    {
        int a = 0;
        for (int i = 0; i < 100; i++) {
            /* Mix of operations with goto to create interesting control flow */
            if (i % 3 == 0) goto label1;
            if (i % 5 == 0) goto label2;
            
            a += arr_int[i % 32];
            continue;
            
        label1:
            a += __builtin_popcount(i);
            asm volatile("" : : : "memory");
            continue;
            
        label2:
            a += i * 2;
            /* Force register spilling */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        }
        
        result += a;
    }
    
    return (int)(result & 0x7FFFFFFF);
}
