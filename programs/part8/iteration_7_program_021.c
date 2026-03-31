/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC RTL backend
 * Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of function calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, short c, float d, double e, 
                int *p1, int *p2, float *p3, double *p4, char *p5) {
    /* Force register pressure by using all arguments */
    global_volatile = a + b + c + (int)d + (int)e;
    if (p1) global_volatile += *p1;
    if (p2) global_volatile += *p2;
    if (p3) global_volatile += (int)*p3;
    if (p4) global_volatile += (int)*p4;
    if (p5) global_volatile += *p5;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to create register pressure */
    int arr1[1024];
    int arr2[1024];
    short arr3[1024];
    float arr4[1024];
    double arr5[1024];
    char arr6[1024];
    int arr7[1024];
    float arr8[1024];
    double arr9[1024];
    int arr10[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        arr3[i] = (short)(i * 3);
        arr4[i] = i * 1.5f;
        arr5[i] = i * 2.5;
        arr6[i] = (char)(i & 0x7F);
        arr7[i] = i * 7;
        arr8[i] = i * 3.14f;
        arr9[i] = i * 2.71828;
        arr10[i] = i * 11;
    }
    
    /* Many scalar variables to increase register pressure */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    short var6 = 6, var7 = 7;
    float var8 = 8.0f, var9 = 9.0f;
    double var10 = 10.0, var11 = 11.0;
    int var12 = 12;
    
    /* Complex loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Create many address computations that are candidates for rematerialization */
        int *ptr1 = &arr1[i];                    /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];             /* Masked index */
        short *ptr3 = &arr3[(i * 3) % 1024];     /* Scaled index */
        float *ptr4 = &arr4[i];                  /* Float array */
        double *ptr5 = &arr5[i];                 /* Double array */
        char *ptr6 = &arr6[i % 512];             /* Different size */
        int *ptr7 = &arr7[1023 - i];             /* Reverse index */
        float *ptr8 = &arr8[(i * 2) % 1024];     /* Scaled float */
        double *ptr9 = &arr9[i];                 /* Another double */
        int *ptr10 = &arr10[(i + 1) % 1024];     /* Offset index */
        
        /* More address computations with different modes */
        int offset1 = i * sizeof(int);           /* Byte offset */
        int offset2 = (i & 0xFF) * 4;            /* Masked and scaled */
        short offset3 = (short)(i * 2);          /* Short offset */
        float offset_f = i * 1.0f;               /* Float offset */
        double offset_d = i * 1.0;               /* Double offset */
        
        /* Conditional to prevent hoisting */
        if (i % 7 == 0) {
            var1 = i * 2;
            var8 = i * 3.0f;
        } else if (i % 13 == 0) {
            var2 = i * 3;
            var10 = i * 4.0;
        }
        
        /* Use all variables to keep them live */
        var3 = var1 + var2;
        var4 = var3 * 2;
        var5 = var4 - var1;
        var6 = (short)(var5 & 0xFFFF);
        var7 = (short)(var6 + 1);
        var9 = var8 + 1.0f;
        var11 = var10 + 1.0;
        var12 = var3 + var4;
        
        /* Function call with many arguments - forces register pressure */
        use_values(var1, var2, var6, var8, var10,
                  ptr1, ptr2, ptr4, ptr5, ptr6);
        
        /* More computations after call using rematerialization candidates */
        int idx1 = (i * 4) % 1024;               /* Candidate for remat */
        int idx2 = (i + 8) % 1024;               /* Another candidate */
        float idx_f = i * 2.0f;                  /* Float candidate */
        double idx_d = i * 3.0;                  /* Double candidate */
        
        /* Use the computed addresses and indices */
        *ptr1 = var1 + idx1;
        *ptr2 = var2 + idx2;
        *ptr3 = (short)(var6 + i);
        *ptr4 = var8 + idx_f;
        *ptr5 = var10 + idx_d;
        
        /* Conditional store based on runtime value */
        if (i % 17 == 0) {
            *ptr7 = var3;
            *ptr8 = var9;
            *ptr9 = var11;
            *ptr10 = var12;
        }
        
        /* Update some variables to create data dependencies */
        var1 = (var1 + 1) % 100;
        var2 = (var2 + 2) % 100;
        var8 = var8 + 0.5f;
        var10 = var10 + 0.25;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)arr4[i] + (int)arr5[i];
        checksum += arr6[i] + arr7[i] + (int)arr8[i] + (int)arr9[i] + arr10[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (int)(checksum % 1000);
}
