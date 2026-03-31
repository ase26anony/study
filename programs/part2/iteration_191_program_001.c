/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   that should generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory pattern generation */
unsigned int g_global_array[256];
int g_global_int = 0x12345678;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Bit-field struct */
struct bitfield_struct {
    unsigned int low : 4;
    unsigned int middle : 8;
    unsigned int high : 20;
};

unsigned int extract_bitfield(struct bitfield_struct *s) {
    /* Accessing bit-fields often generates ZERO_EXTRACT */
    return s->middle;
}

/* Pattern 3: Multiple extractions in a loop */
void zero_extract_loop(volatile unsigned int *data, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Various extractions to increase chances */
        unsigned int a = (*data >> 0) & 0xF;
        unsigned int b = (*data >> 4) & 0xF;
        unsigned int c = (*data >> 8) & 0xFF;
        unsigned int d = (*data >> 16) & 0xFFFF;
        
        /* Use results to prevent dead code elimination */
        g_global_array[i % 256] = a + b + c + d;
    }
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Pattern 1: Writing to low part of a larger variable */
void set_low_part_32(volatile uint32_t *p, uint16_t value) {
    /* This pattern may generate STRICT_LOW_PART when writing 16-bit to 32-bit */
    *p = (*p & 0xFFFF0000) | value;
}

/* Pattern 2: Using char assignment to int */
void set_low_byte(volatile uint32_t *p, uint8_t value) {
    /* Writing a byte to part of a word */
    *p = (*p & 0xFFFFFF00) | value;
}

/* Pattern 3: Pointer cast to smaller type */
void strict_low_part_via_cast(volatile uint64_t *p) {
    /* Cast to write only part of the data */
    *(uint32_t*)p = 0xDEADBEEF;
}

/* ==================== SUBREG Patterns ==================== */

/* Pattern 1: Union for type punning */
union subreg_union {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

uint16_t access_via_union(union subreg_union *u) {
    /* Accessing part of a larger register via union */
    u->halves[0] = 0x1234;
    return u->halves[1];
}

/* Pattern 2: Mixed-size operations */
int32_t mixed_size_ops(int64_t big_val) {
    /* Operations that require mode changes */
    int32_t part1 = (int32_t)big_val;
    int16_t part2 = (int16_t)(big_val >> 32);
    return part1 + part2;
}

/* Pattern 3: Array of different types */
void subreg_array_access(void) {
    uint64_t data[4];
    uint32_t *as_32bit = (uint32_t*)data;
    uint16_t *as_16bit = (uint16_t*)data;
    
    for (int i = 0; i < 8; i++) {
        as_32bit[i] = as_16bit[i*2] + as_16bit[i*2 + 1];
    }
}

/* ==================== Complex MEM Patterns ==================== */

/* Pattern 1: Complex addressing with multiple indices */
int complex_mem_address(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing mode: base + idx1 + idx2*4 + idx3*16 */
    return base[idx1 + idx2*4 + idx3*16];
}

/* Pattern 2: Struct with array */
struct mem_struct {
    int preamble[10];
    int data[100];
    int postamble[10];
};

int access_struct_array(struct mem_struct *s, int i, int j) {
    /* Nested array access within struct */
    return s->data[i * 10 + j];
}

/* Pattern 3: Pointer arithmetic in loop */
void mem_pointer_arithmetic(int *arr, int size) {
    int *end = arr + size;
    int *ptr = arr;
    int sum = 0;
    
    while (ptr < end) {
        sum += *ptr;
        ptr += (g_volatile_flag & 0x3) + 1;  /* Volatile step */
    }
    
    g_global_int = sum;
}

/* ==================== Combined Function ==================== */

/* Function that combines multiple patterns with control flow */
void combined_patterns(volatile int trigger) {
    union subreg_union u;
    struct bitfield_struct bf;
    struct mem_struct ms;
    
    /* Initialize */
    u.full = 0x87654321;
    bf.middle = 0xAB;
    
    /* Complex control flow based on volatile */
    if (trigger & 0x1) {
        /* ZERO_EXTRACT patterns */
        unsigned int extracted = extract_bits_shift(&g_global_int);
        extracted += extract_bitfield(&bf);
        
        /* Use in memory access */
        g_global_array[extracted % 256] = extracted;
    }
    
    if (trigger & 0x2) {
        /* STRICT_LOW_PART patterns */
        set_low_part_32(&g_global_int, 0x5678);
        set_low_byte((volatile uint32_t*)&g_global_int, 0x9A);
    }
    
    if (trigger & 0x4) {
        /* SUBREG patterns */
        uint16_t half = access_via_union(&u);
        int32_t mixed = mixed_size_ops(0x123456789ABCDEF0LL);
        
        /* Use results */
        g_global_array[half % 256] = mixed;
    }
    
    if (trigger & 0x8) {
        /* Complex MEM patterns */
        int idx = complex_mem_address(g_global_array, 
                                     g_volatile_counter % 10,
                                     (g_volatile_counter >> 4) % 10,
                                     (g_volatile_counter >> 8) % 10);
        
        access_struct_array(&ms, idx % 10, (idx >> 4) % 10);
    }
}

/* ==================== Main Function ==================== */

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        g_global_array[i] = i * 3;
    }
    
    /* Create a checksum to ensure all code has observable effect */
    unsigned int checksum = 0;
    
    /* Loop with volatile control to ensure all paths might be taken */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Call individual pattern functions */
        checksum += extract_bits_shift(&g_global_int);
        
        set_low_part_32(&g_global_int, g_volatile_counter & 0xFFFF);
        
        union subreg_union u;
        u.full = g_volatile_counter * 0x10001;
        checksum += access_via_union(&u);
        
        /* Complex memory access */
        checksum += complex_mem_address(g_global_array,
                                       g_volatile_counter % 64,
                                       (g_volatile_counter >> 6) % 64,
                                       0);
        
        /* Combined function with varying triggers */
        combined_patterns(g_volatile_counter);
        
        /* Memory pointer arithmetic */
        mem_pointer_arithmetic(g_global_array, 100);
        
        /* Zero extract in loop */
        zero_extract_loop(&g_global_int, 10);
        
        /* Subreg array access */
        subreg_array_access();
    }
    
    /* Use checksum to prevent optimization */
    printf("Result checksum: %u\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
