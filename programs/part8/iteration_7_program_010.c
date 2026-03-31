/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and exercise the uncovered lines in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_volatile = 0;

/* Non-inline helper function that clobbers caller-saved registers */
__attribute__((noinline, optimize("no-ipa")))
void helper_func(int a, int b, int *ptr1, int *ptr2, 
                 float f1, float f2, double d1, short s1)
{
    /* Simple operations that use all arguments */
    global_volatile = a + b;
    if (ptr1) global_volatile += *ptr1;
    if (ptr2) global_volatile += *ptr2;
    
    /* Use floating point args to ensure FP registers are used */
    global_volatile += (int)f1 + (int)f2 + (int)d1 + s1;
}

/* Another non-inline helper with different signature */
__attribute__((noinline, optimize("no-ipa")))
int helper_func2(long l1, long l2, char *cptr, unsigned short us)
{
    int result = (int)(l1 ^ l2);
    if (cptr) result += *cptr;
    result += us;
    global_volatile += result;
    return result;
}

int main(void)
{
    /* Declare many arrays to increase register pressure */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    char carr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i;
        arr2[i] = 1023 - i;
        farr1[i] = i * 0.5f;
        darr1[i] = i * 0.25;
        sarr1[i] = i % 256;
        carr1[i] = i % 128;
    }
    
    /* Declare many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float fv1 = 1.1f, fv2 = 2.2f, fv3 = 3.3f;
    double dv1 = 10.1, dv2 = 20.2;
    long lv1 = 100, lv2 = 200, lv3 = 300;
    unsigned short us1 = 400, us2 = 500;
    char *cptr1 = carr1, *cptr2 = carr1 + 512;
    
    int checksum = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Expensive to spill, cheap to recompute */
            v2 = i * 3;
        } else if (i % 13 == 0) {
            v3 = i * 4;
            v4 = i * 5;
        }
        
        /* Compute addresses - these are rematerialization candidates */
        int *ptr1 = &arr1[i];                /* Base + offset */
        int *ptr2 = &arr2[i & 0x3F];         /* More complex addressing */
        float *fptr1 = &farr1[i % 512];      /* Different type */
        double *dptr1 = &darr1[i];           /* Double type */
        short *sptr1 = &sarr1[i];            /* Short type */
        
        /* More address computations with different modes */
        char *cptr_local = &carr1[i % 256];
        int *ptr3 = &arr1[(i * 3) % 1024];
        
        /* Use computed addresses in helper calls */
        helper_func(v1 + v2, v3 + v4, ptr1, ptr2, 
                   fv1 + i * 0.1f, fv2 + i * 0.2f, 
                   dv1 + i * 0.01, sarr1[i]);
        
        /* Another helper call with different types */
        int helper_result = helper_func2(lv1 + i, lv2 + i * 2, 
                                        cptr_local, us1 + (i % 100));
        
        /* Use results in conditional stores - keeps values live */
        if (i % 3 == 0) {
            arr1[i] += v1 + v2 + helper_result;
            *ptr2 = v3 + v4;
        } else if (i % 5 == 0) {
            arr2[i] += v5 + v6;
            *fptr1 = fv3 + i * 0.3f;
        }
        
        /* More operations to increase pressure */
        v5 = v1 * v2;        /* Expensive computation */
        v6 = v3 * v4;
        fv1 = fv2 * 1.1f;
        dv1 = dv2 * 1.01;
        
        /* Use all variables to keep them live */
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)darr1[i] 
                  + sarr1[i] + carr1[i] + v1 + v2 + v3 + v4 + v5 + v6 
                  + (int)fv1 + (int)fv2 + (int)fv3 + (int)dv1 + (int)dv2
                  + helper_result;
        
        /* Complex condition to prevent optimization */
        if ((i % 17) == 0) {
            lv1 = lv2 * 2;
            lv2 = lv3 * 3;
            us1 = us2 + i;
        }
        
        /* Use pointers in stores */
        *dptr1 = dv1;
        *sptr1 = (short)(v1 + v2);
        
        /* Another address computation */
        int *ptr4 = &arr2[(i * 7) % 1024];
        *ptr4 = checksum % 1000;
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = checksum % 1000000;
    
    /* Use all arrays and variables one more time */
    for (int i = 0; i < 10; i++) {
        final_result += arr1[i] + arr2[i] + (int)farr1[i] 
                      + (int)darr1[i] + sarr1[i] + carr1[i];
    }
    
    printf("Result: %d\n", final_result);
    return final_result;
}
