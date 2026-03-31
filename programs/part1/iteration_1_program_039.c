/* caller_save_test.c - ISO C99 compliant */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force specific calling conventions and prevent optimizations */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with varying signatures to create register pressure */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result;
}

NOINLINE static float helper2(float a, float b, float c, float d, 
                               float e, float f, float g, float h) {
    volatile float result = a * b + c * d - e * f + g / h;
    return result;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, void* p4,
                               void* p5, void* p6, void* p7) {
    volatile uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3 +
                            (uintptr_t)p4 + (uintptr_t)p5 + (uintptr_t)p6 + 
                            (uintptr_t)p7;
    return (void*)(sum % 256);
}

NOINLINE int helper4(int a, int b, int c, int d, int e, int f) {
    /* Use alloca to force frame pointer usage */
    int* arr = (int*)alloca(sizeof(int) * 4);
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
    return arr[0] + arr[1] - arr[2] + arr[3] + e - f;
}

NOINLINE double helper5(double a, double b, double c, double d, double e) {
    volatile double result = a * b + c - d / e;
    /* Inline assembly to clobber floating point registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    return result;
}

/* Function taking address of locals to force frame pointer */
NOINLINE static int use_frame_pointer(void) {
    int x = 42, y = 73, z = 19;
    int *px = &x, *py = &y, *pz = &z;
    volatile int sum = *px + *py + *pz;
    return sum;
}

/* Complex function with mixed types */
NOINLINE static float complex_helper(int i1, int i2, float f1, float f2,
                                     void* p1, void* p2) {
    volatile float result = (float)(i1 + i2) + f1 * f2;
    result += (float)((uintptr_t)p1 ^ (uintptr_t)p2) / 1000.0f;
    
    /* Clobber multiple registers */
    __asm__ volatile ("" : : : 
#if defined(__x86_64__)
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif defined(__i386__)
        "eax", "ebx", "ecx", "edx", "esi", "edi"
#endif
    );
    
    return result;
}

int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile void* p1 = (void*)0x1000, *p2 = (void*)0x2000;
    volatile void* p3 = (void*)0x3000, *p4 = (void*)0x4000;
    volatile int result = 0;
    volatile float fresult = 0.0f;
    
    /* Take addresses to force frame pointer in main */
    int* ptrs[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Loop to create basic blocks with calls inside */
    for (int i = 0; i < 3; i++) {
        /* Basic block 1: Integer computations and call */
        int temp1 = v1 + v2 * v3 - v4;
        int temp2 = v5 ^ v6 | v7 & v8;
        
        /* Function call with many arguments - will need caller-save */
        int res1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
        
        /* Inline assembly clobbering between computations */
        __asm__ volatile ("" : : : 
#if defined(__x86_64__)
            "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10"
#elif defined(__i386__)
            "eax", "ecx", "edx", "esi", "edi"
#endif
        );
        
        /* Basic block 2: Floating point computations */
        float ftemp1 = f1 * f2 + f3 / f4;
        float ftemp2 = f5 - f1 + f2 * f3;
        
        /* Another call with different signature */
        float fres1 = helper2(f1, f2, f3, f4, f5, f1, f2, f3);
        
        /* Conditional to create basic block boundaries */
        if (res1 > 0) {
            /* Basic block 3: Pointer operations */
            void* pres1 = helper3(p1, p2, p3, p4, p1, p2, p3);
            
            /* Mixed type call */
            fresult += complex_helper(v1, v2, f1, f2, p1, p2);
            
            /* More register pressure */
            int res2 = helper4(v1, v2, v3, v4, v5, v6);
            result += res1 + res2;
            
            /* Clobber registers again */
            __asm__ volatile ("" : : : 
#if defined(__x86_64__)
                "r11", "r12", "r13", "r14", "r15"
#endif
            );
        } else {
            /* Alternative path with different calls */
            double dres = helper5((double)f1, (double)f2, (double)f3, 
                                  (double)f4, (double)f5);
            fresult += (float)dres;
            
            /* Force frame pointer usage */
            result += use_frame_pointer();
        }
        
        /* Modify variables to prevent dead code elimination */
        v1 += i; v2 -= i; v3 *= (i + 1); v4 ^= i;
        f1 += 0.1f * i; f2 -= 0.1f * i; f3 *= 1.1f; f4 /= 1.1f;
    }
    
    /* Final computation using all variables */
    int final_result = 0;
    for (int i = 0; i < 10; i++) {
        final_result += *ptrs[i];
    }
    final_result += (int)fresult;
    
    printf("Result: %d (checksum: %08x)\n", final_result, 
           (unsigned int)final_result);
    
    return final_result != 0 ? 0 : 1;
}
