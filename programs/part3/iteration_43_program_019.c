/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources function).
 * The goal is to exercise the uncovered lines handling:
 *   ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM expressions.
 * 
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 */

#include <stddef.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bf;
    
    /* Array with complex addressing for MEM patterns */
    static volatile int arr[16][16];
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bf.field1 = (*counter) & 0x1F;
    bf.field2 = (*counter >> 5) & 0x7;
    bf.field3 = (*counter >> 8) & 0xFF;
    
    /* MEM: Complex array indexing with pointer arithmetic */
    int idx1 = (*counter) & 0xF;
    int idx2 = (*counter >> 4) & 0xF;
    
    /* Force MEM with addressing calculation */
    volatile int val = arr[idx1][idx2];
    arr[idx2][idx1] = val + bf.field1;
    
    /* More MEM with offset calculation */
    volatile int *ptr = &arr[0][0];
    ptr += (idx1 * 16 + idx2);
    *ptr = bf.field2;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile short s_val = *counter & 0xFFFF;
    volatile char c_val = *counter & 0xFF;
    
    /* STRICT_LOW_PART: Inline assembly modifying low part of register */
    {
        char tmp = c_val;
        /* "=q" constraint for byte-addressable register */
        asm volatile (
            "addb $1, %0\n\t"
            "subb $2, %0"
            : "=q"(tmp)
            : "0"(tmp)
            : "cc"
        );
        c_val = tmp;
    }
    
    /* Another STRICT_LOW_PART with different operation */
    {
        short tmp = s_val;
        asm volatile (
            "incw %0\n\t"
            "decw %0"
            : "=q"(tmp)
            : "0"(tmp)
            : "cc"
        );
        s_val = tmp;
    }
    
    /* SUBREG: Type punning through different sized accesses */
    {
        int int_val = *counter;
        /* Access lower 16 bits via short pointer */
        short *short_ptr = (short *)&int_val;
        *short_ptr = s_val;  /* This should generate SUBREG */
        
        /* Access individual bytes */
        char *char_ptr = (char *)&int_val;
        char_ptr[1] = c_val;  /* Another SUBREG */
        
        /* Mixed size operations */
        long long big_val = (long long)int_val * 3;
        int *int_ptr = (int *)&big_val;
        int_val = int_ptr[0] + int_ptr[1];  /* More SUBREG operations */
    }
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *result) {
    /* Struct with bit-fields at different positions */
    struct mixed_struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 7;
        volatile unsigned int c : 10;
        volatile unsigned int padding : 12;
    } ms;
    
    /* Array for MEM patterns */
    static volatile int buffer[32];
    
    /* Initialize */
    ms.a = (*counter) & 0x7;
    ms.b = (*counter >> 3) & 0x7F;
    ms.c = (*counter >> 10) & 0x3FF;
    
    /* Complex addressing with ternary operator */
    volatile int *ptr;
    int idx = *counter & 0x1F;
    
    /* This complex expression may generate interesting RTL */
    ptr = (ms.a > 3) ? &buffer[idx] : &buffer[31 - idx];
    
    /* ZERO_EXTRACT from bit-field combined with MEM access */
    int temp = *ptr;
    temp += ms.b;
    
    /* More type punning for SUBREG */
    {
        int combined = (ms.c << 16) | temp;
        short *sp = (short *)&combined;
        sp[0] = sp[0] + sp[1];  /* SUBREG access */
        *ptr = combined;
    }
    
    /* Update result to prevent elimination */
    *result += ms.a + ms.b + ms.c + *ptr;
}

/* Function D: Additional patterns with loops inside functions */
NOINLINE static void func_d(volatile int *counter) {
    /* Local array with volatile accesses */
    volatile int local_arr[8];
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        local_arr[i] = (*counter + i) & 0xFF;
    }
    
    /* Mixed size accesses causing SUBREG */
    {
        long long accumulator = 0;
        for (int i = 0; i < 8; i += 2) {
            /* Combine two ints into one long long */
            int *int_ptr = (int *)&accumulator;
            int_ptr[0] = local_arr[i];
            int_ptr[1] = local_arr[i + 1];
            
            /* Access as different types */
            short *short_ptr = (short *)&accumulator;
            short_ptr[2] = short_ptr[0] + short_ptr[1];  /* SUBREG */
        }
    }
    
    /* Bit-field in local struct */
    {
        struct {
            volatile unsigned int low : 4;
            volatile unsigned int high : 4;
        } bits;
        
        bits.low = (*counter) & 0xF;
        bits.high = (*counter >> 4) & 0xF;
        
        /* Use both fields */
        local_arr[0] = (bits.high << 4) | bits.low;
    }
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Use argc to bound loops for compilation (won't actually run) */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Loop to generate repeated RTL patterns */
    for (counter = 0; counter < iterations; counter++) {
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, &result);
        func_d(&counter);
        
        /* Additional direct pattern generation in main */
        {
            /* MEM with complex addressing */
            volatile int array[10][10];
            int i = counter % 10;
            int j = (counter * 3) % 10;
            volatile int v = array[i][j];
            array[j][i] = v + counter;
            
            /* STRICT_LOW_PART on char */
            volatile char c = counter & 0xFF;
            asm volatile (
                "xorb %%al, %%al\n\t"
                "orb %1, %%al"
                : "=q"(c)
                : "q"(c)
                : "cc"
            );
            
            /* ZERO_EXTRACT via bit-field in union */
            union {
                struct {
                    volatile unsigned int bit : 1;
                } s;
                volatile unsigned int full;
            } u;
            u.s.bit = counter & 0x1;
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        /* This branch won't be taken, but prevents optimization */
        return 1;
    }
    
    return 0;
}
