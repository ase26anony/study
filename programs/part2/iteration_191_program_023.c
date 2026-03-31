/* Test program to generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM RTL patterns */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */

/* Bit-field extraction using shift/mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_shift(volatile unsigned int *p) {
    /* Multiple extractions to increase chances */
    unsigned int val = *p;
    unsigned int result = 0;
    
    /* Extract bits 8-15 */
    result |= (val >> 8) & 0xFF;
    
    /* Extract bits 16-23 */
    result |= ((val >> 16) & 0xFF) << 8;
    
    /* Extract bits 0-7 */
    result |= (val & 0xFF) << 16;
    
    return result;
}

/* Bit-field struct for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
};

unsigned int extract_bitfield(struct bitfield_struct *s) {
    /* Taking address and accessing bit-fields */
    unsigned int result = 0;
    result = s->field1;
    result |= s->field2 << 4;
    result |= s->field3 << 12;
    return result;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Writing only low part of a variable */
void write_low_part_32(volatile uint32_t *p, uint8_t value) {
    /* Clear low byte, then set it */
    *p = (*p & ~0xFFU) | (value & 0xFFU);
}

void write_low_part_16(volatile uint16_t *p, uint8_t value) {
    /* Similar for 16-bit */
    *p = (*p & ~0xFFU) | (value & 0xFFU);
}

/* Using casts to write partial data */
void write_with_cast(int32_t *dest, int16_t value) {
    /* This may generate STRICT_LOW_PART when writing to memory */
    *(int16_t*)dest = value;
}

/* ===== SUBREG patterns ===== */

/* Union for SUBREG patterns */
union mixed_types {
    int64_t full;
    int32_t half[2];
    int16_t quarter[4];
    int8_t bytes[8];
};

int32_t access_via_subreg(union mixed_types *u) {
    /* Access parts of larger type */
    int32_t result = 0;
    
    /* Access 32-bit part of 64-bit */
    result += u->half[0];
    
    /* Access 16-bit part */
    result += u->quarter[1];
    
    /* Access 8-bit part */
    result += u->bytes[3];
    
    return result;
}

/* Pointer casting for SUBREG */
int32_t cast_access(int64_t *large) {
    /* Cast to access part of larger type */
    return *(int32_t*)large;
}

/* ===== Complex MEM patterns ===== */

/* Struct with array for complex addressing */
struct complex_mem {
    int data[256];
    int padding[64];
    int more_data[128];
};

int complex_memory_access(struct complex_mem *cm, int idx1, int idx2, int idx3) {
    /* Complex addressing calculation */
    int result = 0;
    
    /* Base + index with scaling */
    result += cm->data[idx1 * 2];
    
    /* Base + multiple indices */
    result += cm->more_data[idx2 + idx3 * 4];
    
    /* Pointer arithmetic */
    int *ptr = &cm->data[0];
    ptr += idx1 + idx2 * 3;
    result += *ptr;
    
    return result;
}

/* Array with non-linear indexing */
int array_with_stride(int arr[][8], int i, int j, int k) {
    /* Multi-dimensional with mixed indices */
    return arr[i][j] + arr[j][k] + arr[k][i];
}

/* ===== Combined function with control flow ===== */

unsigned int combined_operations(void) {
    unsigned int result = 0;
    static union mixed_types u = { .full = 0x123456789ABCDEF0LL };
    static struct bitfield_struct bf = { 3, 0xAB, 0xCDE, 0xF };
    static struct complex_mem cm;
    static int md_array[16][8];
    
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        cm.data[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        cm.more_data[i] = i * 5;
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            md_array[i][j] = i * 8 + j;
        }
    }
    
    volatile int flag = g_volatile_flag;
    
    /* Loop with conditional operations */
    for (int i = 0; i < 10; i++) {
        if (flag & 1) {
            /* ZERO_EXTRACT patterns */
            result ^= extract_bits_shift(&g_volatile_counter);
            result += extract_bitfield(&bf);
        }
        
        if (flag & 2) {
            /* STRICT_LOW_PART patterns */
            write_low_part_32((volatile uint32_t*)&result, i & 0xFF);
            write_with_cast((int32_t*)&cm.data[i % 16], i & 0xFFFF);
        }
        
        if (flag & 4) {
            /* SUBREG patterns */
            result += access_via_subreg(&u);
            result += cast_access(&u.full);
        }
        
        if (flag & 8) {
            /* Complex MEM patterns */
            result += complex_memory_access(&cm, i % 64, (i + 1) % 64, (i + 2) % 64);
            result += array_with_stride(md_array, i % 8, (i + 1) % 8, (i + 2) % 8);
        }
        
        /* Modify volatile to change control flow */
        g_volatile_counter++;
        flag = g_volatile_counter;
    }
    
    return result;
}

/* Helper functions that emphasize specific patterns */
unsigned int emphasize_zero_extract(void) {
    volatile unsigned int source = 0xDEADBEEF;
    unsigned int result = 0;
    
    /* Multiple bit-field extractions */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } bits = { 7, 31, 1023, 16383 };
    
    result = bits.a;
    result |= bits.b << 3;
    result |= bits.c << 8;
    result |= bits.d << 18;
    
    /* Also use shift/mask */
    result ^= (source >> 4) & 0x0F0F0F0F;
    result ^= (source >> 8) & 0x00FF00FF;
    
    return result;
}

void emphasize_strict_low_part(void) {
    static volatile uint32_t data[4] = {0};
    
    /* Write to low parts only */
    for (int i = 0; i < 4; i++) {
        write_low_part_32(&data[i], i * 0x11);
        write_low_part_16((volatile uint16_t*)&data[i], i * 0x22);
    }
    
    /* Direct cast assignment */
    int32_t x;
    for (int i = 0; i < 4; i++) {
        *(int16_t*)&x = i * 0x333;
        data[i % 4] = x;
    }
}

/* Main function to drive everything */
int main(void) {
    unsigned int final_result = 0;
    
    printf("Starting resource pattern test...\n");
    
    /* Call combined function */
    final_result = combined_operations();
    
    /* Call specialized functions */
    final_result ^= emphasize_zero_extract();
    emphasize_strict_low_part();
    
    /* Additional mixed operations */
    union mixed_types mu;
    mu.full = 0x1122334455667788LL;
    
    for (int i = 0; i < 8; i++) {
        /* SUBREG access */
        final_result += mu.bytes[i];
        
        /* Conditional ZERO_EXTRACT */
        if (g_volatile_flag & (1 << (i % 4))) {
            final_result ^= (mu.full >> (i * 8)) & 0xFF;
        }
    }
    
    /* Complex memory access in loop */
    struct complex_mem local_cm;
    for (int i = 0; i < 100; i++) {
        int idx = (i * 7) % 256;
        local_cm.data[idx] = i;
        final_result += complex_memory_access(&local_cm, 
                                            idx % 64, 
                                            (idx + 1) % 64, 
                                            (idx + 2) % 64);
    }
    
    printf("Final result: %u (0x%08X)\n", final_result, final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
