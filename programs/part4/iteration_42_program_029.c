/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline,noipa))
static void stress_sched(int iterations) {
    /* Create register pressure with mixed types */
    volatile int seed = 12345;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, k;
    
    /* Initialize arrays with volatile to prevent optimization */
    volatile int init = seed;
    for (k = 0; k < 32; k++) {
        arr1[k] = init + k;
        arr2[k] = init - k;
        farr1[k] = (float)(init * k) / 3.14159f;
        farr2[k] = (float)(init + k) * 2.71828f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (i = 0; i < iterations; i++) {
        int temp1 = arr1[i & 31];
        int temp2 = arr2[i & 31];
        float ftemp1 = farr1[i & 31];
        float ftemp2 = farr2[i & 31];
        
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 128; j++) {
            /* Chain of dependent integer operations */
            int a = temp1 * 3 + j;
            int b = temp2 * 5 - j;
            int c = a * b + temp1;
            int d = c ^ (a << 3);
            int e = d | (b >> 2);
            int f = e + (c & 0xFF);
            
            /* Chain of dependent floating-point operations */
            float fa = ftemp1 * 1.5f + (float)j;
            float fb = ftemp2 * 2.5f - (float)j;
            float fc = fa * fb + ftemp1;
            float fd = fc / (fa + 1.0f);
            float fe = fd * 3.14159f - fb;
            
            /* Conditional execution with side effects */
            if ((f ^ e) > (d | c)) {
                /* Branch 1: Different arithmetic pattern */
                arr1[(j + 1) & 31] = (f * 7) - (e >> 1);
                farr1[(j + 2) & 31] = fe * 2.0f + fd;
                
                /* More operations in this branch */
                int g = (farr1[(j + 2) & 31] > 0.0f) ? 1 : 0;
                arr2[(j + 3) & 31] = g * 100 + f;
            } else {
                /* Branch 2: Alternative computation path */
                arr1[(j + 4) & 31] = (f / 3) + (e << 1);
                farr1[(j + 5) & 31] = fe / 2.0f - fd;
                
                /* Different operations in else branch */
                float fg = farr1[(j + 5) & 31] * 1.618034f;
                arr2[(j + 6) & 31] = (int)fg * 50 + e;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Cross-branch value usage - extends live ranges */
            temp1 = arr1[(j + 7) & 31] + arr2[(j + 8) & 31];
            temp2 = arr1[(j + 9) & 31] - arr2[(j + 10) & 31];
            
            /* More floating-point ops using values from both branches */
            ftemp1 = farr1[(j + 11) & 31] * farr2[(j + 12) & 31];
            ftemp2 = farr1[(j + 13) & 31] / (farr2[(j + 14) & 31] + 1.0f);
            
            /* Additional arithmetic to increase complexity */
            int h = temp1 * temp2 + (j & 0xF);
            float fh = ftemp1 + ftemp2 * (float)(j & 0x7);
            
            /* Store results with complex indexing */
            arr1[(j + 15) & 31] = h ^ (f > 0 ? f : -f);
            farr2[(j + 16) & 31] = fh + (fe > 0.0f ? fe : -fe);
            
            /* Another conditional with different pattern */
            if ((j & 3) == 0) {
                arr2[(j + 17) & 31] = (int)(fh * 100.0f) + h;
                farr1[(j + 18) & 31] = (float)h / 256.0f;
            }
        }
        
        /* Update loop variant for next iteration */
        volatile int update = seed + i;
        arr1[i & 31] += update;
        arr2[i & 31] -= update;
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = 0;
    for (k = 0; k < 32; k++) {
        checksum += arr1[k] + arr2[k] + (int)farr1[k] + (int)farr2[k];
    }
    
    /* Use checksum to prevent optimization */
    asm volatile("" : "+r" (checksum) : : "memory");
}

int main() {
    int i, total = 0;
    
    /* Call the function multiple times to ensure execution */
    for (i = 0; i < 10; i++) {
        stress_sched(50);
        total += i * 100;
    }
    
    printf("Test completed. Total: %d\n", total);
    return 0;
}
