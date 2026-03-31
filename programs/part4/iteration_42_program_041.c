/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline,noipa))
stress_sched(int iterations) {
    volatile int seed = 12345;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with volatile to prevent optimization */
    for (i = 0; i < 32; i++) {
        arr1[i] = seed + i;
        arr2[i] = seed - i;
        farr1[i] = (float)(seed * i) * 0.1f;
        farr2[i] = (float)(seed / (i + 1)) * 0.01f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < iterations; j++) {
        /* Complex inner loop with high ILP and register pressure */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int t1 = arr1[i-1] * 3;
            int t2 = arr2[i+1] + t1;
            int t3 = t2 ^ (arr1[i] << 2);
            int t4 = t3 - arr2[i-1];
            int t5 = t4 * 7 + j;
            
            /* Chain of dependent floating-point operations */
            float f1 = farr1[i] * 2.5f;
            float f2 = farr2[i-1] + f1;
            float f3 = f2 / (farr1[i+1] + 1.0f);
            float f4 = f3 * farr2[i] - (float)t5;
            
            /* Conditional execution with side effects */
            if ((t5 & 0xF) > (i & 0x7)) {
                /* Branch 1: different arithmetic pattern */
                arr1[i] = t5 + (arr2[i] >> 1);
                farr1[i] = f4 * 0.9f + (float)j;
                
                /* More operations in this branch */
                int t6 = arr1[i] ^ arr2[i];
                float f5 = farr1[i] * farr2[i];
                arr2[i] = t6 * 11;
                farr2[i] = f5 - f4;
            } else {
                /* Branch 2: alternative arithmetic pattern */
                arr1[i] = t5 - (arr2[i] << 1);
                farr1[i] = f4 / 1.1f - (float)j;
                
                /* Different operations in this branch */
                int t6 = arr1[i] | arr2[i];
                float f5 = farr1[i] + farr2[i];
                arr2[i] = t6 / 3;
                farr2[i] = f5 + f4;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extending live ranges) */
            if (i > 10) {
                arr1[0] += t1 / 17;  /* t1 computed at beginning of loop */
                farr1[0] += f1 * 0.01f;  /* f1 computed at beginning */
            }
            
            /* More operations using mixed types */
            int t7 = arr1[i] + (int)farr1[i];
            float f6 = (float)arr2[i] - farr2[i];
            
            /* Store results with complex indexing */
            arr1[(i + 1) % 32] ^= t7;
            farr1[(i + 2) % 32] += f6;
            
            /* Another volatile barrier */
            volatile int barrier = t7;
            (void)barrier;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = arr1[31] ^ j;
        arr2[0] = arr2[30] + j;
        farr1[0] = farr1[29] * 0.5f;
        farr2[0] = farr2[28] / 2.0f;
    }
    
    /* Final volatile store to prevent dead code elimination */
    volatile int final_result = arr1[0] + (int)farr1[0];
    (void)final_result;
}

int main(void) {
    int i;
    int checksum = 0;
    
    /* Call the target function multiple times with different iteration counts */
    for (i = 0; i < 5; i++) {
        stress_sched(100 + i * 50);
        checksum += i * 123;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
