#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force frame pointer usage by taking addresses */
#define USE_FRAME_POINTER volatile int *fp_var = &force_fp

/* Helper functions with different attributes */
static int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    /* Force register pressure in helper */
    volatile int x1 = a * b;
    volatile int x2 = c + d;
    volatile int x3 = e ^ f;
    volatile int x4 = a | c;
    volatile int x5 = b & d;
    
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    
    return x1 + x2 - x3 + x4 - x5;
}

static float __attribute__((noinline)) helper2(float a, float b, float c, float d, 
                                               float e, float f, float g, float h) {
    /* Many float arguments to exceed register limits */
    volatile float r1 = a * b;
    volatile float r2 = c / d;
    volatile float r3 = e + f;
    volatile float r4 = g - h;
    volatile float r5 = a + c + e + g;
    
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    return r1 + r2 + r3 + r4 + r5;
}

int __attribute__((noinline)) helper3(int *ptr, int a, int b, int c, int d, int e) {
    /* Mixed pointer and integer arguments */
    volatile int sum = *ptr + a + b + c + d + e;
    volatile int prod = a * b * c;
    volatile int diff = d - e;
    
    /* Force stack usage */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = sum + i;
    }
    
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13", "r14", "r15");
    
    return sum + prod - diff + arr[5];
}

/* Non-static function with different calling convention */
float __attribute__((noinline)) helper4(double a, double b, int c, float d, 
                                        long e, short f, char g) {
    /* Mixed types to stress register allocation */
    volatile double d1 = a * b;
    volatile float f1 = d * c;
    volatile long l1 = e + (long)c;
    volatile int i1 = (int)f * g;
    
    __asm__ volatile ("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm6", "xmm7");
    
    return (float)d1 + f1 + (float)l1 + (float)i1;
}

/* Function that returns pointer */
int* __attribute__((noinline)) helper5(int a, int b, int c, int d, int e, int f, 
                                       int g, int h, int i, int j) {
    /* Many arguments to force stack passing */
    static int result;
    volatile int sum = a + b + c + d + e + f + g + h + i + j;
    volatile int prod = a * b * c;
    volatile int xor_val = d ^ e ^ f;
    
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
    
    result = sum + prod - xor_val;
    return &result;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile int force_fp = 0;  /* Force frame pointer usage */
    
    USE_FRAME_POINTER;
    
    int checksum = 0;
    
    /* Create complex control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic block 1: Multiple computations between calls */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 - iteration;
        f1 = f1 * 1.5f;
        f2 = f2 / 1.5f;
        
        /* Call with many live values in registers */
        int r1 = helper1(v1, v2, v3, v4, v5, iteration);
        checksum += r1;
        
        /* Inline assembly that clobbers call-clobbered registers */
        __asm__ volatile (
            "mov $0x12345678, %%rax\n\t"
            "mov $0x87654321, %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            : : : "rax", "rbx", "rcx", "rdx"
        );
        
        /* Basic block 2: More computations */
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        f3 = f3 + f1;
        f4 = f4 - f2;
        
        /* Force spill/reload by using all variables */
        v5 = v1 + v2 + v3 + v4;
        v6 = v5 * 2;
        v7 = v6 / 3;
        v8 = v7 | v1;
        v9 = v8 & v2;
        v10 = v9 ^ v3;
        
        /* Another call with float arguments */
        float r2 = helper2(f1, f2, f3, f4, f5, 6.6f, 7.7f, 8.8f);
        checksum += (int)r2;
        
        /* Conditional to create basic block boundary */
        if (iteration % 2 == 0) {
            /* Basic block 3: Inside if */
            v1 = helper3(p1, v1, v2, v3, v4, v5);
            checksum += v1;
            
            /* More inline assembly clobbering */
            __asm__ volatile (
                "pxor %%xmm0, %%xmm0\n\t"
                "mov $1, %%r10\n\t"
                "mov $2, %%r11\n\t"
                : : : "xmm0", "xmm1", "r10", "r11", "r12"
            );
            
            v2 = v2 * 3;
            v3 = v3 + 7;
        } else {
            /* Basic block 4: Inside else */
            float r3 = helper4(1.1, 2.2, v6, 3.3f, 4L, 5, 'a');
            checksum += (int)r3;
            
            v4 = v4 - 5;
            v5 = v5 ^ 0xFF;
        }
        
        /* Basic block 5: After if/else */
        int* ptr_result = helper5(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += *ptr_result;
        
        /* Loop creates another basic block boundary */
        f5 = f5 * 2.0f;
        p2 = &v10;
        
        /* Final inline assembly with many clobbers */
        __asm__ volatile (
            "mov %%rsp, %%rax\n\t"
            "add $16, %%rax\n\t"
            : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
    }
    
    /* Use alloca to force frame pointer usage */
    int* dynamic = alloca(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) {
        dynamic[i] = checksum + i;
        checksum += dynamic[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
