/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL intermediate representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable
   control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory operations */
volatile unsigned int global_bitfield = 0xABCD1234;
volatile unsigned int global_value = 0;
int global_array[256];
int global_array2[128];

/* Struct with bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

/* Union for SUBREG generation */
union mixed_types {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* Struct for complex memory addressing */
struct nested_struct {
    int data[64];
    struct bitfield_struct bf;
    int padding[8];
};

/* Helper function 1: Generates ZERO_EXTRACT patterns through bit-field access */
int extract_bits_complex(struct bitfield_struct *s, volatile int selector) {
    unsigned int result = 0;
    
    /* Different bit-field accesses based on selector */
    if (selector & 1) {
        result += s->low8;          /* 8-bit extract from low bits */
    }
    if (selector & 2) {
        result += s->mid16;         /* 16-bit extract from middle */
    }
    if (selector & 4) {
        result += s->high8;         /* 8-bit extract from high bits */
    }
    
    /* Additional manual bit extraction that may generate ZERO_EXTRACT */
    volatile unsigned int *p = &global_bitfield;
    if (selector & 8) {
        /* This should generate ZERO_EXTRACT: extract bits 8-15 */
        result += ((*p >> 8) & 0xFF);
    }
    if (selector & 16) {
        /* Extract bits 16-31 */
        result += ((*p >> 16) & 0xFFFF);
    }
    
    return result;
}

/* Helper function 2: Generates STRICT_LOW_PART patterns */
void modify_low_parts(volatile unsigned int *dest, unsigned char byte_val, 
                      unsigned short short_val, volatile int mode) {
    if (mode == 0) {
        /* Write only low byte - may generate STRICT_LOW_PART */
        *dest = (*dest & ~0xFF) | byte_val;
    } else if (mode == 1) {
        /* Write only low 16 bits */
        *dest = (*dest & ~0xFFFF) | short_val;
    } else {
        /* Combined operation */
        *dest = (*dest & ~0xFFFF) | ((short_val & 0xFF) | ((byte_val << 8) & 0xFF00));
    }
    
    /* Another pattern that may generate STRICT_LOW_PART */
    union mixed_types u;
    u.full = *dest;
    if (mode & 1) {
        /* Write to low 16 bits through union */
        u.halves[0] = short_val;
    }
    *dest = u.full;
}

/* Helper function 3: Generates SUBREG patterns through type punning */
int subreg_operations(union mixed_types *u, volatile int index) {
    int result = 0;
    
    /* Access different parts of the same storage */
    result += u->halves[0];          /* Access as 16-bit */
    result += u->halves[1];          /* Another 16-bit access */
    
    /* Byte accesses that may require SUBREG */
    result += u->bytes[index & 3];
    result += u->bytes[(index + 1) & 3];
    
    /* Cast between different sized types */
    int32_t temp = u->full;
    if (index & 1) {
        int16_t *ptr16 = (int16_t *)&temp;
        result += ptr16[0];
        result += ptr16[1];
    }
    
    return result;
}

/* Helper function 4: Generates complex MEM patterns with addressing modes */
int complex_memory_access(struct nested_struct *ns, int idx1, int idx2, 
                          volatile int offset) {
    int result = 0;
    
    /* Array access with non-trivial index calculation */
    result += ns->data[idx1 * 4 + idx2];
    
    /* More complex addressing with multiple components */
    result += ns->data[(idx1 + offset) * 2 - idx2];
    
    /* Pointer arithmetic that creates complex MEM addresses */
    int *ptr = &ns->data[0];
    result += ptr[idx1 * 8 + offset];
    result += ptr[idx2 * 4 - offset];
    
    /* Struct member access through pointer */
    result += ns->bf.low8;
    result += ns->bf.mid16;
    
    return result;
}

/* Main function that combines all patterns with control flow */
int main(void) {
    int i, total = 0;
    
    /* Initialize data structures */
    struct bitfield_struct bfs = {0xAA, 0xBBCC, 0xDD};
    union mixed_types mu;
    mu.full = 0x12345678;
    
    struct nested_struct ns;
    for (i = 0; i < 64; i++) {
        ns.data[i] = i * 3 + 1;
    }
    ns.bf.low8 = 0x11;
    ns.bf.mid16 = 0x2233;
    ns.bf.high8 = 0x44;
    
    /* Initialize global arrays */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 2;
    }
    for (i = 0; i < 128; i++) {
        global_array2[i] = i * 3;
    }
    
    /* Main loop with volatile-controlled execution */
    for (v_counter = 0; v_counter < 100; v_counter++) {
        /* Volatile conditions create unpredictable control flow */
        if (v_flag1 || (v_counter % 7 == 0)) {
            /* Generate ZERO_EXTRACT patterns */
            total += extract_bits_complex(&bfs, v_counter);
            
            /* Update global_bitfield to affect future extractions */
            global_bitfield = (global_bitfield * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        if (v_flag2 || (v_counter % 5 == 0)) {
            /* Generate STRICT_LOW_PART patterns */
            modify_low_parts(&global_value, 
                           (v_counter & 0xFF),
                           (v_counter * 3) & 0xFFFF,
                           v_counter % 3);
            
            total += global_value;
        }
        
        if ((v_counter % 3) == 0) {
            /* Generate SUBREG patterns */
            mu.full = (mu.full * 1664525 + 1013904223) & 0xFFFFFFFF;
            total += subreg_operations(&mu, v_counter);
        }
        
        if ((v_counter % 4) == 0) {
            /* Generate complex MEM patterns */
            int idx1 = (v_counter * 2) % 16;
            int idx2 = (v_counter * 3) % 16;
            total += complex_memory_access(&ns, idx1, idx2, v_counter % 8);
            
            /* Additional array access with complex addressing */
            total += global_array[(idx1 * 7 + idx2 * 3) % 256];
            total += global_array2[(idx1 * 5 - idx2 * 2 + 64) % 128];
        }
        
        /* Occasionally toggle volatile flags */
        if (v_counter % 23 == 0) {
            v_flag1 = !v_flag1;
        }
        if (v_counter % 29 == 0) {
            v_flag2 = !v_flag2;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional forced exit with result to ensure all code paths matter */
    if (total > 1000000) {
        return 1;
    }
    
    return 0;
}
