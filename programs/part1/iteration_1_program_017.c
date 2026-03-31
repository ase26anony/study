#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create varied call sites */

__attribute__((noinline))
static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result * 2;
}

__attribute__((noinline))
float helper2(float a, float b, float c, float d, 
              float e, float f, float g, float h,
              float i, float j) {
    volatile float sum = a + b + c + d + e + f + g + h + i + j;
    return sum / 10.0f;
}

__attribute__((noinline, optimize("no-omit-frame-pointer")))
void* helper3(void* p1, void* p2, void* p3, void* p4,
              void* p5, void* p6, void* p7, void* p8) {
    /* Force frame pointer usage by taking address of locals */
    int local1 = 42;
    int local2 = 99;
    volatile int* addr1 = &local1;
    volatile int* addr2 = &local2;
    
    /* Complex pointer arithmetic to keep values live */
    uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3 + (uintptr_t)p4 +
                   (uintptr_t)p5 + (uintptr_t)p6 + (uintptr_t)p7 + (uintptr_t)p8;
    
    /* Use alloca to influence stack frame */
    char* buffer = (char*)alloca(64);
    for (int i = 0; i < 64; i++) {
        buffer[i] = (char)(sum + i);
    }
    
    return (void*)(sum % 256);
}

__attribute__((noinline))
double helper4(double a, double b, double c, double d,
               double e, double f) {
    volatile double prod = a * b * c * d * e * f;
    
    /* Inline assembly that clobbers multiple registers */
    __asm__ volatile (
        "# Force clobbering\n"
        : 
        : "r"(a), "r"(b), "r"(c)
        : "rax", "r10", "r11", "xmm0", "xmm1", "xmm2", "memory"
    );
    
    return prod + 1.0;
}

static int __attribute__((noinline)) 
helper5(int a, int b, int c, int d, int e) {
    /* Mix of operations to create register pressure */
    int t1 = a * b;
    int t2 = c * d;
    int t3 = e * t1;
    int t4 = t2 + t3;
    int t5 = t1 - t2;
    int t6 = t3 * t4;
    int t7 = t5 + t6;
    
    /* More inline assembly clobbering */
    __asm__ volatile (
        "# Clobber integer registers\n"
        : 
        : "r"(t1), "r"(t2)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return t7 % 1000;
}

/* Function with many live values across calls */
__attribute__((optimize("no-omit-frame-pointer")))
int compute_checksum(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    
    volatile double d1 = 1.01;
    volatile double d2 = 2.02;
    volatile double d3 = 3.03;
    
    void* p1 = (void*)&v1;
    void* p2 = (void*)&v2;
    void* p3 = (void*)&v3;
    void* p4 = (void*)&v4;
    void* p5 = (void*)&v5;
    void* p6 = (void*)&v6;
    
    int result = 0;
    
    /* Complex control flow creating basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            v1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
            
            /* Keep values live in registers between calls */
            v2 = v1 + v3;
            v3 = v2 * v4;
            
            /* Inline assembly that clobbers call-clobbered registers */
            __asm__ volatile (
                "# Clobber before next call\n"
                : 
                : "r"(v1), "r"(v2), "r"(v3)
                : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
            );
            
            f1 = helper2(f1, f2, f3, f4, f5, 
                         f1 + 0.5f, f2 + 1.0f, f3 + 1.5f,
                         f4 + 2.0f, f5 + 2.5f);
                         
            /* More computations keeping values live */
            v4 = v3 - v2;
            v5 = v4 * v1;
            
        } else {
            /* Alternative basic block path */
            void* ptr_result = helper3(p1, p2, p3, p4, p5, p6, p1, p2);
            v6 = (int)(uintptr_t)ptr_result;
            
            /* Force register pressure with many live values */
            v7 = v6 + v1 + v2 + v3 + v4 + v5;
            v8 = v7 * 2;
            v9 = v8 - v6;
            v10 = v9 / 2;
            
            /* Another inline assembly clobber */
            __asm__ volatile (
                "# Extensive clobber list\n"
                : 
                : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                  "r"(f1), "r"(f2)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                  "memory"
            );
            
            d1 = helper4(d1, d2, d3, d1 * 1.1, d2 * 1.2, d3 * 1.3);
        }
        
        /* Common code with another call */
        v1 = helper5(v1, v2, v3, v4, v5);
        
        /* Loop-carried dependencies to prevent optimization */
        v2 += iteration;
        v3 -= iteration;
        f1 += (float)iteration * 0.1f;
        f2 -= (float)iteration * 0.05f;
        
        /* Mix in more inline assembly */
        __asm__ volatile (
            "# Final clobber in loop\n"
            : 
            : "r"(v1), "r"(v2), "r"(v3), "r"(f1), "r"(f2)
            : "rax", "rcx", "rdx", "xmm0", "xmm1", "memory"
        );
    }
    
    /* Final computation using all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)d1 + (int)d2 + (int)d3;
    
    return result;
}

/* Additional function to create more call sites */
__attribute__((noinline, optimize("O3")))
void stress_caller_save(int iterations) {
    volatile int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying call patterns */
        if (i % 4 == 0) {
            accum += helper1(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        } else if (i % 4 == 1) {
            accum += (int)helper2((float)i, (float)(i+1), (float)(i+2),
                                 (float)(i+3), (float)(i+4), (float)(i+5),
                                 (float)(i+6), (float)(i+7), (float)(i+8),
                                 (float)(i+9));
        } else if (i % 4 == 2) {
            void* ptrs[8];
            for (int j = 0; j < 8; j++) {
                ptrs[j] = (void*)(uintptr_t)(i + j);
            }
            accum += (int)(uintptr_t)helper3(ptrs[0], ptrs[1], ptrs[2], ptrs[3],
                                            ptrs[4], ptrs[5], ptrs[6], ptrs[7]);
        } else {
            accum += helper5(i, i*2, i*3, i*4, i*5);
        }
        
        /* Insert inline assembly between calls */
        __asm__ volatile (
            "# Stress-test clobber\n"
            : 
            : "r"(accum), "r"(i)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "xmm0", "xmm1", "memory"
        );
    }
    
    printf("Stress test accumulated: %d\n", accum);
}

int main(void) {
    printf("Starting caller-save coverage test...\n");
    
    /* First test: complex computation with many live values */
    int checksum = compute_checksum();
    printf("Checksum 1: %d\n", checksum);
    
    /* Second test: stress with many calls */
    stress_caller_save(50);
    
    /* Third test: nested calls with register pressure */
    {
        volatile int a = 100, b = 200, c = 300;
        volatile float x = 10.5f, y = 20.5f, z = 30.5f;
        
        for (int i = 0; i < 10; i++) {
            a = helper1(a, b, c, i, i+1, i+2, i+3, i+4);
            
            __asm__ volatile (
                "# Nested test clobber\n"
                : 
                : "r"(a), "r"(b), "r"(c), "r"(x), "r"(y), "r"(z)
                : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "memory"
            );
            
            x = helper2(x, y, z, x+1.0f, y+2.0f, z+3.0f,
                       x*1.1f, y*1.2f, z*1.3f, (float)i);
            
            b = helper5(b, c, a, i, i*2);
        }
        
        printf("Final values: a=%d, b=%d, c=%d, x=%.2f, y=%.2f, z=%.2f\n",
               a, b, c, x, y, z);
    }
    
    printf("Test completed.\n");
    return 0;
}
