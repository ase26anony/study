/* reload_secondary_test.c
 * Test program to cover secondary reload initialization in GCC's reload pass.
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer reload_secondary_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long double variables */
    ld1 = (long double)i1 / 3.1415926535L;
    ld2 = (long double)i2 / 2.7182818284L;
    ld3 = (long double)i3 / 1.4142135623L;
    ld4 = (long double)i4 / 1.7320508075L;
    
    /* Force x87 operations with explicit register constraints */
    
    /* Pattern 1: Basic x87 operation with "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %2\n\t"           /* Load operand2 to st(0) */
        "fldt %1\n\t"           /* Load operand1 to st(0), operand2 moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(0) + st(1), pop stack */
        "fstpt %0"
        : "=m" (ld5)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Pattern 2: Mixed constraints - "t" for x87, "rm" for general reg/memory */
    /* This may trigger secondary reload for the integer operand */
    {
        long double result;
        int int_val = i3 + 42;
        
        asm volatile (
            "fldt %1\n\t"       /* Load ld3 to st(0) */
            "fildl %2\n\t"      /* Load int_val to st(0), ld3 moves to st(1) */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld3), "m" (int_val)
            : "st", "st(1)"
        );
        ld6 = result;
    }
    
    /* Pattern 3: Multiple alternative constraints with x87 register */
    /* The "rm,t" alternative may force secondary reload setup */
    {
        long double temp = ld4;
        int alt_val = i4;
        
        asm volatile (
            "fldt %1\n\t"
            "fildl %k2\n\t"     /* Use %k2 for 32-bit register */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld7)
            : "m" (temp), "r" (alt_val)  /* "r" constraint - may need secondary reload */
            : "st", "st(1)"
        );
    }
    
    /* Pattern 4: Output in x87 register with input in memory */
    /* This forces moving the result from x87 to memory via secondary reload */
    {
        long double out_val;
        asm volatile (
            "fldt %1\n\t"
            "fsqrt\n\t"
            : "=t" (out_val)    /* Output in x87 top register */
            : "m" (ld5)
        );
        ld8 = out_val;
    }
    
    /* Pattern 5: Complex pattern with two x87 registers ("t" and "u") */
    {
        long double a = ld2, b = ld3;
        asm volatile (
            "fldt %2\n\t"   /* b -> st(0) */
            "fldt %1\n\t"   /* a -> st(0), b -> st(1) */
            "fsubrp %%st, %%st(1)\n\t"  /* st(1) = st(1) - st(0), pop */
            "fstpt %0"
            : "=m" (ld9)
            : "m" (a), "m" (b)
            : "st", "st(1)"
        );
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    /* This creates additional register class pressure */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i5);
        i8 = (int)crc;
    }
    
    /* More operations to increase register pressure */
    for (volatile int j = 0; j < 3; j++) {
        /* Pattern with input/output reloads */
        long double src = ld6 + (long double)j;
        long double dst;
        
        asm volatile (
            "fldt %1\n\t"
            "fchs\n\t"          /* Change sign */
            "fstpt %0"
            : "=m" (dst)
            : "m" (src)
            : "st"
        );
        
        ld10 = dst;
        
        /* Integer operation that might conflict with x87 */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (i9)
            : "r" (i6), "r" (i7)
            : "eax"
        );
    }
    
    /* Pattern 6: Division with fixed register constraint ("a" for eax) */
    /* Mixed with x87 operations to create complex reload requirements */
    {
        int dividend = i8;
        int divisor = i9 ? i9 : 1;
        int quotient;
        
        asm volatile (
            "movl %1, %%eax\n\t"
            "cltd\n\t"          /* Sign extend eax to edx:eax */
            "idivl %2\n\t"
            "movl %%eax, %0"
            : "=r" (quotient)
            : "r" (dividend), "r" (divisor)
            : "eax", "edx"
        );
        i10 = quotient;
    }
    
    /* Final mixing of all values */
    ld11 = ld1 + ld2 + ld3 + ld4 + ld5 + ld6 + ld7 + ld8 + ld9 + ld10;
    ld12 = ld11 * (long double)i10;
    
    /* Store to globals to prevent optimization */
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
    global_results[10] = ld11;
    global_results[11] = ld12;
    
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
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code runs */
    long double sum = 0.0L;
    for (int i = 0; i < 12; i++) {
        sum += global_results[i];
    }
    
    int isum = 0;
    for (int i = 0; i < 10; i++) {
        isum += global_ints[i];
    }
    
    printf("Checksum: long double sum = %Lf, int sum = %d\n", sum, isum);
    
    return 0;
}
