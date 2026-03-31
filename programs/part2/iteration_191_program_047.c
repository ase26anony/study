/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking pass (resource.cc lines 282-290). It creates:
 * 1. ZERO_EXTRACT patterns via bit-field operations
 * 2. STRICT_LOW_PART patterns via partial register writes
 * 3. SUBREG patterns via type-punning and mixed-size accesses
 * 4. Complex MEM patterns via addressing modes with arithmetic
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays/structs for memory operand patterns */
unsigned int g_bitfield_array[256];
int g_data_buffer[1024];

/* Struct with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

/* Union for SUBREG patterns */
union type_punning_union {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
    int8_t bytes[8];
};

/* Struct with array for complex MEM addressing */
struct array_container {
    int values[64];
    int padding[16];
};

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Pattern 1: Direct bit-field extraction from struct */
unsigned int extract_bitfield_direct(struct bitfield_struct *s) {
    /* Accessing bit-field members may generate ZERO_EXTRACT */
    unsigned int val = s->mid8;  /* 8-bit extract from middle */
    return val * g_volatile_flag;
}

/* Pattern 2: Manual bit extraction with shifts */
unsigned int extract_bits_manual(volatile unsigned int *p) {
    /* Complex expression that might generate ZERO_EXTRACT */
    return ((*p >> g_volatile_counter) & 0x3F) + 
           ((*p >> 16) & 0xFF);
}

/* Pattern 3: Multiple extractions in loop */
unsigned int extract_multiple_bits(void) {
    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        /* Each iteration extracts different bit ranges */
        result ^= (g_bitfield_array[i] >> (i * 4)) & 0xF;
    }
    return result;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Pattern 1: Partial write to 32-bit integer */
void write_low_partial(volatile uint32_t *dest, uint16_t value) {
    /* Writing only low 16 bits of a 32-bit location */
    *dest = (*dest & 0xFFFF0000) | value;
}

/* Pattern 2: Byte-wise assignment to integer */
void write_single_byte(volatile uint32_t *dest, uint8_t byte, int pos) {
    /* Write single byte at position pos (0-3) */
    uint32_t mask = 0xFF << (pos * 8);
    *dest = (*dest & ~mask) | (byte << (pos * 8));
}

/* Pattern 3: Using char pointer to modify part of int */
void modify_via_char_ptr(volatile uint32_t *p) {
    volatile uint8_t *byte_ptr = (volatile uint8_t *)p;
    byte_ptr[1] = g_volatile_counter & 0xFF;  /* Modify only second byte */
}

/* ========== SUBREG PATTERNS ========== */

/* Pattern 1: Union-based type punning */
int32_t access_via_subreg_union(union type_punning_union *u) {
    /* Access 64-bit value as smaller parts */
    u->quarters[1] = 0xABCD;      /* Write 16-bit to middle of 64-bit */
    return u->halves[0] + u->bytes[3];  /* Mixed-size accesses */
}

/* Pattern 2: Pointer casting between types */
int32_t access_via_cast(int64_t *large) {
    /* Treat 64-bit location as 32-bit */
    int32_t *as_32bit = (int32_t *)large;
    as_32bit[1] = 0x12345678;     /* Write to high 32 bits */
    return as_32bit[0];           /* Read low 32 bits */
}

/* Pattern 3: Array of different-sized elements */
int mixed_size_array_access(void) {
    union type_punning_union arr[4];
    int32_t sum = 0;
    
    for (int i = 0; i < 4; i++) {
        arr[i].full = i * 0x1000;
        sum += arr[i].halves[i % 2];  /* Access as 16-bit */
        arr[i].bytes[2] = i;          /* Access as 8-bit */
    }
    return sum;
}

/* ========== COMPLEX MEM PATTERNS ========== */

/* Pattern 1: Array access with complex index calculation */
int complex_array_index(struct array_container *cont, int a, int b, int c) {
    /* Non-trivial addressing: base + (a + b*2 + c*3) * sizeof(int) */
    return cont->values[a + b * 2 + c * 3];
}

/* Pattern 2: Pointer arithmetic with multiple terms */
int *compute_complex_address(int *base, int idx1, int idx2, int idx3) {
    /* Address with arithmetic on multiple variables */
    return &base[idx1 * 8 + idx2 * 4 + idx3];
}

/* Pattern 3: Nested struct/array access */
int nested_mem_access(struct array_container **cont_array, int i, int j, int k) {
    /* cont_array[i]->values[j + k * 2] */
    return cont_array[i]->values[j + k * 2];
}

/* ========== MAIN FUNCTION WITH CONTROL FLOW ========== */

int main(void) {
    int result = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        g_bitfield_array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 1024; i++) {
        g_data_buffer[i] = i;
    }
    
    /* Create local variables for patterns */
    struct bitfield_struct bf = {0xAA, 0xBB, 0xCCDD};
    union type_punning_union u;
    u.full = 0x0123456789ABCDEF;
    
    volatile uint32_t partial_reg = 0x87654321;
    struct array_container containers[4];
    struct array_container *cont_ptrs[4];
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 64; j++) {
            containers[i].values[j] = i * 100 + j;
        }
        cont_ptrs[i] = &containers[i];
    }
    
    /* Main loop with conditional execution */
    for (g_volatile_counter = 0; g_volatile_counter < 10; g_volatile_counter++) {
        
        /* ZERO_EXTRACT patterns (conditionally executed) */
        if (g_volatile_counter & 1) {
            result ^= extract_bitfield_direct(&bf);
            result += extract_bits_manual(&g_bitfield_array[g_volatile_counter]);
        }
        
        /* STRICT_LOW_PART patterns */
        if (g_volatile_counter & 2) {
            write_low_partial(&partial_reg, g_volatile_counter * 0x101);
            modify_via_char_ptr(&partial_reg);
        }
        
        /* SUBREG patterns */
        if (g_volatile_counter & 4) {
            result += access_via_subreg_union(&u);
            result ^= access_via_cast(&u.full);
        }
        
        /* Complex MEM patterns */
        int idx = g_volatile_counter % 8;
        result += complex_array_index(&containers[idx % 4], 
                                     idx, idx/2, idx/3);
        
        if (g_volatile_counter > 5) {
            int *addr = compute_complex_address(g_data_buffer, 
                                               idx, idx+1, idx+2);
            result += *addr;
        }
    }
    
    /* Final mixed operation combining all patterns */
    result += extract_multiple_bits();
    write_single_byte(&partial_reg, result & 0xFF, 2);
    result += mixed_size_array_access();
    result += nested_mem_access(cont_ptrs, 1, 2, 3);
    
    /* Ensure result is used */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
