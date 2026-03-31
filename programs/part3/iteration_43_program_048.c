/* resource_patterns.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic, particularly targeting the uncovered lines
 * in resource.cc that handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM.
 */

#include <stddef.h>

/* Force no inlining to preserve RTL patterns */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT and MEM */
NOINLINE static void pattern_zero_extract_mem(volatile int trigger) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 12;
    } bf;
    
    /* Array with complex addressing for MEM patterns */
    static volatile int mem_array[32][16];
    
    /* ZERO_EXTRACT: Assignment to volatile bit-field */
    bf.field1 = trigger & 0x1F;
    bf.field2 = (trigger >> 5) & 0x7F;
    bf.field3 = (trigger >> 12) & 0xFFF;
    
    /* MEM: Complex addressing with multiple indices */
    int idx1 = (trigger * 7) & 0x1F;
    int idx2 = (trigger * 13) & 0x0F;
    
    /* Force MEM reference with addressing calculation */
    volatile int val = mem_array[idx1][idx2];
    
    /* More complex MEM with pointer arithmetic */
    volatile int *ptr = &mem_array[0][0];
    ptr += idx1 * 16 + idx2;
    volatile int val2 = *ptr;
    
    /* Prevent dead code elimination */
    (void)val;
    (void)val2;
}

/* Pattern 2: STRICT_LOW_PART and SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(volatile int trigger) {
    /* Use char/short types to encourage QI/HI modes */
    volatile char c = trigger & 0xFF;
    volatile short s = trigger & 0xFFFF;
    volatile int i = trigger;
    
    /* STRICT_LOW_PART: Inline assembly modifying only part of register */
    /* Modify low byte of integer */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)  /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* Modify low word of integer */
    asm volatile (
        "addw $1, %0\n\t"
        : "=r"(s)  /* Word-sized operation */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG: Type punning between different sizes */
    /* Cast int to short pointer for SUBREG access */
    short *ps = (short*)&i;
    *ps = (trigger >> 8) & 0xFFFF;  /* Modify low word via SUBREG */
    
    /* More SUBREG: Access different parts of the same memory */
    char *pc = (char*)&i;
    pc[1] = trigger & 0xFF;  /* Modify second byte */
    pc[3] = (trigger >> 8) & 0xFF;  /* Modify fourth byte */
    
    /* Mixed-size operations to force SUBREG conversions */
    i = (int)s + (int)c;  /* Promote char/short to int */
    s = (short)(i >> 8);  /* Demote with truncation */
    
    /* Prevent dead code elimination */
    (void)c;
    (void)s;
    (void)i;
}

/* Pattern 3: Complex expression mixing multiple patterns */
NOINLINE static void pattern_complex_mix(volatile int trigger) {
    /* Volatile struct with bit-fields */
    struct mixed_struct {
        volatile unsigned int flags : 4;
        volatile unsigned int value : 20;
        volatile unsigned int pad : 8;
    } ms;
    
    /* Array for MEM patterns */
    static volatile int data[64];
    
    /* Initialize with trigger value */
    ms.flags = trigger & 0x0F;
    ms.value = (trigger >> 4) & 0xFFFFF;
    ms.pad = (trigger >> 24) & 0xFF;
    
    /* Complex addressing calculation */
    int index = ((trigger * 3) + (ms.flags * 7)) & 0x3F;
    
    /* MEM with addressing mode that includes bit-field reference */
    volatile int *addr = &data[index + (ms.value & 0x1F)];
    
    /* Conditional selection of address */
    volatile int *selected_addr;
    if (trigger & 0x100) {
        selected_addr = &data[ms.flags];
    } else {
        selected_addr = addr;
    }
    
    /* Assignment that could involve multiple RTL transformations */
    *selected_addr = (ms.value << 8) | ms.flags;
    
    /* Additional SUBREG pattern via type punning */
    unsigned char *byte_ptr = (unsigned char*)selected_addr;
    byte_ptr[2] = ms.pad;  /* SUBREG store */
    
    /* Prevent dead code elimination */
    (void)byte_ptr;
}

/* Pattern 4: Loop-based pattern generation */
NOINLINE static void pattern_loop_based(volatile int iterations) {
    /* Mixed-size array for SUBREG patterns */
    volatile int int_array[32];
    volatile short short_array[64];
    volatile char char_array[128];
    
    /* Initialize arrays */
    for (volatile int i = 0; i < 32; i++) {
        int_array[i] = i * 3;
    }
    
    /* Complex loop with mixed operations */
    for (volatile int i = 0; i < iterations && i < 16; i++) {
        /* MEM with complex addressing */
        int idx1 = (i * 5) & 0x1F;
        int idx2 = (i * 11) & 0x3F;
        int idx3 = (i * 17) & 0x7F;
        
        /* Mixed-size accesses causing SUBREG */
        short_array[idx2] = (short)(int_array[idx1] >> 4);
        char_array[idx3] = (char)(short_array[idx2] & 0xFF);
        
        /* Bit-field operation in loop (ZERO_EXTRACT) */
        struct {
            volatile unsigned int loop_field : 6;
        } lf;
        lf.loop_field = i & 0x3F;
        
        /* Inline assembly in loop (STRICT_LOW_PART potential) */
        volatile char loop_char = char_array[idx3];
        asm volatile (
            "incb %0\n\t"
            : "+q"(loop_char)
            :
            : "cc"
        );
        char_array[idx3] = loop_char;
    }
}

/* Main function that drives all patterns */
int main(int argc, char *argv[]) {
    /* Use argc to bound loops, preventing infinite loops in analysis */
    volatile int iterations = (argc > 1) ? 10 : 5;
    volatile int trigger = 0x12345678;
    volatile int sum = 0;
    
    /* Call pattern functions in a loop */
    for (volatile int i = 0; i < iterations; i++) {
        /* Update trigger to create varying patterns */
        trigger = (trigger * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Execute all patterns */
        pattern_zero_extract_mem(trigger + i);
        pattern_strict_low_part_subreg(trigger ^ i);
        pattern_complex_mix(trigger * i);
        pattern_loop_based(i + 1);
        
        /* Accumulate to prevent dead code elimination */
        sum += trigger;
    }
    
    /* Final dummy operation using all patterns' results */
    volatile int final = sum & 0xFF;
    
    /* The following would cause UB if run, but is syntactically valid */
    /* Uncomment only for compilation testing, not execution */
    /*
    volatile int *danger = (volatile int*)0x1000;
    *danger = final;
    */
    
    return final != 0 ? 0 : 1;
}
