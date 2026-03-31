/* reload_secondary_test.c
 * Test program to trigger secondary reloads in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer reload_secondary_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure reload happens at call boundaries */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Initialize long doubles */
    ld1 = (long double)(seed + 1) / 3.14159265358979323846L;
    ld2 = (long double)(seed + 2) / 2.71828182845904523536L;
    ld3 = (long double)(seed + 3) / 1.41421356237309504880L;
    ld4 = (long double)(seed + 4) / 1.61803398874989484820L;
    ld5 = (long double)(seed + 5) / 3.14159265358979323846L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i6 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i7 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 register usage with inline asm */
    /* This should trigger secondary reloads for moving between x87 and general regs */
    
    /* Example 1: Simple x87 operation with 't' constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* load ld1 into st(0) */
        "fldt %2\n\t"           /* load ld2 into st(0), ld1 moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop st(0) */
        "fstpt %0"
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Example 2: Mixed constraints that may require secondary reload */
    /* 't' = top of x87 stack, 'u' = second x87 register, 'r' = general register */
    {
        long double temp_ld = ld3;
        int temp_int = i3;
        
        /* This asm has multiple alternatives for the third operand:
         * "rm,t" means either memory/general register OR x87 top register
         * GCC may choose the 't' alternative, requiring secondary reload
         */
        asm volatile (
            "fldt %1\n\t"       /* load temp_ld into st(0) */
            "fildl %2\n\t"      /* load temp_int into st(0), temp_ld moves to st(1) */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld7)
            : "m" (temp_ld), "rm,t" (temp_int)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* Example 3: More complex pattern with output in x87 register */
    {
        long double result;
        
        /* Output constraint '=t' means result must be in x87 top register */
        /* Input constraint '0' means same as output (in x87 register) */
        /* Second input has alternative constraints including 't' */
        asm volatile (
            "faddp %%st, %%st(1)"
            : "=t" (result)
            : "0" (ld4), "t,rm" (ld5)
            : "st(1)"
        );
        
        ld8 = result;
    }
    
    /* Example 4: CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        
        /* __builtin_ia32_crc32qi uses eax for accumulator */
        crc = __builtin_ia32_crc32qi(crc, data);
        i8 = (int)crc;
        
        /* Now mix with x87 operation to create register pressure */
        asm volatile (
            "fildl %1\n\t"
            "fstpt %0"
            : "=m" (ld9)
            : "rm" (i8)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* Create more register pressure with additional operations */
    for (volatile int loop = 0; loop < 3; loop++) {
        /* Complex asm with multiple x87 registers */
        long double a = ld1 + (long double)loop;
        long double b = ld2 + (long double)loop;
        long double c;
        
        asm volatile (
            "fldt %2\n\t"   /* b -> st(0) */
            "fldt %1\n\t"   /* a -> st(0), b -> st(1) */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (c)
            : "m" (a), "m" (b)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        ld10 = c;
        
        /* Integer operation to mix register classes */
        i9 = i1 + i2 + loop;
        
        /* Another asm with mixed constraints */
        {
            int src = i9;
            long double dst;
            
            asm volatile (
                "fildl %1\n\t"
                "fchs\n\t"      /* change sign */
                "fstpt %0"
                : "=m" (dst)
                : "rm,t" (src)  /* Alternative constraints may trigger secondary reload */
                : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
            );
            
            ld11 = dst;
        }
    }
    
    /* Store results to global arrays to prevent optimization */
    global_results[result_index] = ld6;
    global_ints[result_index] = i6;
    result_index = (result_index + 1) % 20;
    
    global_results[result_index] = ld7;
    global_ints[result_index] = i7;
    result_index = (result_index + 1) % 20;
    
    global_results[result_index] = ld8;
    global_ints[result_index] = i8;
    result_index = (result_index + 1) % 20;
    
    global_results[result_index] = ld9;
    global_ints[result_index] = i9;
    result_index = (result_index + 1) % 20;
    
    global_results[result_index] = ld10;
    global_ints[result_index] = i10;
    result_index = (result_index + 1) % 20;
    
    global_results[result_index] = ld11;
    global_ints[result_index] = i11;
    result_index = (result_index + 1) % 20;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds to exercise different paths */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 20; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
