/* test_resource_tracking.c
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

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays/structs for memory operand patterns */
int g_array[256];
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
} g_bitfield;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    uint64_t full;
    uint32_t halves[2];
    uint16_t quarters[4];
    uint8_t bytes[8];
} g_union;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Pattern 1: Direct bit-field extraction from struct */
unsigned int extract_bitfield_low(void) {
    /* This should generate ZERO_EXTRACT for the bit-field read */
    return g_bitfield.low8;
}

/* Pattern 2: Manual bit extraction that may become ZERO_EXTRACT */
unsigned int extract_bits_manual(volatile unsigned int *p) {
    /* Complex expression to encourage ZERO_EXTRACT representation */
    return (*p >> (g_volatile_flag & 3)) & ((1 << 8) - 1);
}

/* Pattern 3: Multiple extractions in a loop */
unsigned int extract_multiple(void) {
    unsigned int result = 0;
    volatile unsigned int source = 0x12345678;
    
    for (int i = 0; i < 4; i++) {
        /* Each iteration extracts different 8-bit chunks */
        result ^= (source >> (i * 8)) & 0xFF;
    }
    return result;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Pattern 1: Partial write to 32-bit variable via 16-bit pointer */
void write_low_partial(uint32_t *dest, uint16_t value) {
    /* Cast to smaller type for partial write */
    *(uint16_t *)dest = value;
}

/* Pattern 2: Byte-wise write that may generate STRICT_LOW_PART */
void write_single_byte(volatile uint32_t *p) {
    uint8_t byte_val = g_volatile_counter & 0xFF;
    /* Write only low byte */
    *p = (*p & ~0xFF) | byte_val;
}

/* Pattern 3: Inline assembly hint for partial register (x86 specific) */
#ifdef __x86_64__
void write_low_byte_asm(volatile uint8_t *p) {
    uint8_t val = g_volatile_counter & 0xFF;
    asm volatile("movb %0, %1" : : "r"(val), "m"(*p));
}
#endif

/* ========== SUBREG Patterns ========== */

/* Pattern 1: Union-based type punning */
uint32_t access_via_union(void) {
    g_union.full = 0x1122334455667788ULL;
    /* Access 64-bit as 32-bit - likely SUBREG */
    return g_union.halves[g_volatile_flag & 1];
}

/* Pattern 2: Pointer casting between different sizes */
uint16_t pointer_cast_subreg(uint64_t *src) {
    /* Cast 64-bit pointer to 16-bit pointer */
    return *(uint16_t *)src;
}

/* Pattern 3: Mixed-size operations in expression */
uint32_t mixed_size_ops(uint32_t a, uint16_t b) {
    /* Operations with different sizes may require SUBREG */
    uint64_t temp = (uint64_t)a * (uint64_t)b;
    return (uint32_t)(temp >> 16);
}

/* ========== Complex MEM Patterns ========== */

/* Pattern 1: Array access with complex index calculation */
int complex_array_index(int index1, int index2) {
    /* Non-trivial addressing: base + (i + j*stride) */
    return g_array[(index1 + index2 * 64) & 255];
}

/* Pattern 2: Struct with array member and pointer arithmetic */
struct Container {
    int data[128];
    int metadata;
};

int struct_array_access(struct Container *c, int i, int j) {
    /* Address calculation involving struct field */
    return c->data[(i * j) & 127];
}

/* Pattern 3: Multi-dimensional array with variable indices */
int multi_dim_access(int i, int j, int k) {
    static int md_array[8][8][8];
    /* Complex 3D addressing */
    return md_array[i & 7][j & 7][k & 7];
}

/* Pattern 4: Pointer chain with offsets */
int pointer_chain(int ***triple_ptr, int a, int b) {
    /* Dereference chain with arithmetic */
    return **(triple_ptr + a) + b;
}

/* ========== Combined Function ========== */

/* Function that uses multiple patterns together */
unsigned int combined_operations(void) {
    unsigned int result = 0;
    
    /* ZERO_EXTRACT pattern */
    result ^= extract_bitfield_low();
    
    /* STRICT_LOW_PART pattern */
    uint32_t temp32 = 0xFFFFFFFF;
    write_low_partial(&temp32, 0x1234);
    result ^= temp32;
    
    /* SUBREG pattern */
    result ^= access_via_union();
    
    /* Complex MEM pattern */
    result ^= complex_array_index(g_volatile_counter, g_volatile_counter + 1);
    
    return result;
}

/* ========== Main Function ========== */

int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    g_bitfield.low8 = 0xAB;
    g_bitfield.mid8 = 0xCD;
    g_bitfield.high16 = 0xEF01;
    
    /* Loop to increase optimization opportunities */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Use volatile flag to create unpredictable control flow */
        if (g_volatile_flag) {
            /* Call pattern functions based on counter value */
            switch (g_volatile_counter % 7) {
                case 0:
                    final_result += extract_bits_manual(&g_volatile_counter);
                    break;
                case 1:
                    final_result += extract_multiple();
                    break;
                case 2:
                    write_single_byte((volatile uint32_t *)&final_result);
                    break;
                case 3:
                    final_result ^= pointer_cast_subreg((uint64_t *)&g_union);
                    break;
                case 4:
                    final_result += mixed_size_ops(final_result, g_volatile_counter);
                    break;
                case 5: {
                    struct Container c;
                    for (int i = 0; i < 128; i++) c.data[i] = i;
                    final_result += struct_array_access(&c, 
                        g_volatile_counter, g_volatile_counter + 1);
                    break;
                }
                case 6:
                    final_result += combined_operations();
                    break;
            }
        }
        
        /* Additional MEM pattern with pointer arithmetic */
        int *ptr = g_array + (g_volatile_counter & 0x7F);
        final_result += *ptr;
        
        /* Additional SUBREG pattern via union */
        g_union.quarters[g_volatile_counter & 3] = final_result & 0xFFFF;
        final_result ^= g_union.halves[0];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
