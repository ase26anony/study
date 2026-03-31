/* Program to generate specific RTL patterns for GCC resource tracking coverage */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ====== ZERO_EXTRACT patterns ====== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_ze(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Bit-field struct with address taken */
struct BitFieldStruct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 4;
    unsigned int field4 : 16;
};

unsigned int read_bitfield_ze(struct BitFieldStruct *bfs) {
    /* Taking address and accessing bit-field */
    unsigned int val = bfs->field2;
    return val;
}

/* ====== STRICT_LOW_PART patterns ====== */

/* Pattern 1: Writing only low byte of a larger integer */
void set_low_byte_slp(volatile unsigned int *p, unsigned char v) {
    /* Write only low 8 bits, preserving high bits */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast to smaller type assignment */
void write_low_half_slp(volatile uint32_t *p) {
    /* Write to low 16 bits via pointer cast */
    *(uint16_t *)p = 0xABCD;
}

/* ====== SUBREG patterns ====== */

/* Pattern 1: Union for type punning */
union TypePunningUnion {
    int32_t full;
    struct {
        int16_t low;
        int16_t high;
    } parts;
};

int32_t access_via_subreg_union(volatile union TypePunningUnion *u) {
    /* Access parts of larger register */
    u->parts.low = 100;
    u->parts.high = 200;
    return u->full;
}

/* Pattern 2: Pointer casting between different sizes */
int64_t subreg_via_cast(volatile int64_t *ll) {
    /* Access 64-bit as 32-bit */
    int32_t low_part = *(int32_t *)ll;
    return low_part * 2;
}

/* ====== Complex MEM patterns ====== */

/* Pattern 1: Array with complex indexing */
struct ComplexArray {
    int data[256];
    int metadata;
};

int access_complex_mem(struct ComplexArray *ca, int idx1, int idx2) {
    /* Complex address calculation: base + (idx1 + idx2*8) * sizeof(int) */
    return ca->data[idx1 + idx2 * 8];
}

/* Pattern 2: Nested struct with pointer arithmetic */
struct Inner {
    int values[10];
};

struct Outer {
    struct Inner blocks[5];
    int count;
};

int nested_mem_access(struct Outer *outer, int block_idx, int elem_idx) {
    /* Multi-level addressing */
    return outer->blocks[block_idx].values[elem_idx];
}

/* ====== Combined function with control flow ====== */

int combined_operations(volatile int mode) {
    int result = 0;
    static unsigned int static_buffer[128];
    struct BitFieldStruct bfs = {0};
    union TypePunningUnion u;
    struct ComplexArray ca;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        ca.data[i] = i;
    }
    
    /* Complex control flow with volatile condition */
    for (int i = 0; i < 10; i++) {
        if (g_volatile_flag) {
            /* ZERO_EXTRACT pattern */
            result += extract_bits_ze(&static_buffer[i % 128]);
            
            /* STRICT_LOW_PART pattern */
            set_low_byte_slp(&static_buffer[(i + 1) % 128], i & 0xFF);
            
            /* Update bit-field struct */
            bfs.field2 = i & 0xFF;
            result += read_bitfield_ze(&bfs);
        } else {
            /* SUBREG pattern */
            u.full = i * 100;
            result += access_via_subreg_union(&u);
            
            /* Complex MEM pattern */
            result += access_complex_mem(&ca, i % 16, (i / 16) % 16);
        }
        
        /* Alternate between patterns based on loop counter */
        if (i % 3 == 0) {
            write_low_half_slp((uint32_t *)&static_buffer[i % 128]);
        }
        
        g_volatile_counter++;
    }
    
    return result;
}

/* ====== Helper functions for different contexts ====== */

int helper_zero_extract(void) {
    volatile unsigned int buffer[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    int sum = 0;
    
    for (int i = 0; i < 4; i++) {
        sum += extract_bits_ze(&buffer[i]);
        sum += extract_bits_ze(&buffer[(i + 1) % 4]);
    }
    
    return sum;
}

int helper_strict_low_part(void) {
    volatile uint32_t values[8] = {0};
    int sum = 0;
    
    for (int i = 0; i < 8; i++) {
        set_low_byte_slp(&values[i], i * 10);
        write_low_half_slp(&values[(i + 1) % 8]);
        sum += values[i];
    }
    
    return sum;
}

int helper_subreg(void) {
    union TypePunningUnion unions[4];
    volatile int64_t large_ints[4] = {1000, 2000, 3000, 4000};
    int sum = 0;
    
    for (int i = 0; i < 4; i++) {
        unions[i].full = i * 1000;
        sum += access_via_subreg_union(&unions[i]);
        sum += subreg_via_cast(&large_ints[i]);
    }
    
    return sum;
}

int helper_complex_mem(void) {
    struct ComplexArray ca;
    struct Outer outer;
    int sum = 0;
    
    /* Initialize structures */
    for (int i = 0; i < 256; i++) ca.data[i] = i * 2;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            outer.blocks[i].values[j] = i * 100 + j;
        }
    }
    
    /* Access with complex addressing */
    for (int i = 0; i < 16; i++) {
        sum += access_complex_mem(&ca, i, 15 - i);
        sum += nested_mem_access(&outer, i % 5, i % 10);
    }
    
    return sum;
}

/* ====== Main function ====== */

int main(void) {
    int final_result = 0;
    
    printf("Starting RTL pattern generation test...\n");
    
    /* Call all helper functions to generate various patterns */
    final_result += helper_zero_extract();
    final_result += helper_strict_low_part();
    final_result += helper_subreg();
    final_result += helper_complex_mem();
    
    /* Combined operations with control flow */
    final_result += combined_operations(g_volatile_flag);
    
    /* Additional loop to increase pass activity */
    for (int iteration = 0; iteration < 3; iteration++) {
        g_volatile_flag = iteration % 2;
        
        /* Mix patterns in loop */
        final_result += helper_zero_extract() >> (iteration * 2);
        final_result += helper_strict_low_part() & (0xFF << iteration);
        
        if (iteration % 2) {
            final_result += helper_subreg();
        } else {
            final_result += helper_complex_mem();
        }
    }
    
    printf("Final result: %d\n", final_result);
    printf("Volatile counter: %u\n", (unsigned int)g_volatile_counter);
    
    return final_result != 0 ? 0 : 1;
}
