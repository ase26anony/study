/* reload_secondary_test.c
 * Test program to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer reload_secondary_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15, ld16;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed - 456;
    i5 = seed * seed;
    i6 = seed / 3;
    i7 = seed % 7;
    i8 = seed | 0xFF00;
    i9 = seed & 0x0F0F;
    i10 = ~seed;
    
    /* Initialize long doubles with conversions from integers */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 2.2L;
    ld3 = (long double)i3 * 3.3L;
    ld4 = (long double)i4 * 4.4L;
    ld5 = (long double)i5 * 5.5L;
    ld6 = (long double)i6 * 6.6L;
    ld7 = (long double)i7 * 7.7L;
    ld8 = (long double)i8 * 8.8L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i11 = (int)(tsc & 0xFFFFFFFF);
        i12 = (int)(tsc >> 32);
    }
    
    /* Force x87 register usage with inline assembly */
    /* First, load values into x87 stack */
    asm volatile ("fldt %1\n\t"
                  "fstpt %0"
                  : "=m" (ld9)
                  : "m" (ld1)
                  : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Complex asm with multiple alternatives to trigger secondary reloads */
    /* The "rm,t" constraint may force secondary reload for integer operand */
    asm volatile ("fildl %2\n\t"
                  "faddp %%st, %%st(1)\n\t"
                  "fstpt %0"
                  : "=m" (ld10)
                  : "0" (ld2), "rm,t" (i3)
                  : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Another asm with x87 stack manipulation */
    asm volatile ("fldt %1\n\t"
                  "fldt %2\n\t"
                  "fmulp %%st, %%st(1)\n\t"
                  "fistpl %0"
                  : "=m" (i13)
                  : "t" (ld3), "u" (ld4)
                  : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Mix x87 and general registers with CRC32 builtin */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i4);
        i14 = (int)crc;
    }
    
    /* More x87 operations to keep pressure */
    asm volatile ("fldt %1\n\t"
                  "fldt %2\n\t"
                  "fdivrp %%st, %%st(1)\n\t"
                  "fstpt %0"
                  : "=m" (ld11)
                  : "t" (ld5), "u" (ld6)
                  : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Use "a" constraint (rax/eax) which may need secondary reload */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl $100, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r" (i15)
                  : "rm" (i5)
                  : "eax");
    
    /* Complex pattern: x87 operation with memory output and mixed constraints */
    ld12 = ld7;
    asm volatile ("fldt %1\n\t"
                  "fldt %2\n\t"
                  "fsubrp %%st, %%st(1)\n\t"
                  "fstpt %0"
                  : "=m" (ld12)
                  : "t" (ld7), "m,t" (ld8)
                  : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Loop to increase register pressure and prevent optimization */
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* Mix operations in loop */
        asm volatile ("fldt %1\n\t"
                      "fsin\n\t"
                      "fstpt %0"
                      : "=m" (ld13)
                      : "m" (ld9)
                      : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        
        /* Use "c" constraint (rcx/ecx) for shift */
        asm volatile ("movl %1, %%ecx\n\t"
                      "movl %2, %%eax\n\t"
                      "shrl %%cl, %%eax\n\t"
                      "movl %%eax, %0"
                      : "=r" (i16)
                      : "c" (i6 & 31), "r" (i7)
                      : "eax", "ecx");
    }
    
    /* Store results to globals to prevent elimination */
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld13;
    
    global_ints[global_index % 32] = i13;
    global_ints[(global_index + 1) % 32] = i14;
    global_ints[(global_index + 2) % 32] = i15;
    global_ints[(global_index + 3) % 32] = i16;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Compute checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 32 && i < global_index; i++) {
        checksum += (int)global_results[i];
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
