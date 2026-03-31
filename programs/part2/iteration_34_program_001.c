/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic is exercised */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * 3 + 5;
    i5 = seed / 2 + 7;
    i6 = seed + 11;
    i7 = seed * 5 - 3;
    i8 = seed ^ 0x5678;
    i9 = seed + 13;
    i10 = seed * 7 + 17;
    i11 = seed / 3 + 19;
    i12 = seed + 23;
    i13 = seed * 11 - 7;
    i14 = seed ^ 0x9ABC;
    i15 = seed + 29;
    
    /* Initialize long double variables */
    ld1 = (long double)i1 / 3.0L;
    ld2 = (long double)i2 / 7.0L;
    ld3 = (long double)i3 / 11.0L;
    ld4 = (long double)i4 / 13.0L;
    ld5 = (long double)i5 / 17.0L;
    ld6 = (long double)i6 / 19.0L;
    ld7 = (long double)i7 / 23.0L;
    ld8 = (long double)i8 / 29.0L;
    ld9 = (long double)i9 / 31.0L;
    ld10 = (long double)i10 / 37.0L;
    ld11 = (long double)i11 / 41.0L;
    ld12 = (long double)i12 / 43.0L;
    ld13 = (long double)i13 / 47.0L;
    ld14 = (long double)i14 / 53.0L;
    ld15 = (long double)i15 / 59.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 register usage with inline assembly */
    /* Using "t" constraint (top of x87 stack) and "u" constraint (second x87 register) */
    
    /* First x87 operation: fadd */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "u" (ld2)
        : "st(1)"
    );
    
    /* Second x87 operation: fmul */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* Mixed operation with integer input - may require secondary reload */
    /* The "rm,t" alternative constraint: either memory/register OR x87 top */
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld5)
        : "0" (ld5), "rm,t" (i5)
        : "st(1)"
    );
    
    /* Another mixed operation with different integer */
    asm volatile (
        "fildl %2\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld6), "rm,t" (i6)
        : "st(1)"
    );
    
    /* Complex chain of operations to increase register pressure */
    for (volatile int j = 0; j < 3; j++) {
        /* Use CRC32 builtin which has fixed register constraints */
        i7 = __builtin_ia32_crc32qi(i7, (unsigned char)i8);
        
        /* More x87 operations */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp\n\t"
            "fstpt %0"
            : "=m" (ld7)
            : "m" (ld8), "m" (ld9)
        );
        
        /* Operation requiring value in specific register class */
        /* Division operation that might require rax/rdx */
        {
            volatile int64_t dividend = i9;
            volatile int32_t divisor = i10;
            volatile int64_t quotient;
            
            asm volatile (
                "mov %1, %%rax\n\t"
                "cqo\n\t"
                "idivl %2"
                : "=a" (quotient), "=d" (i11)
                : "a" (dividend), "rm" (divisor)
                : "cc"
            );
            i9 = (int)quotient;
        }
        
        /* Another x87 operation with memory operand */
        asm volatile (
            "fldt %1\n\t"
            "fchs\n\t"
            "fstpt %0"
            : "=m" (ld10)
            : "m" (ld11)
        );
    }
    
    /* Store results to prevent optimization */
    global_results[0] = ld1;
    global_results[1] = ld2;
    global_results[2] = ld3;
    global_results[3] = ld4;
    global_results[4] = ld5;
    global_results[5] = ld6;
    global_results[6] = ld7;
    global_results[7] = ld8;
    global_results[8] = ld9;
    global_results[9] = ld10;
    
    global_ints[0] = i1;
    global_ints[1] = i2;
    global_ints[2] = i3;
    global_ints[3] = i4;
    global_ints[4] = i5;
    global_ints[5] = i6;
    global_ints[6] = i7;
    global_ints[7] = i8;
    global_ints[8] = i9;
    global_ints[9] = i10;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 10; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code runs */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
