/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 3.14159f; }
__attribute__((noinline)) double external_func3(double x) { return x / 2.71828; }

/* Volatile function pointer to prevent optimization */
static void (*volatile volatile_func_ptr)(void);

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;  /* Prevent dead store elimination */
    int a = v1 + 1;
    int b = a * 2;
    int c = b - 3;
    int d = c / 4;
    int e = d % 5;
    int f = e | 0xFF;
    int g = f & 0x0F;
    int h = g ^ 0x33;
    
    float fa = (float)a * 1.1f;
    float fb = (float)b * 2.2f;
    float fc = (float)c * 3.3f;
    float fd = (float)d * 4.4f;
    
    double da = (double)a * 1.111;
    double db = (double)b * 2.222;
    double dc = (double)c * 3.333;
    double dd = (double)d * 4.444;
    
    char ca = (char)a;
    short sa = (short)a;
    long long lla = (long long)a * 1000000LL;
    
    int* pa = &a;
    float* pfa = &fa;
    double* pda = &da;
    
    /* Complex intermediate computation - this is the value we want to rematerialize */
    int intermediate = (a * b) + (c * d) - (e * f) + (g * h);
    intermediate = external_func1(intermediate);
    
    /* Volatile variable for conditional control */
    volatile int control = seed % 7;
    
    /* Label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile_func_ptr = (void(*)())labels[control % 5];
    
    /* Deeply nested conditional structure */
    if (control > 0) {
        if (control > 2) {
            switch (control) {
                case 3: {
                    /* Use intermediate in conditional block with inline asm */
                    int temp = intermediate;
                    asm volatile (
                        "mov %0, %0\n\t"
                        "add %0, #1\n\t"
                        : "+r" (temp)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
                    );
                    intermediate = temp;
                    break;
                }
                case 4: {
                    float ftemp = external_func2(fa);
                    asm volatile (
                        "fadds s0, s0, s0\n\t"
                        : 
                        : "w" (ftemp)
                        : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                          "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15"
                    );
                    break;
                }
                default: {
                    double dtemp = external_func3(da);
                    asm volatile (
                        "faddd d0, d0, d0\n\t"
                        : 
                        : "w" (dtemp)
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15"
                    );
                    break;
                }
            }
            
            /* Another level of nesting */
            if (control % 2 == 0) {
                for (int i = 0; i < 3; i++) {
                    intermediate += i;
                    if (i == 1) {
                        /* Use intermediate with different mode (char) */
                        char ctemp = (char)intermediate;
                        asm volatile (
                            "add %0, #1\n\t"
                            : "+r" (ctemp)
                            :
                            : "memory"
                        );
                        intermediate = ctemp;
                    }
                }
            }
        } else {
            /* Use intermediate with short mode */
            short stemp = (short)intermediate;
            asm volatile (
                "add %0, #1\n\t"
                : "+r" (stemp)
                :
                : "cc", "memory"
            );
            intermediate = stemp;
        }
        
        /* Opaque control flow jump */
        goto *volatile_func_ptr;
    }
    
label1:
    /* Use intermediate after conditional block - forces live range extension */
    intermediate = intermediate * 2;
    
label2:
    /* More computations with mixed types to create different modes */
    fa = fa + fb;
    da = da + db;
    
label3:
    /* Use intermediate again */
    intermediate = intermediate + (int)fa;
    
label4:
    /* Pointer arithmetic */
    pa = pa + 1;
    pfa = pfa + 1;
    pda = pda + 1;
    
label5:
    /* Final aggregation to prevent elimination */
    int result = intermediate + a + b + c + d + e + f + g + h;
    result += (int)fa + (int)fb + (int)fc + (int)fd;
    result += (int)da + (int)db + (int)dc + (int)dd;
    result += ca + sa + (int)(lla & 0xFFFFFFFF);
    result += (int)((uintptr_t)pa + (uintptr_t)pfa + (uintptr_t)pda);
    
    return result;
}

/* Main function with loop to increase pressure */
int main(int argc, char** argv) {
    int total = 0;
    
    /* Loop to create more register pressure in caller */
    for (int i = 0; i < 100; i++) {
        int seed = argc > 1 ? atoi(argv[1]) : i;
        total += stress_function(seed + i);
        
        /* Additional local variables in main to increase pressure */
        int x1 = i * 2;
        int x2 = i * 3;
        int x3 = i * 4;
        float fx1 = (float)i * 1.5f;
        double dx1 = (double)i * 2.5;
        
        /* Use them to prevent elimination */
        total += x1 + x2 + x3 + (int)fx1 + (int)dx1;
    }
    
    return total % 256;  /* Prevent compiler from optimizing everything away */
}
