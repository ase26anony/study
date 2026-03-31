/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   patterns that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory operand patterns */
unsigned int g_bitfield_source = 0xDEADBEEF;
int g_array[256];
long long g_large_value = 0x123456789ABCDEF0LL;

/* Structs for bit-field and memory access patterns */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

struct NestedStruct {
    int data[4];
    struct BitFieldStruct bf;
    int more_data[4];
};

/* Union for SUBREG pattern generation */
union MixedSizeUnion {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

/* ========== ZERO_EXTRACT patterns ========== */
/* Pattern 1: Direct bit-field extraction */
unsigned int extract_bitfield_direct(struct BitFieldStruct *s) {
    /* Accessing bit-field members may generate ZERO_EXTRACT */
    return s->mid16;
}

/* Pattern 2: Manual bit extraction that may become ZERO_EXTRACT */
unsigned int extract_bits_manual(volatile unsigned int *p) {
    /* Complex expression to prevent optimization into simpler form */
    unsigned int val = *p;
    if (g_volatile_flag) {
        /* This shift-and-mask pattern may generate ZERO_EXTRACT */
        return (val >> 8) & 0xFFFF;
    }
    return (val >> 4) & 0xFFF;
}

/* Pattern 3: Multiple extractions in a loop */
unsigned int extract_multiple_bits(void) {
    unsigned int result = 0;
    volatile unsigned int source = g_bitfield_source;
    
    for (int i = 0; i < 4; i++) {
        /* Different extraction widths */
        unsigned int extracted = (source >> (i * 8)) & ((1 << (8 - i)) - 1);
        result ^= extracted;
    }
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Pattern 1: Writing to partial register via pointer cast */
void write_low_part_pointer(int32_t *dest, int16_t value) {
    /* Cast to smaller type may generate STRICT_LOW_PART */
    *(int16_t *)dest = value;
}

/* Pattern 2: Byte-wise assignment to larger integer */
void set_low_byte_volatile(volatile unsigned int *p) {
    /* Writing only low byte */
    *p = (*p & ~0xFF) | (g_volatile_counter & 0xFF);
}

/* Pattern 3: Using union for partial write */
void union_partial_write(union MixedSizeUnion *u, int index, int8_t value) {
    /* Writing to one byte of the union */
    u->bytes[index] = value;
}

/* ========== SUBREG patterns ========== */
/* Pattern 1: Union-based type punning */
int32_t subreg_via_union(union MixedSizeUnion *u) {
    /* Accessing smaller parts of the union */
    int32_t result = u->full;
    result += u->halves[0];
    result -= u->bytes[2];
    return result;
}

/* Pattern 2: Pointer casting between different sizes */
int64_t subreg_pointer_cast(void) {
    /* Cast between different pointer types */
    long long *llptr = &g_large_value;
    int32_t *iptr = (int32_t *)llptr;
    return *iptr + *(iptr + 1);
}

/* Pattern 3: Mixed-size operations in expression */
int32_t mixed_size_operations(int32_t a, int16_t b, int8_t c) {
    /* Operations mixing different sizes may generate SUBREG */
    int32_t result = a + b;  /* b promoted, but may have SUBREG in RTL */
    result *= c;             /* c promoted */
    return result;
}

/* ========== Complex MEM patterns ========== */
/* Pattern 1: Array access with complex index calculation */
int complex_array_access(int index1, int index2) {
    /* Complex addressing mode */
    return g_array[(index1 * 3 + index2 * 7) & 0xFF];
}

/* Pattern 2: Struct with nested array access */
int nested_struct_access(struct NestedStruct *ns, int i, int j) {
    /* Multiple memory operands with different addressing */
    int result = ns->data[i];
    result += ns->more_data[j];
    result += ns->bf.low8;  /* Also triggers bit-field access */
    return result;
}

/* Pattern 3: Pointer arithmetic in loop */
int pointer_arithmetic_sum(int *base, int n) {
    int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing with pointer increment */
        sum += *ptr;
        ptr += (g_volatile_flag & 1) ? 1 : 2;
    }
    return sum;
}

/* ========== Combined patterns ========== */
/* Function that combines multiple patterns */
unsigned int combined_patterns(void) {
    unsigned int result = 0;
    static union MixedSizeUnion u = { .full = 0x12345678 };
    static struct NestedStruct ns = {0};
    static struct BitFieldStruct bfs = {0};
    
    /* Initialize with some values */
    for (int i = 0; i < 4; i++) {
        ns.data[i] = i * 100;
        ns.more_data[i] = i * 200;
    }
    bfs.low8 = 0xAB;
    bfs.mid16 = 0xCDEF;
    
    /* Mix different patterns */
    if (g_volatile_flag & 1) {
        /* ZERO_EXTRACT pattern */
        result ^= extract_bits_manual(&g_bitfield_source);
    }
    
    if (g_volatile_flag & 2) {
        /* STRICT_LOW_PART pattern */
        write_low_part_pointer(&ns.data[0], 0x1234);
    }
    
    if (g_volatile_counter % 3 == 0) {
        /* SUBREG pattern */
        result += subreg_via_union(&u);
    }
    
    /* Complex MEM pattern */
    result += complex_array_access(g_volatile_counter, g_volatile_counter + 1);
    
    return result;
}

/* Main function with control flow to ensure all patterns are exercised */
int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3;
    }
    
    /* Create a complex control flow to ensure resource tracking sees various patterns */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Change volatile flag to create different execution paths */
        g_volatile_flag = (g_volatile_counter % 7) + 1;
        
        /* Call different pattern generators based on control flow */
        switch (g_volatile_counter % 5) {
            case 0:
                final_result ^= extract_multiple_bits();
                break;
            case 1:
                set_low_byte_volatile(&g_bitfield_source);
                break;
            case 2:
                final_result += mixed_size_operations(
                    g_volatile_counter, 
                    g_volatile_counter * 2,
                    g_volatile_counter * 3
                );
                break;
            case 3:
                final_result += pointer_arithmetic_sum(g_array, 10);
                break;
            case 4:
                final_result += combined_patterns();
                break;
        }
        
        /* Ensure all patterns get some use */
        if (g_volatile_counter % 13 == 0) {
            union MixedSizeUnion u = { .full = g_volatile_counter };
            union_partial_write(&u, 1, g_volatile_counter & 0xFF);
            final_result += u.full;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %u\n", final_result);
    return (int)final_result;
}
