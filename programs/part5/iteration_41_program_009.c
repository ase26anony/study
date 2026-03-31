/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x) {
    unsigned int sum = 0;
    /* Multiple bit-field extractions with varying widths */
    sum += (x >> 0) & 0x1F;      /* Extract bits 0-4 */
    sum += (x >> 5) & 0x3F;      /* Extract bits 5-10 */
    sum += (x >> 11) & 0x7FF;    /* Extract bits 11-21 */
    sum += (x >> 22) & 0x3FF;    /* Extract bits 22-31 */
    return sum;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s) {
    unsigned int sum = 0;
    /* Bit-field comparisons and arithmetic */
    if (s->field1 == 3) {
        sum += s->field1 * 2;
    }
    if (s->field2 > 10) {
        sum += s->field2 - 5;
    }
    sum += s->field3 << 2;
    sum += s->field4 >> 1;
    
    /* Bit-field assignment */
    s->field1 = (sum & 0x1F);
    s->field2 = ((sum >> 5) & 0x7F);
    
    return sum;
}

/* Complex bit-field expression */
unsigned int test_zero_extract_complex(unsigned int x) {
    /* Combined mask and shift operations */
    unsigned int part1 = (x & 0x0000FF00) >> 8;
    unsigned int part2 = (x & 0x00FF0000) >> 16;
    unsigned int part3 = (x & 0xFF000000) >> 24;
    
    /* Nested extractions */
    return ((part1 & 0x0F) << 4) | 
           ((part2 & 0x1F) << 9) | 
           ((part3 & 0x07) << 14);
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *data, int len) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < len; i++) {
        /* char assignment in loop - may generate partial register write */
        char temp = data[i] + g_volatile_seed;
        checksum += temp;
        
        /* Force register promotion and partial writeback */
        short promoted = temp * 2;
        checksum += promoted;
        
        /* Cast to smaller type */
        char truncated = (char)(promoted & 0xFF);
        checksum += truncated;
    }
    
    return checksum;
}

/* Volatile pointer to force strict low part */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Volatile write of short - may use STRICT_LOW_PART */
        *ptr = (short)(i + g_volatile_seed);
        
        /* Read back and use */
        short val = *ptr;
        sum += val;
        
        /* Pointer arithmetic */
        ptr++;
    }
    
    return sum;
}

/* Inline assembly for byte register access */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    unsigned int input = g_volatile_seed;
    
    /* Assembly that works with byte registers */
    asm volatile (
        "movb %1, %%al\n\t"
        "addb $5, %%al\n\t"
        "movb %%al, %0"
        : "=r"(result)
        : "r"(input)
        : "%al"
    );
    
    return result;
}

/* Function with small integer parameters */
unsigned int test_strict_low_part_args(short s, char c) {
    /* Parameters may be passed in wider registers */
    unsigned int sum = s + c;
    
    /* Modify parameters locally */
    s = (short)(s * 2 + 1);
    c = (char)(c + 5);
    
    sum += s;
    sum += c;
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type punning */
unsigned int test_subreg_union(unsigned int value) {
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    u.full = value;
    
    /* Access sub-parts through union */
    unsigned int sum = u.halves[0] + u.halves[1];
    sum += u.bytes[0] + u.bytes[1] + u.bytes[2] + u.bytes[3];
    
    /* Modify through sub-parts */
    u.halves[1] = (u.halves[0] + g_volatile_seed) & 0xFFFF;
    
    return sum + u.full;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(int *array, int size) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Cast to smaller type and back */
        short s = (short)(array[i] & 0xFFFF);
        char c = (char)(array[i] & 0xFF);
        
        checksum += s;
        checksum += c;
        
        /* Cast in expression */
        checksum += (unsigned short)(array[i] >> 16);
    }
    
    return checksum;
}

/* SIMD-like operations using subregs */
unsigned int test_subreg_simd(unsigned int packed) {
    /* Treat 32-bit as packed 8-bit values */
    unsigned int b0 = (packed >> 0) & 0xFF;
    unsigned int b1 = (packed >> 8) & 0xFF;
    unsigned int b2 = (packed >> 16) & 0xFF;
    unsigned int b3 = (packed >> 24) & 0xFF;
    
    /* Operations on sub-parts */
    b0 = (b0 + 1) & 0xFF;
    b1 = (b1 * 2) & 0xFF;
    b2 = (b2 ^ 0x55) & 0xFF;
    b3 = (b3 - 5) & 0xFF;
    
    /* Re-pack */
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

/* ========== Memory references with complex addresses ========== */

/* Array access with indexing */
unsigned int test_mem_complex_addr(int *base, int *offsets, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex address calculation */
        int *addr = base + offsets[i];
        
        /* Access through pointer */
        sum += *addr;
        
        /* Modify through pointer with offset */
        *(addr + 1) = sum & 0xFF;
    }
    
    return sum;
}

/* Structure with mixed types */
struct mixed_struct {
    int a;
    short b;
    char c;
    int d;
};

unsigned int test_mem_struct(struct mixed_struct *arr, int count) {
    unsigned int total = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access structure members - may involve SUBREG for short/char */
        total += arr[i].a;
        total += arr[i].b;  /* short access */
        total += arr[i].c;  /* char access */
        
        /* Modify members */
        arr[i].b = (short)(total & 0xFFFF);
        arr[i].c = (char)(total & 0xFF);
    }
    
    return total;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data */
    int data_array[100];
    short short_array[50];
    char char_array[200];
    struct bitfield_struct bf_struct = {3, 20, 500, 600};
    struct mixed_struct mixed_arr[10];
    int offsets[20];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        data_array[i] = (i * 37 + g_volatile_seed) & 0xFFFFFFFF;
    }
    for (int i = 0; i < 50; i++) {
        short_array[i] = (short)((i * 13 + g_volatile_seed) & 0xFFFF);
    }
    for (int i = 0; i < 200; i++) {
        char_array[i] = (char)((i * 7 + g_volatile_seed) & 0xFF);
    }
    for (int i = 0; i < 10; i++) {
        mixed_arr[i].a = i * 100;
        mixed_arr[i].b = (short)(i * 20);
        mixed_arr[i].c = (char)(i * 3);
        mixed_arr[i].d = i * 1000;
    }
    for (int i = 0; i < 20; i++) {
        offsets[i] = (i * 2) % 50;
    }
    
    /* Test ZERO_EXTRACT patterns */
    final_result += test_zero_extract_int(g_volatile_seed);
    final_result += test_zero_extract_struct(&bf_struct);
    final_result += test_zero_extract_complex(g_volatile_seed * 17);
    
    /* Test STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(char_array, 200);
    final_result += test_strict_low_part_volatile(short_array, 50);
    final_result += test_strict_low_part_asm();
    final_result += test_strict_low_part_args(1000, 50);
    
    /* Test SUBREG patterns */
    final_result += test_subreg_union(g_volatile_seed * 23);
    final_result += test_subreg_casts(data_array, 100);
    final_result += test_subreg_simd(g_volatile_seed * 19);
    
    /* Test memory reference patterns */
    final_result += test_mem_complex_addr(data_array, offsets, 20);
    final_result += test_mem_struct(mixed_arr, 10);
    
    /* Use result to prevent dead code elimination */
    if (argc > 1) {
        printf("Result: %u\n", final_result);
    }
    
    return (int)(final_result & 0x7FFFFFFF);
}
