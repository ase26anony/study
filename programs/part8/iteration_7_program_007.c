/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * and reach uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent optimization of helper calls */
volatile int global_sink;

/* Non-inline helper function to clobber caller-saved registers */
__attribute__((noinline, optimize("O0")))
void use_registers(int a, int b, int c, int d, int e, int f,
                   float g, float h, double i, double j,
                   short k, short l, void *m, void *n) {
    /* Use all arguments to prevent dead argument elimination */
    global_sink = a + b + c + d + e + f;
    global_sink += (int)g + (int)h;
    global_sink += (int)i + (int)j;
    global_sink += k + l;
    global_sink += (long)m + (long)n;
    
    /* Additional computation to increase register pressure in caller */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
                     "r"(g), "r"(h), "r"(i), "r"(j), "r"(k), "r"(l),
                     "r"(m), "r"(n) : "memory");
}

int main(void) {
    /* Declare arrays to work with */
    int arr1[1024];
    int arr2[1024];
    float farr1[1024];
    double darr1[1024];
    short sarr1[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        farr1[i] = i * 1.5f;
        darr1[i] = i * 2.5;
        sarr1[i] = i & 0xFFFF;
    }
    
    /* Many scalar variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22;
    short s1 = 10, s2 = 20;
    void *p1 = &arr1[0], *p2 = &arr2[0];
    
    /* Main loop with high register pressure */
    for (int i = 0; i < 1024; i++) {
        /* Prevent hoisting with runtime condition */
        if (i % 7 == 0) {
            v1 = i * 2;      /* Different computation pattern */
        } else {
            v1 = i * 3;
        }
        
        /* Expensive-to-spill address computations (remat candidates) */
        int *ptr1 = &arr1[i];                    /* Base + index */
        int *ptr2 = &arr2[i & 0x3F];             /* Base + masked index */
        float *fptr1 = &farr1[i * 2 % 1024];     /* More complex index */
        double *dptr1 = &darr1[(i + 3) % 1024];  /* Offset index */
        short *sptr1 = &sarr1[(i * 5) % 1024];   /* Scaled index */
        
        /* Additional computations that are cheap to recompute */
        int offset1 = i * sizeof(int);           /* Scaled offset */
        int offset2 = (i + 1) * sizeof(float);   /* Different offset */
        float scale_factor = i * 0.25f;          /* Floating computation */
        double dscale = i * 0.125;               /* Double computation */
        
        /* Use all variables to keep them live */
        v2 = arr1[i] + v1;
        v3 = arr2[i] + v2;
        v4 = (int)farr1[i] + v3;
        v5 = (int)darr1[i] + v4;
        v6 = sarr1[i] + v5;
        
        f1 = farr1[i] * scale_factor;
        f2 = f1 + farr1[(i + 1) % 1024];
        f3 = f2 * 2.0f;
        
        d1 = darr1[i] * dscale;
        d2 = d1 + darr1[(i + 2) % 1024];
        
        s1 = sarr1[i] + (i & 0xFF);
        s2 = s1 + sarr1[(i + 3) % 1024];
        
        /* Update pointers */
        p1 = ptr1 + (i % 16);
        p2 = ptr2 + ((i + 1) % 16);
        
        /* Call function that clobbers caller-saved registers */
        /* Pass many arguments to increase register pressure */
        use_registers(v1, v2, v3, v4, v5, v6,
                     f1, f2, d1, d2,
                     s1, s2, p1, p2);
        
        /* Use computed values after call (forces save/restore or remat) */
        *ptr1 = v1 + v2;
        *ptr2 = v3 + v4;
        *fptr1 = f1 + f2;
        *dptr1 = d1 + d2;
        *sptr1 = s1 + s2;
        
        /* More computations using the address expressions */
        arr1[(i + offset1 / sizeof(int)) % 1024] = *ptr1 + offset1;
        arr2[(i + offset2 / sizeof(int)) % 1024] = *ptr2 + offset2;
        
        /* Conditional store based on runtime value */
        if (i % 13 == 0) {
            farr1[i] = f3 * 3.0f;
            darr1[i] = d2 * 1.5;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
        checksum += (unsigned long)farr1[i];
        checksum += (unsigned long)darr1[i];
        checksum += sarr1[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 1000);
}
