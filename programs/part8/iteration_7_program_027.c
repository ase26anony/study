/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and reach uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, noipa))
void use_values(int a, int b, short c, float d, double e, 
                int *p1, int *p2, float *p3, double *p4, char *p5) {
    /* Complex enough to use multiple registers */
    global_volatile_sink = a + b + c + (int)d + (int)e;
    if (p1) global_volatile_sink += *p1;
    if (p2) global_volatile_sink += *p2;
    if (p3) global_volatile_sink += (int)*p3;
    if (p4) global_volatile_sink += (int)*p4;
    if (p5) global_volatile_sink += *p5;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Declare many arrays to increase register pressure */
    int arr1[1024], arr2[1024], arr3[1024];
    float farr1[1024], farr2[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1024 - i;
        arr3[i] = i * 2;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 3.14159;
        sarr1[i] = i & 0x7FFF;
        carr1[i] = i & 0xFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    short v6 = 6, v7 = 7, v8 = 8;
    float v9 = 9.0f, v10 = 10.0f;
    double v11 = 11.0, v12 = 12.0;
    int *v13 = &arr1[0];
    float *v14 = &farr1[0];
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Expensive to spill, cheap to recompute */
            v9 = i * 1.7f;
        } else if (i % 13 == 0) {
            v2 = i * 3;
            v10 = i * 2.3f;
        }
        
        /* Compute addresses that are cheap to rematerialize */
        int *ptr1 = &arr1[i];           /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];    /* Base + masked index */
        float *ptr3 = &farr1[i];
        double *ptr4 = &darr1[i];
        char *ptr5 = &carr1[i];
        
        /* Complex addressing modes that might be rematerialized */
        int offset1 = i * sizeof(int);      /* Computable offset */
        int *ptr6 = (int*)((char*)arr3 + offset1);
        
        float offset2 = i * 1.5f;
        float *ptr7 = &farr2[(int)offset2 & 0x3FF];
        
        /* Use all variables to keep them live */
        v3 = v1 + v2;
        v4 = v3 * v5;
        v6 = (short)(v4 & 0x7FFF);
        v7 = v6 + 1;
        v8 = v7 * 2;
        v11 = v9 + v10;
        v12 = v11 * 2.0;
        
        /* Non-inline call clobbers caller-saved registers */
        use_values(v1, v2, v6, v9, v11, 
                  ptr1, ptr2, ptr3, ptr4, ptr5);
        
        /* Use values after call - they need to be preserved/rematerialized */
        arr1[i] += v3 + v4;
        arr2[i & 0x3F] += v6 + v7 + v8;
        farr1[i] += v9 + v10;
        darr1[i] += v12;
        
        /* More computations with the pointers */
        if (ptr6) *ptr6 += i;
        if (ptr7) *ptr7 += v9;
        
        /* Conditional store based on runtime value */
        if (i % 19 == 0) {
            v13 = ptr1;
            v14 = ptr3;
        } else if (i % 23 == 0) {
            v13 = ptr2;
            v14 = ptr7;
        }
        
        /* Use the pointer variables */
        if (v13) *v13 += 1;
        if (v14) *v14 += 1.0f;
        
        /* Loop-carried dependency to prevent vectorization */
        v5 = v4 ^ i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + 
                   (unsigned)farr1[i] + (unsigned)darr1[i] +
                   sarr1[i] + carr1[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return (checksum > 0) ? 0 : 1;
}
