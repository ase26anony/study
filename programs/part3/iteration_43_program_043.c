/* resource_coverage.c
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic, particularly targeting lines 282-290 of resource.cc
 * which handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM expressions.
 */

#include <stddef.h>

/* Force no inlining to ensure functions remain distinct for RTL generation */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT and MEM combination */
NOINLINE static void pattern_zero_extract_mem(volatile int trigger) {
    /* Struct with volatile bit-field to generate ZERO_EXTRACT */
    struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bit_struct;
    
    /* Array with complex addressing for MEM patterns */
    static volatile int mem_array[16][8];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bit_struct.field1 = (trigger & 0x1F);
    bit_struct.field2 = ((trigger >> 5) & 0x7F);
    bit_struct.field3 = ((trigger >> 12) & 0x07);
    
    /* MEM pattern with complex addressing */
    int idx1 = (trigger & 0xF);
    int idx2 = ((trigger >> 4) & 0x7);
    volatile int val = mem_array[idx1][idx2];
    
    /* More complex MEM addressing with pointer arithmetic */
    volatile int *ptr = &mem_array[0][0];
    ptr += idx1 * 8 + idx2;
    volatile int val2 = *ptr;
    
    /* Prevent dead code elimination */
    (void)val;
    (void)val2;
}

/* Pattern 2: STRICT_LOW_PART and SUBREG combination */
NOINLINE static void pattern_strict_low_part_subreg(volatile int trigger) {
    /* Use different sized types to encourage SUBREG usage */
    volatile short s_val = (trigger & 0xFFFF);
    volatile char c_val = (trigger & 0xFF);
    volatile int i_val = trigger;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying only part of register */
    /* Modify low byte of short value */
    asm volatile (
        "addb %b1, %b0"
        : "=q"(c_val)
        : "q"(c_val), "0"(c_val)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different constraint */
    asm volatile (
        "orb $0x0F, %b0"
        : "=q"(s_val)
        : "0"(s_val)
        : "cc"
    );
    
    /* SUBREG pattern: type punning through pointer casts */
    /* Access int as short (SUBREG from SImode to HImode) */
    short *ps = (short*)&i_val;
    *ps = (trigger & 0xFFFF);
    
    /* Access int as char (SUBREG from SImode to QImode) */
    char *pc = (char*)&i_val;
    pc[1] = (trigger >> 8) & 0xFF;
    
    /* Mixed-size operations that may generate SUBREG */
    i_val = (int)s_val + (int)c_val;
    
    /* Prevent dead code elimination */
    (void)i_val;
}

/* Pattern 3: Complex expression mixing multiple patterns */
NOINLINE static void pattern_complex_mix(volatile int trigger) {
    /* Volatile struct with bit-fields for ZERO_EXTRACT */
    struct {
        volatile unsigned int flags : 4;
        volatile unsigned int data : 12;
    } volatile bit_fields;
    
    /* Arrays for MEM patterns */
    static volatile int array1[32];
    static volatile short array2[64];
    
    /* Complex addressing calculation */
    int idx = (trigger & 0x1F);
    int offset = ((trigger >> 5) & 0x3F);
    
    /* MEM pattern with scaled indexing */
    volatile int *mem_ptr = &array1[idx];
    
    /* Conditional that may affect addressing mode */
    volatile int *selected_ptr = (trigger & 0x100) ? 
                                 (volatile int*)&bit_fields.flags : 
                                 mem_ptr;
    
    /* ZERO_EXTRACT through pointer if bit-fields selected */
    if (trigger & 0x100) {
        /* This may generate ZERO_EXTRACT when compiled */
        bit_fields.data = (offset & 0xFFF);
    } else {
        /* MEM access with pointer arithmetic */
        *mem_ptr = array2[offset];
    }
    
    /* SUBREG pattern with type punning */
    if (trigger & 0x200) {
        /* Treat int array as short array */
        volatile short *short_view = (volatile short*)array1;
        short_view[idx * 2] = (trigger & 0xFFFF);
    }
    
    /* Prevent dead code elimination */
    (void)selected_ptr;
}

/* Pattern 4: Nested structures with bit-fields and arrays */
NOINLINE static void pattern_nested_structs(volatile int trigger) {
    /* Nested struct with bit-fields */
    struct Inner {
        volatile unsigned int a : 3;
        volatile unsigned int b : 9;
        volatile unsigned int c : 4;
    };
    
    struct Outer {
        struct Inner inner;
        volatile int array[4];
        volatile short shorts[8];
    };
    
    static volatile struct Outer outer_struct;
    
    /* ZERO_EXTRACT on nested struct member */
    outer_struct.inner.b = (trigger & 0x1FF);
    
    /* MEM with complex addressing through struct */
    int idx = (trigger & 0x3);
    outer_struct.array[idx] = trigger;
    
    /* SUBREG through pointer casting within struct */
    volatile int *int_ptr = (volatile int*)&outer_struct.shorts[0];
    *int_ptr = (trigger & 0xFFFF);
    
    /* More complex MEM addressing */
    volatile short *sptr = &outer_struct.shorts[idx * 2];
    *sptr = (trigger >> 8) & 0xFFFF;
    
    /* Prevent dead code elimination */
    (void)int_ptr;
}

/* Main function that drives all patterns */
int main(int argc, char *argv[]) {
    volatile int iteration_limit = 10;
    
    /* Use argc to bound loops if provided, otherwise use default */
    if (argc > 1) {
        /* Simple hash of argv[1] to get iteration count */
        const char *str = argv[1];
        iteration_limit = 0;
        while (*str) {
            iteration_limit = (iteration_limit * 31) + *str++;
        }
        iteration_limit = (iteration_limit & 0xFF) + 5;
    }
    
    volatile int sum = 0;
    
    /* Loop to ensure RTL patterns are generated multiple times */
    for (volatile int i = 0; i < iteration_limit; i++) {
        int trigger_val = i + (iteration_limit << 8);
        
        /* Call each pattern function with different triggers */
        pattern_zero_extract_mem(trigger_val);
        pattern_strict_low_part_subreg(trigger_val + 1);
        pattern_complex_mix(trigger_val + 2);
        pattern_nested_structs(trigger_val + 3);
        
        /* Accumulate to prevent dead code elimination */
        sum += trigger_val;
    }
    
    /* Final dummy use of sum */
    volatile int result = sum & 0xFF;
    
    return result;
}
