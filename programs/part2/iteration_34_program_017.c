/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -c test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload logic is exercised */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int aux1, aux2;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 2;
    i3 = seed * 3 + 3;
    i4 = seed * 4 + 4;
    i5 = seed * 5 + 5;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long double values */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 1.2L;
    ld3 = (long double)i3 * 1.3L;
    ld4 = (long double)i4 * 1.4L;
    ld5 = (long double)i5 * 1.5L;
    
    /* Force x87 register usage with inline assembly */
    /* This should trigger secondary reloads for moving values into x87 stack */
    
    /* Example 1: x87 addition with 't' (top of stack) constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: x87 multiplication with mixed constraints */
    /* The 'u' constraint is second x87 register */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* Example 3: Multi-alternative constraint that may require secondary reload */
    /* "rm,t" means either memory/general register OR x87 top register */
    /* This is key for triggering secondary reload initialization */
    {
        long double result;
        int int_val = i6;
        
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (result)
            : "0" (ld5), "rm,t" (int_val)
            : "st(1)"
        );
        ld8 = result;
    }
    
    /* Example 4: Complex pattern with CRC32 builtin (fixed register usage) */
    /* CRC32 uses eax for accumulator */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        
        crc = __builtin_ia32_crc32qi(crc, data);
        i8 = (int)crc;
        
        /* Now mix this with x87 operation */
        ld9 = (long double)crc * 0.01L;
    }
    
    /* Example 5: Division with fixed register constraints */
    /* Division on x86 uses rax/eax and rdx/edx implicitly */
    {
        volatile int dividend = i1 + i2;
        volatile int divisor = i3 + 4;
        int quotient, remainder;
        
        asm volatile (
            "cltd\n\t"
            "idivl %2"
            : "=a" (quotient), "=d" (remainder)
            : "rm" (divisor), "a" (dividend), "d" (0)
        );
        
        i9 = quotient;
        i10 = remainder;
    }
    
    /* More x87 operations to increase register pressure */
    for (volatile int j = 0; j < 3; j++) {
        long double temp;
        
        /* Force reloads by using same variable in multiple asm statements */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp\n\t"
            "fstpt %0"
            : "=m" (temp)
            : "m" (ld6), "m" (ld7)
        );
        
        ld10 = temp + (long double)j;
        
        /* Another multi-alternative constraint example */
        {
            int choice = i4 + j;
            long double out;
            
            /* This asm has two alternatives for the third operand:
               "rm" (general register or memory) or "t" (x87 top) */
            asm volatile (
                "fldt %1\n\t"
                "faddp %%st(1), %%st"
                : "=t" (out)
                : "0" (ld8), "rm,t" (choice)
                : "st(1)"
            );
            
            ld11 = out;
        }
    }
    
    /* Mix MMX and x87 to create complex reload scenarios (if MMX available) */
    /* Note: MMX registers overlap with x87, creating interesting conflicts */
#ifdef __MMX__
    {
        volatile long double mmx_temp = ld9;
        /* This might trigger secondary reloads when moving between
           MMX and x87 register sets */
        asm volatile (
            "emms\n\t"  /* Clear MMX/x87 state */
            "fldt %1\n\t"
            "fstpt %0"
            : "=m" (mmx_temp)
            : "m" (ld10)
        );
        ld12 = mmx_temp;
    }
#endif
    
    /* Store results to globals to prevent elimination */
    global_results[global_index++] = ld6;
    global_results[global_index++] = ld7;
    global_results[global_index++] = ld8;
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    
    global_ints[global_index % 32] = i8;
    global_ints[(global_index + 1) % 32] = i9;
    global_ints[(global_index + 2) % 32] = i10;
}

/* Secondary test function with different patterns */
__attribute__((noinline, noipa))
void test_more_reloads(int seed) {
    volatile long double a, b, c;
    volatile int x, y;
    
    a = (long double)seed * 3.14159L;
    b = (long double)(seed + 1) * 2.71828L;
    x = seed * 7;
    y = seed * 11;
    
    /* This pattern specifically aims for secondary_in_reload/secondary_out_reload */
    /* by using a constraint that requires intermediate register */
    {
        long double result;
        
        /* The "rm,t" alternative constraint on x may force GCC to choose
           the 't' alternative and thus need secondary reloads to get
           an integer into x87 stack */
        asm volatile (
            "fildl %2\n\t"      /* Load integer into x87 stack */
            "fmulp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (result)
            : "t" (a), "rm,t" (x)
            : "st(1)"
        );
        c = result;
    }
    
    /* Another example with output reload */
    {
        long double out_val;
        int int_out;
        
        /* Complex pattern that might need secondary_out_icode */
        asm volatile (
            "fldt %1\n\t"
            "frndint\n\t"
            "fistpl %0\n\t"
            "fwait"
            : "=m" (int_out), "=t" (out_val)
            : "1" (b)
            : "st(1)"
        );
        
        global_ints[0] = int_out;
        global_results[0] = out_val;
    }
}

int main(int argc, char **argv) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_more_reloads(seed + 2);
    test_secondary_reloads(seed + 3);
    
    /* Use results to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 32 && i < global_index; i++) {
        checksum += (int)global_results[i] + global_ints[i];
    }
    
    return checksum & 0xFF;
}
