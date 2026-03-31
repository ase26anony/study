/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Force compiler to generate ZERO_EXTRACT patterns */
static void __attribute__((noinline))
pattern_zero_extract_mem(volatile int *base, int idx1, int idx2)
{
    /* Struct with volatile bit-field - generates ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    /* MEM pattern with complex addressing */
    volatile int *ptr = base + idx1 * 8 + idx2;
    
    /* Assignment to volatile bit-field - should generate ZERO_EXTRACT */
    bit_struct.field1 = (*ptr & 0x1F);
    bit_struct.field2 = (*ptr >> 5) & 0x7;
    
    /* More MEM access with addressing */
    volatile int val = *(ptr + 4);
    bit_struct.field3 = val & 0xFF;
}

/* Force STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile short *ps, volatile char *pc)
{
    int combined;
    short temp_short;
    char temp_char;
    
    /* SUBREG pattern: access int as smaller types */
    combined = 0x12345678;
    
    /* Type punning for SUBREG */
    temp_short = *((volatile short *)&combined + 1);  /* SUBREG access */
    *ps = temp_short;
    
    /* STRICT_LOW_PART via inline assembly */
    /* Modify only low byte of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q" (temp_char)    /* =q constraint for byte-addressable reg */
        : "0" (*pc)           /* Same register as input */
        : "cc"
    );
    
    /* More SUBREG: store byte into int */
    *((volatile char *)&combined) = temp_char;  /* SUBREG store */
    
    /* Another STRICT_LOW_PART pattern */
    short s = *ps;
    asm volatile (
        "orb $0x10, %b0\n\t"  /* Modify only low byte */
        : "+q" (s)            /* +q constraint for byte register */
        :
        : "cc"
    );
    *ps = s;
}

/* Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *arr, int i, int j, int cond)
{
    /* Complex MEM addressing with multiple indices */
    volatile int *ptr;
    
    /* Ternary selecting different addressing modes */
    ptr = cond ? (arr + i * 16 + j) : (arr + j * 8 + i);
    
    /* MEM access */
    volatile int val = *ptr;
    
    /* Bit-field struct for ZERO_EXTRACT */
    struct {
        volatile unsigned int low : 4;
        volatile unsigned int high : 4;
    } bf;
    
    /* ZERO_EXTRACT from memory value */
    bf.low = val & 0xF;
    bf.high = (val >> 4) & 0xF;
    
    /* More complex addressing */
    volatile int val2 = *(ptr + (i & 3));
    
    /* SUBREG access through pointer cast */
    short *short_ptr = (short *)&val2;
    volatile short sval = *short_ptr;  /* SUBREG load */
    
    /* Use sval to prevent elimination */
    bf.low ^= sval & 0xF;
}

/* Additional pattern focusing on MEM with complex addressing */
static void __attribute__((noinline))
pattern_mem_complex(volatile int matrix[][8], int n, int m)
{
    /* Multi-dimensional array with volatile indices */
    volatile int idx1 = n;
    volatile int idx2 = m;
    
    /* Complex MEM addressing */
    volatile int *elem = &matrix[idx1 & 3][idx2 & 7];
    
    /* Chain of MEM accesses */
    volatile int a = *elem;
    volatile int b = *(elem + 1);
    volatile int c = *(elem + 2);
    
    /* Use values to prevent elimination */
    struct {
        volatile unsigned int x : 2;
        volatile unsigned int y : 2;
        volatile unsigned int z : 2;
    } bits;
    
    bits.x = a & 0x3;
    bits.y = b & 0x3;
    bits.z = c & 0x3;
    
    /* More addressing with pointer arithmetic */
    volatile int *p = elem;
    for (int i = 0; i < 3; i++) {
        volatile int v = *p++;
        (void)v;  /* Use v to prevent elimination */
    }
}

int main(int argc, char **argv)
{
    volatile int iterations = (argc > 1) ? 10 : 5;  /* Prevent infinite loops */
    volatile int result = 0;
    
    /* Data structures for patterns */
    volatile int array[64];
    volatile int matrix[8][8];
    volatile short short_array[32];
    volatile char char_array[32];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        short_array[i] = i * 2;
        char_array[i] = i;
    }
    
    /* Main loop to trigger resource tracking */
    for (volatile int iter = 0; iter < iterations; iter++) {
        int idx = iter & 0x3F;  /* Limit to array bounds */
        
        /* Call pattern functions with volatile-derived arguments */
        pattern_zero_extract_mem(array, idx & 7, (idx >> 3) & 7);
        
        pattern_strict_low_part_subreg(&short_array[idx & 0x1F], 
                                      &char_array[idx & 0x1F]);
        
        pattern_mixed_complex(array, idx & 7, (idx >> 3) & 7, iter & 1);
        
        pattern_mem_complex(matrix, idx & 7, (idx >> 3) & 7);
        
        /* Accumulate dummy result to prevent elimination */
        result += array[idx] + short_array[idx & 0x1F];
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        return 1;  /* Never happens, but compiler doesn't know */
    }
    
    return 0;
}
