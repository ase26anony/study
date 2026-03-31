/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   patterns that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile int g_volatile_index = 0;

/* Global variables for memory access patterns */
int g_global_array[256];
volatile unsigned int g_bitfield_source = 0xDEADBEEF;

/* ========== ZERO_EXTRACT patterns ========== */
/* Bit-field extraction using shift/mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* Access specific bit range: bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Struct with bit-field - taking address may create ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int low8: 8;
    unsigned int mid8: 8;
    unsigned int high16: 16;
};

unsigned int extract_bitfield(struct BitFieldStruct *s) {
    /* Accessing bit-field members */
    unsigned int val = s->mid8;  /* May generate ZERO_EXTRACT for bit-field read */
    return val;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Writing only low part of a variable */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    /* Clear low byte, then set it */
    *p = (*p & ~0xFF) | v;
}

/* Cast to smaller type assignment - may create STRICT_LOW_PART */
void write_low_half(int32_t *x, int16_t value) {
    /* Direct assignment to low part */
    *(int16_t*)x = value;
}

/* ========== SUBREG patterns ========== */
/* Union for type punning - generates SUBREG when accessing parts */
union MixedTypes {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
    int8_t bytes[8];
};

int32_t access_via_subreg(union MixedTypes *u) {
    /* Access part of larger register */
    u->quarters[1] = 0x1234;  /* May generate SUBREG */
    return u->halves[0];      /* Another SUBREG access */
}

/* Pointer casting between different sizes */
int32_t cast_subreg_access(int64_t *ll) {
    /* Access part of larger type */
    int32_t result = *(int32_t*)ll;  /* May generate SUBREG */
    return result;
}

/* ========== Complex MEM patterns ========== */
/* Memory access with complex addressing */
int complex_mem_access(int *base, int idx1, int idx2, int stride) {
    /* Complex address calculation */
    return base[idx1 + idx2 * stride + 3];
}

/* Struct with array - accessing with index */
struct ArrayContainer {
    int data[100];
    int metadata;
};

int struct_mem_access(struct ArrayContainer *cont, int index) {
    /* Address calculation through struct */
    return cont->data[index * 2] + cont->metadata;
}

/* ========== Combined function with control flow ========== */
/* This function combines multiple patterns in a loop with conditionals */
unsigned int combined_operations(void) {
    unsigned int result = 0;
    union MixedTypes u;
    struct BitFieldStruct bfs = {0};
    struct ArrayContainer container;
    
    /* Initialize data */
    u.full = 0x1122334455667788ULL;
    bfs.mid8 = 0xAB;
    for (int i = 0; i < 100; i++) {
        container.data[i] = i * 3;
    }
    container.metadata = 999;
    
    /* Loop with volatile condition to create complex control flow */
    for (int i = 0; i < 10; i++) {
        if (g_volatile_flag) {
            /* ZERO_EXTRACT pattern */
            result ^= extract_bits_shift(&g_bitfield_source);
            
            /* Update bitfield source */
            g_bitfield_source = (g_bitfield_source << 1) | (result & 1);
        } else {
            /* STRICT_LOW_PART pattern */
            set_low_byte(&g_bitfield_source, (unsigned char)(result & 0xFF));
        }
        
        /* SUBREG pattern - always executed */
        result += access_via_subreg(&u);
        
        /* Complex MEM pattern */
        int idx = g_volatile_index % 50;
        result += struct_mem_access(&container, idx);
        
        /* Another MEM pattern with complex addressing */
        result += complex_mem_access(g_global_array, 
                                    i, 
                                    idx, 
                                    4);
        
        /* Conditional STRICT_LOW_PART via cast */
        if (result % 3 == 0) {
            write_low_half((int32_t*)&result, (int16_t)(result >> 16));
        }
    }
    
    return result;
}

/* ========== Helper functions for more pattern variations ========== */
/* Function emphasizing ZERO_EXTRACT */
unsigned int bitfield_heavy(void) {
    struct BitFieldStruct bfs;
    bfs.low8 = 0x12;
    bfs.mid8 = 0x34;
    bfs.high16 = 0x5678;
    
    unsigned int r = 0;
    r |= bfs.low8;      /* Potential ZERO_EXTRACT */
    r |= bfs.mid8 << 8; /* Another potential ZERO_EXTRACT */
    r |= bfs.high16 << 16;
    
    /* Also extract via shift/mask */
    volatile unsigned int v = 0x89ABCDEF;
    r ^= (v >> 4) & 0x0F0F0F0F;  /* Multiple extractions */
    
    return r;
}

/* Function emphasizing SUBREG operations */
int64_t subreg_heavy(void) {
    union MixedTypes u1, u2;
    u1.full = 0xAABBCCDDEEFF1122ULL;
    u2.full = 0x2233445566778899ULL;
    
    /* Multiple subreg accesses */
    u1.quarters[0] = u2.quarters[3];
    u1.halves[1] = u2.halves[0];
    u1.bytes[7] = u2.bytes[1];
    
    /* Cast between pointer types */
    int32_t part1 = cast_subreg_access(&u1.full);
    int32_t part2 = *(int32_t*)((char*)&u2.full + 4);
    
    return (int64_t)part1 * part2;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_global_array[i] = i * i;
    }
    
    /* Call combined function multiple times with different volatile conditions */
    for (int iteration = 0; iteration < 5; iteration++) {
        g_volatile_flag = iteration % 2;
        g_volatile_index = iteration * 7;
        
        final_result += combined_operations();
        
        /* Mix in pattern-specific functions */
        if (iteration % 2) {
            final_result ^= bitfield_heavy();
        } else {
            final_result ^= (unsigned int)subreg_heavy();
        }
    }
    
    /* Use result to prevent optimization */
    printf("Final result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
