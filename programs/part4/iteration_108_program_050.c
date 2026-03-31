/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -march=i386 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to prevent optimization */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : 
        "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Do some work to prevent the call from being eliminated */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
int __attribute__((noinline, noclone)) clobber_callee2(int a, int b, int c, int d, 
                                                      int e, int f, int g, int h) {
    int result;
    #ifdef __i386__
    asm volatile("movl %%eax, %0\n\t"
                 "addl $1, %%eax\n\t"
                 "movl %%ebx, %%eax\n\t"
                 "addl $2, %%ebx\n\t"
                 : "=r"(result) : "a"(a), "b"(b), "c"(c), "d"(d) : 
                   "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    result = a + b + c + d + e + f + g + h;
    #endif
    return result;
}

int main(int argc, char **argv) {
    /* Use argc to create input-dependent but deterministic behavior */
    volatile int seed = argc > 1 ? atoi(argv[1]) : (int)time(NULL);
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent optimization */
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed ^ 0x1234;
    v4 = seed - 100;
    v5 = (seed << 3) | (seed >> 29);
    v6 = ~seed;
    v7 = seed * seed;
    v8 = seed % 17;
    v9 = v1 + v2;
    v10 = v3 * v4;
    v11 = v5 ^ v6;
    v12 = v7 - v8;
    v13 = v9 + v10;
    v14 = v11 * v12;
    v15 = v13 ^ v14;
    v16 = global_seed + seed;
    v17 = v1 * v16;
    v18 = v2 + v17;
    v19 = v3 * v18;
    v20 = v4 + v19;
    v21 = v5 * v20;
    v22 = v6 + v21;
    v23 = v7 * v22;
    v24 = v8 + v23;
    v25 = v9 * v24;
    v26 = v10 + v25;
    v27 = v11 * v26;
    v28 = v12 + v27;
    v29 = v13 * v28;
    v30 = v14 + v29;
    
    /* Create multiple call sites with different register pressure patterns */
    for (int i = 0; i < 3; i++) {
        /* Complex conditional to create different basic blocks */
        if ((seed + i) % 3 == 0) {
            /* HIGH REGISTER PRESSURE PATH - many variables live across call */
            
            /* More computations to keep variables in registers */
            v1 = v1 + v2 + v3;
            v4 = v4 * v5 - v6;
            v7 = v7 ^ v8 | v9;
            v10 = v10 + v11 * v12;
            v13 = v13 - v14 + v15;
            v16 = v16 * v17 / (v18 + 1);
            v19 = v19 ^ v20 & v21;
            v22 = v22 + v23 - v24;
            v25 = v25 * v26 % 31;
            v27 = v27 + v28 ^ v29;
            v30 = v30 * 3 + 7;
            
            /* Call that clobbers registers - many variables are live */
            clobber_callee(&v1, &v2, &v3, &v4);
            
            /* Use results after call to keep them live */
            v5 = v1 + v2;
            v6 = v3 * v4;
            v7 = v5 ^ v6;
            v8 = v7 - v8;
            
        } else if ((seed + i) % 3 == 1) {
            /* MEDIUM REGISTER PRESSURE PATH */
            
            /* Different computation pattern */
            v9 = v9 + v10 - v11;
            v12 = v12 * v13 + v14;
            v15 = v15 ^ v16 | v17;
            
            /* Another call with different arguments */
            int result = clobber_callee2(v9, v10, v11, v12, v13, v14, v15, v16);
            
            v18 = v18 + result;
            v19 = v19 * result;
            
        } else {
            /* LOW PRESSURE PATH - simpler computation, no call */
            v20 = v20 + 1;
            v21 = v21 * 2;
            v22 = v22 ^ 0x55;
            v23 = v23 - 10;
        }
        
        /* Mix variables across iterations to create data dependencies */
        v1 = v1 + v30;
        v2 = v2 ^ v29;
        v3 = v3 * v28;
        v4 = v4 + v27;
        
        /* Read from volatile global to prevent reordering */
        seed = global_seed + i;
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    /* Add some I/O to create side effects */
    printf("Checksum: %d (seed was %d)\n", checksum, seed);
    
    /* Another conditional with call at block end */
    volatile int flag = checksum % 2;
    if (flag) {
        /* More computations before call at block end */
        int t1 = v1 * v2 + v3;
        int t2 = v4 ^ v5 | v6;
        int t3 = v7 - v8 * v9;
        int t4 = v10 + v11 ^ v12;
        
        /* Call at what might be BB_END before insertion */
        clobber_callee(&t1, &t2, &t3, &t4);
        
        v13 = t1 + t2;
        v14 = t3 * t4;
    } else {
        v13 = v13 + 100;
        v14 = v14 * 200;
    }
    
    printf("Final values: v13=%d, v14=%d\n", v13, v14);
    
    return checksum & 0xFF;
}
