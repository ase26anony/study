/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_seed;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "eax", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Opaque operation to prevent optimization */
    *p1 = *p1 ^ 0x55;
    *p2 = *p2 + *p3;
    *p4 = *p4 - 1;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1)
                 : "eax", "ecx", "edx", "esi", "edi", "st", "st(1)", "st(2)", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    *f1 = *f1 * 2.0f;
    *f2 = *f2 / 3.0f;
    *i1 = *i1 | 0xFF;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + (int)time(NULL);
    global_seed = seed;
    
    int i, j;
    int checksum = 0;
    
    /* Loop to create multiple call sites */
    for (j = 0; j < 3; j++) {
        /* Declare MANY local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        float f1, f2, f3, f4, f5;
        
        /* Initialize with complex arithmetic to prevent optimization */
        v1 = seed + j * 11;
        v2 = seed ^ 0x1234;
        v3 = v1 * v2 + j;
        v4 = global_seed & 0xFF;
        v5 = v3 - v4;
        v6 = (v5 << 3) | (v5 >> 29);
        v7 = v6 * 1103515245 + 12345;
        v8 = v7 ^ v6;
        v9 = v8 % 1001;
        v10 = v9 * v8 + v7;
        
        v11 = v10 + 1;
        v12 = v11 * 2;
        v13 = v12 - v11;
        v14 = v13 | v12;
        v15 = v14 ^ v13;
        v16 = v15 + v14;
        v17 = v16 * 3;
        v18 = v17 / 2;
        v19 = v18 - v17;
        v20 = v19 ^ v18;
        
        f1 = (float)v1 * 0.1f;
        f2 = (float)v2 * 0.2f;
        f3 = f1 + f2;
        f4 = f3 * 1.5f;
        f5 = f4 - f3;
        
        /* Complex conditional to create different basic blocks */
        if ((seed + j) % 3 != 0) {
            /* High register pressure path - many variables live across call */
            
            /* More computations to keep variables live */
            v1 = v1 + v20;
            v2 = v2 * v19;
            v3 = v3 | v18;
            v4 = v4 ^ v17;
            v5 = v5 + v16;
            
            f1 = f1 * 2.0f + f5;
            f2 = f2 / 3.0f - f4;
            
            /* Call that clobbers registers - variables must be saved */
            clobber_callee(&v1, &v2, &v3, &v4);
            
            /* Use results after call to keep them live */
            v6 = v1 + v2;
            v7 = v3 * v4;
            v8 = v5 ^ v6;
            
            /* Another call with different register types */
            clobber_callee2(&f1, &f2, &v7);
            
            /* More computations */
            v9 = v7 + v8 + (int)f1;
            v10 = v9 * 2 - (int)f2;
            
            checksum += v10;
        } else {
            /* Lower pressure path */
            v1 = v1 + 1;
            v2 = v2 - 1;
            checksum += v1 + v2;
        }
        
        /* Another conditional inside the loop */
        if (j % 2 == 0) {
            /* Different call pattern */
            int t1 = v10 + v9;
            int t2 = v8 + v7;
            clobber_callee(&t1, &t2, &v6, &v5);
            checksum += t1 + t2;
        }
        
        /* Use volatile to prevent moving computations across calls */
        int volatile_read = global_seed;
        v11 = v11 + volatile_read;
        v12 = v12 ^ volatile_read;
        
        /* Mix in some I/O which can't be optimized away */
        if (j == 0) {
            int c = getchar();
            if (c != EOF) {
                v13 = v13 + c;
            }
        }
        
        /* Final computation using many variables */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        checksum += (int)(f1 + f2 + f3 + f4 + f5);
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional test case in separate function */
    test_function(argc);
    
    return checksum & 0xFF;
}

/* Another function with different register pressure pattern */
__attribute__((noinline, noclone))
int test_function(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    
    /* Initialize with param-dependent values */
    a1 = param * 1;
    a2 = param * 2;
    a3 = param * 3;
    a4 = param * 4;
    a5 = param * 5;
    a6 = param * 6;
    a7 = param * 7;
    a8 = param * 8;
    a9 = param * 9;
    a10 = param * 10;
    
    b1 = a1 ^ 0xAA;
    b2 = a2 + a1;
    b3 = a3 * a2;
    b4 = a4 | a3;
    b5 = a5 & a4;
    b6 = a6 - a5;
    b7 = a7 + a6;
    b8 = a8 ^ a7;
    b9 = a9 * a8;
    b10 = a10 / 9;
    
    /* Switch statement to create multiple basic blocks */
    switch (param % 4) {
        case 0:
            /* Call at end of this basic block */
            clobber_callee(&a1, &a2, &a3, &a4);
            /* Fall through */
        case 1:
            b1 = b1 + a1;
            b2 = b2 + a2;
            clobber_callee2((float*)&b1, (float*)&b2, &b3);
            break;
        case 2:
            b4 = b4 * 2;
            b5 = b5 / 2;
            /* No call here */
            break;
        default:
            clobber_callee(&a5, &a6, &a7, &a8);
            clobber_callee(&a9, &a10, &b9, &b10);
            break;
    }
    
    /* Use all variables to keep them live */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
}
