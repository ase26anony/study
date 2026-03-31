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
#include <stdlib.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global arrays/structs for memory operand patterns */
int g_array[256];
struct BitFieldStruct {
    unsigned int low_bits : 8;
    unsigned int high_bits : 8;
    unsigned int pad : 16;
} g_bitfield;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
    int8_t bytes[8];
} g_union;

/* ========== ZERO_EXTRACT Patterns ========== */
/* Method 1: Explicit bit-field extraction */
unsigned int extract_low_byte(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 0-7 */
    return (*p) & 0xFF;
}

/* Method 2: Bit-field struct member access */
unsigned int read_bitfield(struct BitFieldStruct *bfs) {
    /* Accessing bit-field members often creates ZERO_EXTRACT */
    unsigned int val = bfs->low_bits;
    val |= (bfs->high_bits << 8);
    return val;
}

/* Method 3: Shift-and-mask pattern */
unsigned int extract_middle_bits(volatile unsigned int *p) {
    /* Extract bits 8-15: (*p >> 8) & 0xFF */
    return (*p >> 8) & 0xFF;
}

/* ========== STRICT_LOW_PART Patterns ========== */
/* Method 1: Partial write to 32-bit via 16-bit pointer */
void write_low_half(volatile uint32_t *p, uint16_t value) {
    /* Cast to smaller type for partial write */
    *(volatile uint16_t *)p = value;
}

/* Method 2: Mask-and-set low byte pattern */
void set_low_byte_only(volatile uint32_t *p, uint8_t byte_val) {
    /* Clear low byte, then set it */
    *p = (*p & ~0xFF) | byte_val;
}

/* Method 3: Inline assembly for explicit low-part write (x86) */
#ifdef __x86_64__
void asm_low_part_write(volatile uint32_t *p, uint16_t val) {
    /* "movw" instruction writes only low 16 bits */
    asm volatile ("movw %w1, %0" : "=m"(*p) : "r"(val));
}
#endif

/* ========== SUBREG Patterns ========== */
/* Method 1: Union-based type punning */
int32_t access_via_subreg_union(int64_t value) {
    g_union.full = value;
    /* Accessing halves[0] creates SUBREG from the full register */
    return g_union.halves[0] + g_union.halves[1];
}

/* Method 2: Pointer casting between different sizes */
int16_t pointer_cast_subreg(volatile int32_t *src) {
    /* Cast 32-bit pointer to 16-bit pointer */
    int16_t low = *(volatile int16_t *)src;
    int16_t high = *((volatile int16_t *)src + 1);
    return low + high;
}

/* Method 3: Array element with different type size */
int8_t mixed_size_array_access(volatile int32_t arr[], int index) {
    /* Access as 8-bit when array is 32-bit */
    volatile int8_t *byte_ptr = (volatile int8_t *)&arr[index];
    return byte_ptr[0] + byte_ptr[3];
}

/* ========== Complex MEM Patterns ========== */
/* Method 1: Array access with complex index calculation */
int complex_mem_access(int *base, int idx1, int idx2, int idx3) {
    /* base[(idx1 + idx2) * idx3] creates addressing with arithmetic */
    return base[(idx1 + idx2) * idx3];
}

/* Method 2: Struct with array member and computed offset */
struct Container {
    int data[100];
    int metadata;
};

int struct_array_access(struct Container *c, int i, int j) {
    /* c->data[i + j*10] with struct pointer */
    return c->data[i + j * 10];
}

/* Method 3: Pointer arithmetic with multiple operations */
int pointer_arithmetic_mem(int *ptr, int a, int b, int c) {
    /* Complex address calculation: ptr[a + (b << 2) + c*3] */
    return ptr[a + (b << 2) + c * 3];
}

/* ========== Combined Function ========== */
/* This function combines multiple patterns in control flow
   to increase chance of hitting all target lines */
unsigned int combined_operations(volatile int mode) {
    unsigned int result = 0;
    static int call_count = 0;
    
    /* Use volatile flag for unpredictable control flow */
    if (g_volatile_flag) {
        /* ZERO_EXTRACT pattern */
        volatile unsigned int extract_src = 0xABCD1234;
        result += extract_low_byte(&extract_src);
        result += extract_middle_bits(&extract_src);
        
        /* Update bitfield struct */
        g_bitfield.low_bits = result & 0xFF;
        result += read_bitfield(&g_bitfield);
    }
    
    if (mode & 1) {
        /* STRICT_LOW_PART patterns */
        volatile uint32_t low_part_target = 0xFFFFFFFF;
        write_low_half(&low_part_target, 0x1234);
        result += low_part_target & 0xFFFF;
        
        set_low_byte_only(&low_part_target, 0xAB);
        result += low_part_target & 0xFF;
        
        #ifdef __x86_64__
        asm_low_part_write(&low_part_target, 0xCDEF);
        result += low_part_target & 0xFFFF;
        #endif
    }
    
    if (mode & 2) {
        /* SUBREG patterns */
        result += access_via_subreg_union(0x1122334455667788LL);
        
        volatile int32_t subreg_src = 0xAABBCCDD;
        result += pointer_cast_subreg(&subreg_src);
        
        volatile int32_t arr[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
        result += mixed_size_array_access(arr, 2);
    }
    
    /* Always do complex MEM patterns */
    int indices[3] = {1, 2, 3};
    result += complex_mem_access(g_array, 
                                 indices[0], 
                                 indices[1], 
                                 indices[2]);
    
    struct Container cont;
    for (int i = 0; i < 10; i++) {
        cont.data[i] = i * 100;
    }
    result += struct_array_access(&cont, 1, 2);
    
    result += pointer_arithmetic_mem(g_array, 10, 20, 30);
    
    call_count++;
    return result + call_count;
}

/* ========== Main Driver ========== */
int main(void) {
    unsigned int final_result = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Initialize union */
    g_union.full = 0x0123456789ABCDEFLL;
    
    /* Initialize bitfield struct */
    g_bitfield.low_bits = 0xAA;
    g_bitfield.high_bits = 0xBB;
    
    /* Loop with volatile condition to create multiple basic blocks */
    for (g_volatile_counter = 0; g_volatile_counter < 100; g_volatile_counter++) {
        /* Vary the mode based on volatile counter */
        int mode = (g_volatile_counter % 4);
        
        /* Call combined function - this should generate all target RTL patterns */
        unsigned int iter_result = combined_operations(mode);
        
        /* Mix results to prevent dead code elimination */
        final_result ^= iter_result;
        final_result = (final_result << 1) | (final_result >> 31); /* rotate */
        
        /* Modify globals to affect future iterations */
        g_array[g_volatile_counter % 256] = iter_result;
        g_union.quarters[(g_volatile_counter % 4)] = iter_result & 0xFFFF;
    }
    
    /* Use result to prevent optimization */
    printf("Final result: %u (0x%08X)\n", final_result, final_result);
    
    return (int)(final_result % 256);
}
