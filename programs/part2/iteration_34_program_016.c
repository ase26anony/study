/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed ^ 0x1234;
    i4 = seed - 100;
    i5 = seed + 200;
    i6 = seed * 3;
    i7 = seed / 2;
    i8 = seed % 17;
    i9 = seed << 3;
    i10 = seed >> 2;
    
    /* Initialize long double variables */
    ld1 = (long double)seed + 0.1;
    ld2 = (long double)seed * 2.2;
    ld3 = (long double)seed / 3.3;
    ld4 = (long double)seed - 4.4;
    ld5 = (long double)seed + 5.5;
    ld6 = (long double)seed * 6.6;
    ld7 = (long double)seed / 7.7;
    ld8 = (long double)seed - 8.8;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i11 = (int)(ts1 & 0xFFFFFFFF);
        i12 = (int)(ts1 >> 32);
        
        /* Create register pressure */
        i13 = i1 + i2;
        i14 = i3 * i4;
        
        ts2 = __builtin_ia32_rdtsc();
        i15 = (int)(ts2 & 0xFFFFFFFF);
        i16 = (int)(ts2 >> 32);
    }
    
    /* Force x87 operations with 't' (top of x87 stack) constraints */
    /* This should trigger secondary reloads for moving values into x87 regs */
    
    /* Simple x87 operation */
    asm volatile ("faddp %%st(1), %%st" 
                  : "=t" (ld9)
                  : "0" (ld1), "t" (ld2)
                  : "st(1)");
    
    /* More complex: mixing x87 with general registers */
    /* This asm has alternative constraints that may force secondary reloads */
    {
        long double temp = ld3;
        int int_val = i5;
        
        /* Multi-alternative constraint: "rm,t" - either memory/general reg OR x87 top */
        /* The compiler may choose the 't' alternative, requiring secondary reload */
        asm volatile ("fildl %2\n\t"
                      "faddp %%st(1), %%st"
                      : "=t" (ld10)
                      : "0" (temp), "rm,t" (int_val)
                      : "st(1)");
    }
    
    /* Another x87 operation using 'u' constraint (second x87 register) */
    {
        long double result;
        asm volatile ("fmulp %%st(2), %%st\n\t"
                      "fstpt %0"
                      : "=m" (result)
                      : "t" (ld4), "u" (ld5)
                      : "st", "st(1)");
        ld11 = result;
    }
    
    /* Chain multiple x87 operations to increase register pressure */
    asm volatile ("fldt %1\n\t"
                  "fldt %2\n\t"
                  "faddp\n\t"
                  "fstpt %0"
                  : "=m" (ld12)
                  : "m" (ld6), "m" (ld7));
    
    /* Mix with integer operations using CRC32 builtin (fixed register usage) */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i8);
        i17 = (int)crc;
    }
    
    /* Another complex asm with multiple outputs and x87 constraints */
    {
        long double out1, out2;
        int out_int;
        
        asm volatile ("fldt %3\n\t"
                      "fldt %4\n\t"
                      "faddp %%st(1), %%st\n\t"
                      "fistpl %2\n\t"
                      "fstpt %1\n\t"
                      "fstpt %0"
                      : "=m" (out1), "=m" (out2), "=m" (out_int)
                      : "m" (ld8), "m" (ld1)
                      : "st", "st(1)");
        
        ld13 = out1;
        ld14 = out2;
        i18 = out_int;
    }
    
    /* Loop to keep variables live and increase optimization complexity */
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* More x87 operations in loop */
        asm volatile ("fldt %1\n\t"
                      "fsin\n\t"
                      "fstpt %0"
                      : "=m" (ld15)
                      : "m" (ld2));
        
        /* Use the result */
        i19 = i19 + (int)ld15;
    }
    
    /* Store results to globals to prevent elimination */
    global_results[global_index] = ld9;
    global_results[global_index + 1] = ld10;
    global_results[global_index + 2] = ld11;
    global_results[global_index + 3] = ld12;
    global_results[global_index + 4] = ld13;
    global_results[global_index + 5] = ld14;
    global_results[global_index + 6] = ld15;
    
    global_ints[global_index] = i17;
    global_ints[global_index + 1] = i18;
    global_ints[global_index + 2] = i19;
    
    global_index = (global_index + 7) % 32;
}

int main(int argc, char **argv) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    test_secondary_reloads(seed + 3);
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
