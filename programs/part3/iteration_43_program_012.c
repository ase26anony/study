/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources), particularly
 * targeting the uncovered lines handling ZERO_EXTRACT, STRICT_LOW_PART,
 * SUBREG, and MEM expressions.
 *
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 * Or for more aggressive optimization: gcc -O3 -m32 -funroll-loops -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Force functions to not be inlined to ensure they generate independent RTL */
#define NOINLINE __attribute__((noinline))

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void func_a(volatile int *counter) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bs;
    
    /* Array with complex addressing for MEM patterns */
    volatile int arr[16][16];
    volatile int *ptr;
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bs.field1 = (*counter) & 0x1F;
    bs.field2 = (*counter >> 5) & 0x07;
    bs.field3 = (*counter >> 8) & 0xFF;
    
    /* MEM: Complex addressing with pointer arithmetic */
    ptr = &arr[0][0];
    ptr += (*counter % 256);  /* Force non-constant offset */
    
    /* More MEM: Multi-dimensional array access */
    arr[(*counter) % 16][(*counter >> 4) % 16] = bs.field1;
    
    /* ZERO_EXTRACT with MEM: Bit-field in pointed-to struct */
    struct bitfield_struct *bsp = &bs;
    bsp->field2 = (*counter) & 0x07;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void func_b(volatile int *counter) {
    volatile short s_val;
    volatile char c_val;
    int mixed_int;
    
    /* Initialize values */
    s_val = (*counter) & 0xFFFF;
    c_val = (*counter) & 0xFF;
    mixed_int = *counter;
    
    /* STRICT_LOW_PART: Inline assembly modifying only low part of register */
    /* Using byte operation on char/byte-sized variable */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c_val)      /* =q constraint for byte-addressable register */
        : "0"(c_val)       /* Matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART with word operation */
    asm volatile (
        "addw $1, %0\n\t"
        : "=r"(s_val)      /* Word-sized operation */
        : "0"(s_val)
        : "cc"
    );
    
    /* SUBREG: Type punning through pointer casts */
    /* Cast int pointer to short pointer for partial access */
    short *short_ptr = (short *)&mixed_int;
    *short_ptr = s_val;  /* This generates SUBREG access */
    
    /* More SUBREG: Access different parts of the same memory */
    char *char_ptr = (char *)&mixed_int;
    char_ptr[1] = c_val;  /* Access byte within int */
    
    /* Mixed-size operations that may generate SUBREG */
    mixed_int = (mixed_int & 0xFFFF0000) | s_val;
}

/* Function C: Complex expression mixing multiple patterns */
NOINLINE static void func_c(volatile int *counter, volatile int *arr_base) {
    /* Struct with bit-fields at different positions */
    struct complex_struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 5;
        volatile unsigned int c : 10;
        volatile unsigned int d : 14;
    } cs;
    
    volatile int temp;
    volatile int * volatile ptr;  /* volatile pointer */
    
    /* Initialize */
    cs.a = (*counter) & 0x07;
    cs.b = (*counter >> 3) & 0x1F;
    cs.c = (*counter >> 8) & 0x3FF;
    cs.d = (*counter >> 18) & 0x3FFF;
    
    /* Complex addressing with ternary operator */
    ptr = (*(counter) & 1) ? (int *)&cs.a : arr_base;
    
    /* MEM with complex index calculation */
    temp = ptr[(*counter) % 8];
    
    /* ZERO_EXTRACT in conditional context */
    if (temp & 1) {
        cs.b = (temp >> 1) & 0x1F;
    } else {
        cs.c = (temp >> 6) & 0x3FF;
    }
    
    /* More pointer arithmetic for MEM patterns */
    int *alias_ptr = (int *)&cs;
    alias_ptr[0] = alias_ptr[0] ^ (*counter);  /* Type punning */
}

/* Function D: Additional patterns with loops inside function */
NOINLINE static void func_d(volatile int *counter) {
    volatile int local_arr[8];
    volatile int i, j;
    
    /* Initialize array */
    for (i = 0; i < 8; i++) {
        local_arr[i] = (*counter + i) & 0xFF;
    }
    
    /* Nested array access for complex MEM patterns */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            /* Complex addressing mode */
            int idx = (i * 2 + j) % 8;
            volatile int *elem = &local_arr[idx];
            
            /* STRICT_LOW_PART via inline asm on array element */
            asm volatile (
                "orb $1, %b0\n\t"  /* Byte operation */
                : "+q"(*elem)      /* Byte register constraint */
                :
                : "cc"
            );
        }
    }
    
    /* SUBREG through union type punning */
    union {
        int full;
        short halves[2];
        char bytes[4];
    } pun;
    
    pun.full = *counter;
    pun.halves[1] = local_arr[0];  /* SUBREG access */
    pun.bytes[2] = (char)(local_arr[1] & 0xFF);  /* Another SUBREG */
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int result = 0;
    volatile int arr[64];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 64; i++) {
        arr[i] = i * 3;
    }
    
    /* Use argc to bound loops for compilation (won't actually run) */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Main loop to generate repeated RTL patterns */
    for (volatile int iter = 0; iter < iterations; iter++) {
        counter = iter;
        
        /* Call each pattern-generating function */
        func_a(&counter);
        func_b(&counter);
        func_c(&counter, arr);
        func_d(&counter);
        
        /* Prevent dead code elimination */
        result += counter;
        
        /* Complex addressing in main loop */
        volatile int *ptr = &arr[counter % 64];
        *ptr = result & 0xFF;
        
        /* Bit-field in main to ensure coverage */
        struct {
            volatile unsigned int main_field : 4;
        } mf;
        mf.main_field = (result >> 4) & 0x0F;
    }
    
    /* Final dummy use of result to prevent optimization */
    volatile int final = result;
    
    /* The program doesn't need to run correctly, just compile */
    /* So we return 0 regardless */
    return 0;
}
