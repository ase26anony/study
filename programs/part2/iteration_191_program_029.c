/* This program is designed to trigger specific RTL patterns in GCC's resource
   tracking pass, particularly targeting the uncovered lines in resource.cc
   that handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM operands. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays and structs for memory operand patterns */
unsigned int g_data_array[256];
int g_large_buffer[1024];

/* Struct with bit-fields for ZERO_EXTRACT patterns */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int64_t full64;
    int32_t parts32[2];
    int16_t parts16[4];
    int8_t  parts8[8];
};

/* Struct with array for complex MEM addressing */
struct ArrayContainer {
    int data[50];
    int padding[10];
};

/* ==================== PATTERN 1: ZERO_EXTRACT ==================== */
/* Bit-field extraction that should generate ZERO_EXTRACT in RTL */
unsigned int pattern_zero_extract(struct BitFieldStruct *bfs) {
    /* Multiple bit-field accesses with volatile guard */
    unsigned int result = 0;
    if (g_volatile_flag) {
        result = bfs->low8;          /* 8-bit extract from bit position 0 */
        result += bfs->mid8 << 8;    /* 8-bit extract from bit position 8 */
        result += bfs->high16 << 16; /* 16-bit extract from bit position 16 */
    } else {
        /* Alternative extract using shift/mask */
        volatile unsigned int *p = &g_volatile_counter;
        result = (*p >> 4) & 0x0F;   /* Extract bits 4-7 */
    }
    return result;
}

/* Another ZERO_EXTRACT variant using explicit mask/shift */
unsigned int extract_bits_complex(volatile unsigned int *arr, int idx) {
    /* Complex addressing with bit extraction */
    unsigned int val = arr[idx % 256];
    /* Extract three non-contiguous bit ranges */
    unsigned int part1 = (val >> 2) & 0x3;   /* bits 2-3 */
    unsigned int part2 = (val >> 8) & 0xF;   /* bits 8-11 */
    unsigned int part3 = (val >> 16) & 0xFF; /* bits 16-23 */
    return part1 + part2 + part3;
}

/* ==================== PATTERN 2: STRICT_LOW_PART ==================== */
/* Operations that write only to low parts of registers */
void pattern_strict_low_part(volatile unsigned int *dest, unsigned char value) {
    /* Write only low byte while preserving high bytes */
    if (g_volatile_flag > 0) {
        *dest = (*dest & ~0xFF) | value;  /* Only low 8 bits changed */
    }
    
    /* Another pattern: writing to short within int */
    volatile uint32_t *p32 = dest;
    uint16_t low_half = value * 257;  /* Expand to 16 bits */
    /* Cast to induce partial write */
    *(uint16_t *)p32 = low_half;      /* Write only low 16 bits */
}

/* STRICT_LOW_PART through pointer casting */
void write_low_part_via_cast(int32_t *ptr) {
    volatile int16_t *ptr16 = (volatile int16_t *)ptr;
    *ptr16 = g_volatile_counter & 0xFFFF;  /* Write only low 16 bits */
}

/* ==================== PATTERN 3: SUBREG ==================== */
/* Operations that access parts of larger types */
int32_t pattern_subreg(union MixedSizeUnion *u) {
    int32_t result = 0;
    
    /* Access 32-bit part of 64-bit union */
    result += u->parts32[0];
    
    /* Access 16-bit part of 64-bit union */
    result += u->parts16[2];
    
    /* Mixed-size access pattern */
    if (g_volatile_flag) {
        int64_t temp = u->full64;
        int32_t low = (int32_t)temp;          /* Extract low 32 bits */
        int32_t high = (int32_t)(temp >> 32); /* Extract high 32 bits */
        result = low - high;
    }
    
    return result;
}

/* SUBREG through pointer type punning */
int16_t subreg_via_pointers(int64_t *large) {
    /* Access different-sized views of same memory */
    int32_t *as_32bit = (int32_t *)large;
    int16_t *as_16bit = (int16_t *)large;
    
    return as_16bit[1] + as_32bit[0] % 256;
}

/* ==================== PATTERN 4: COMPLEX MEM OPERANDS ==================== */
/* Memory accesses with non-trivial addressing */
int pattern_complex_mem(struct ArrayContainer *container, int idx1, int idx2) {
    int result = 0;
    
    /* Complex addressing with multiple computations */
    result += container->data[idx1 * 3 + idx2];
    
    /* More complex: array with struct offset */
    result += g_large_buffer[idx1 * 16 + idx2 * 4 + 8];
    
    /* Even more complex: pointer arithmetic in loop */
    int *ptr = &container->data[0];
    for (int i = 0; i < 5; i++) {
        result += ptr[idx1 + i * idx2];
    }
    
    return result;
}

/* MEM with symbolic address calculation */
int* get_complex_address(int base[], int offset1, int offset2) {
    /* Address calculation that may create complex MEM in RTL */
    return &base[offset1 * 8 + offset2 * 2 + 4];
}

/* ==================== MAIN FUNCTION ==================== */
/* Combines all patterns with control flow */
int main(void) {
    /* Initialize data */
    struct BitFieldStruct bfs = {0xAA, 0xBB, 0xCCDD};
    union MixedSizeUnion u;
    u.full64 = 0x0123456789ABCDEFULL;
    struct ArrayContainer container;
    
    for (int i = 0; i < 50; i++) {
        container.data[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        g_data_array[i] = i * 0x01010101;
    }
    
    unsigned int checksum = 0;
    
    /* Loop with volatile condition to create complex control flow */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Mix all patterns with conditional execution */
        if (g_volatile_counter % 3 == 0) {
            checksum ^= pattern_zero_extract(&bfs);
        }
        
        if (g_volatile_counter % 5 == 0) {
            pattern_strict_low_part(&g_data_array[g_volatile_counter % 256], 
                                   (unsigned char)g_volatile_counter);
        }
        
        if (g_volatile_counter % 7 == 0) {
            checksum += pattern_subreg(&u);
        }
        
        if (g_volatile_counter % 11 == 0) {
            checksum += pattern_complex_mem(&container, 
                                           g_volatile_counter % 10,
                                           (g_volatile_counter / 10) % 5);
        }
        
        /* Additional mixed patterns */
        checksum += extract_bits_complex(g_data_array, g_volatile_counter % 128);
        
        write_low_part_via_cast((int32_t*)&g_data_array[g_volatile_counter % 64]);
        
        checksum += subreg_via_pointers((int64_t*)&g_data_array[g_volatile_counter % 32]);
        
        /* Use complex address calculation */
        int *addr = get_complex_address(g_large_buffer, 
                                       g_volatile_counter % 8,
                                       (g_volatile_counter / 8) % 8);
        checksum += *addr;
    }
    
    /* Final computation to ensure all code has observable effect */
    printf("Final checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
