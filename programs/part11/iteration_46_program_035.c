/* test_resource_marking.c
 * Designed to generate RTL patterns that exercise uncovered lines in resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP(expr) asm volatile("" : "+r"(expr))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
static int test_bitfields(void) {
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
    } bf = {0};
    
    register unsigned int a = 0xABCD;
    register unsigned int b = 0x1234;
    register unsigned int c = 0x5678;
    
    KEEP(a); KEEP(b); KEEP(c);
    
    /* Multiple bitfield assignments with complex expressions */
    bf.field4 = (a & 0xF) + (b & 0x7);          /* Should generate ZERO_EXTRACT */
    bf.field8 = ((a >> 4) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex bitfield store */
    bf.field12 = __builtin_popcount(c) & 0xFFF; /* Builtin with bitfield */
    
    /* Read back to prevent elimination */
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG operations through type narrowing */
static int test_subreg(void) {
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = 0x13579BDF;
    
    KEEP(r1); KEEP(r2); KEEP(r3);
    
    /* Explicit narrowing casts - should generate SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* int -> short */
    vs2 = (short)(r1 + r2);             /* expression then narrowing */
    vs3 = (short)((r1 & 0xFFFF) | ((r2 >> 16) & 0xFFFF)); /* complex narrowing */
    
    /* char operations with implicit truncation */
    vc1 = (char)(r1 * r2);              /* multiplication with truncation */
    vc2 = (char)((r1 & 0xFF) + (r2 & 0xFF) + (r3 & 0xFF));
    
    /* Arithmetic that forces SUBREG */
    {
        register char rc1 = 100;
        register char rc2 = 200;
        KEEP(rc1); KEEP(rc2);
        vc1 = rc1 + rc2;                /* char + char -> char with overflow */
    }
    
    return vs1 + vs2 + vs3 + vc1 + vc2;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
static int test_complex_addressing(void) {
    int array[256] = {0};
    int *restrict ptr = array;  /* Use restrict to help optimization */
    int sum = 0;
    
    register int rval1 = 0x11111111;
    register int rval2 = 0x22222222;
    register int rval3 = 0x33333333;
    
    KEEP(rval1); KEEP(rval2); KEEP(rval3);
    
    /* Various complex addressing modes */
    for (int i = 0; i < 32; i++) {
        /* Non-linear index calculation */
        int idx = (i * 13 + 7) & 0xFF;  /* Prime multiplier for non-simple pattern */
        
        /* Store with complex addressing - should generate MEM with complex address */
        ptr[idx] = rval1 + i;           /* Base + scaled index */
        ptr[idx + 1] = rval2 - i;       /* Offset from computed index */
        ptr[(idx * 3) & 0xFF] = rval3 ^ i; /* More complex computation */
    }
    
    /* Multi-dimensional style access */
    {
        int matrix[16][16] = {0};
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                /* Row-major with stride */
                matrix[i][j] = rval1 + i * 16 + j;  /* 2D array access */
            }
        }
        
        /* Sum to prevent elimination */
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                sum += matrix[i][j];
            }
        }
    }
    
    /* Pointer arithmetic with multiple offsets */
    {
        int *p = array;
        p[10] = rval1;
        *(p + 20) = rval2;              /* Pointer + constant offset */
        *(p + 30 + (rval3 & 0xF)) = rval3; /* Complex offset */
    }
    
    /* Sum array to prevent elimination */
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    
    return sum;
}

/* Test 4: Combined patterns */
static int test_combined(void) {
    volatile struct {
        unsigned int flags : 16;
        short data[32];
    } combined = {0};
    
    int *restrict arr = (int*)combined.data;  /* Aliasing for complexity */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    int sum = 0;
    
    KEEP(r1); KEEP(r2);
    
    /* Combined: bitfield + array with complex index */
    combined.flags = (r1 & 0xFFFF) | ((r2 >> 16) & 0xFFFF);
    
    /* Store to short array with narrowing and complex addressing */
    for (int i = 0; i < 16; i++) {
        int idx = (i * 5 + 3) & 0x1F;  /* Non-linear index */
        combined.data[idx] = (short)(r1 + i * r2);  /* Narrowing store */
    }
    
    /* Inline assembly to directly influence RTL generation */
    {
        int temp;
        /* Memory output with complex addressing */
        asm volatile (
            "# Force complex memory pattern\n"
            : "=m" (combined.data[7])   /* Output to memory with fixed offset */
            : "r" (r1)                  /* Input from register */
            : "memory"
        );
        
        /* Another with more complex addressing */
        asm volatile (
            ""
            : "=m" (arr[10 + (r1 & 0x3)])  /* Computed offset */
            :
            : "memory"
        );
    }
    
    /* Read back all values */
    sum += combined.flags;
    for (int i = 0; i < 32; i++) {
        sum += combined.data[i];
    }
    
    return sum;
}

/* Test 5: Additional patterns for specific architectures */
static int test_arch_specific(void) {
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } multi = {0};
    
    register uint64_t r64 = 0xFEDCBA9876543210ULL;
    register uint32_t r32 = 0x12345678;
    
    KEEP(r64); KEEP(r32);
    
    /* Chain of bitfield assignments */
    multi.a = __builtin_parity(r64) & 0x7;      /* Parity on 64-bit */
    multi.b = __builtin_popcount(r32) & 0x1F;   /* Population count */
    multi.c = (r32 >> 10) & 0x3FF;              /* Extract bits */
    multi.d = ((r32 & 0xFFFF) * 3) & 0x3FFF;    /* Computation then mask */
    
    /* Mixed-size operations */
    {
        volatile short vs;
        register int ri = 0x87654321;
        KEEP(ri);
        
        vs = (short)(ri >> 8);          /* Shift then narrow */
        vs = (short)(vs * 2);           /* Operation on narrowed value */
        
        return multi.a + multi.b + multi.c + multi.d + vs;
    }
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource marking...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_bitfields();
    checksum += test_subreg();
    checksum += test_complex_addressing();
    checksum += test_combined();
    checksum += test_arch_specific();
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates code was executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
