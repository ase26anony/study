/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) {
    volatile int barrier = 0;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Create register pressure with many live values */
            int a = arr1[i-1] + arr2[i+1];
            int b = arr1[i] * arr2[i];
            int c = a ^ b;
            int d = arr1[i+1] - arr2[i-1];
            
            float fa = farr1[i-1] * farr2[i+1];
            float fb = farr1[i] + farr2[i];
            float fc = fa - fb;
            float fd = farr1[i+1] / (farr2[i-1] + 1.0f);
            
            /* Volatile read creates scheduling barrier */
            int vol_read = barrier;
            
            /* Complex conditional with side effects in both branches */
            if ((c + vol_read) % 7 > 3) {
                /* Branch 1: chain of dependent operations */
                int t1 = a * d;
                int t2 = b ^ t1;
                int t3 = c + t2;
                arr1[i] = t3 ^ (d << 2);
                
                float ft1 = fa * fd;
                float ft2 = fb + ft1;
                float ft3 = fc - ft2;
                farr1[i] = ft3 * 1.125f;
                
                /* Inline assembly as scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* Use values computed much earlier */
                sum += arr1[i-1] + t3;
                fsum += farr1[i-1] + ft3;
            } else {
                /* Branch 2: different operation chain */
                int t1 = b - a;
                int t2 = d ^ t1;
                int t3 = c * t2;
                arr2[i] = t3 | (a >> 1);
                
                float ft1 = fb - fa;
                float ft2 = fd * ft1;
                float ft3 = fc + ft2;
                farr2[i] = ft3 / 1.0625f;
                
                /* Another inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Different late use of early values */
                sum += arr2[i+1] - t3;
                fsum += farr2[i+1] - ft3;
            }
            
            /* Cross-iteration dependencies */
            arr1[i-1] = arr2[i+1] ^ (i * outer);
            arr2[i-1] = arr1[i+1] & (i + outer);
            
            farr1[i-1] = farr2[i+1] * (0.5f + i);
            farr2[i-1] = farr1[i+1] + (0.25f * outer);
            
            /* More arithmetic to increase pressure */
            int e = (a * b) + (c ^ d);
            int f = (a << 3) | (b >> 2);
            int g = e ^ f;
            
            float fe = (fa * fb) + (fc - fd);
            float ff = fa * 2.0f + fb * 0.5f;
            float fg = fe * ff;
            
            /* Final accumulation with mixed types */
            sum += g % 1023;
            fsum += fg - (int)fg;
        }
        
        /* Modify barrier to affect conditionals */
        barrier = outer % 256;
    }
    
    /* Store final results */
    result[0] = sum;
    result[1] = (int)fsum;
}

int main() {
    int result[2] = {0, 0};
    int total_iterations = 1000;
    
    /* Call multiple times to ensure optimization */
    for (int run = 0; run < 3; run++) {
        stress_sched(total_iterations, result);
        
        /* Print progress to prevent dead code elimination */
        printf("Run %d: sum = %d, fsum = %d\n", 
               run, result[0], result[1]);
        
        /* Modify total_iterations slightly */
        total_iterations += run * 50;
    }
    
    /* Final checksum */
    int checksum = result[0] + result[1];
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
