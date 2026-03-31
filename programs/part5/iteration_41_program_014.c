/* test_resource_coverage.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_coverage.c -o test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all test_resource_coverage.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple extractions to prevent optimization */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift (common ZERO_EXTRACT) */
    result += (x >> shift) & 0x1F;  /* Extract 5 bits */
    
    /* Pattern 2: Multiple extractions with different widths */
    result += (x >> 8) & 0xFF;      /* Extract byte */
    result += (x >> 16) & 0x7;      /* Extract 3 bits */
    
    /* Pattern 3: Variable width extraction */
    unsigned int mask = (1 << (shift & 0x7)) - 1;
    result += (x >> 4) & mask;
    
    return result;
}

/* Bit-field structure operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignment (can generate ZERO_EXTRACT in SET_DEST) */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0xFF;
    
    /* Bit-field comparison and extraction */
    if (s->field3 == 2) {
        result += 1;
    }
    
    /* Combine multiple bit-fields */
    result += s->field1;
    result += s->field2 << 3;
    result += s->field4 >> 8;
    
    return result;
}

/* Complex bit-field expression */
unsigned int test_zero_extract_complex(unsigned int x, unsigned int y) {
    /* Nested extractions and operations */
    unsigned int a = (x & 0xFF00) >> 8;
    unsigned int b = (y & 0xF0) >> 4;
    unsigned int c = (x >> 16) & 0x3FF;
    
    /* Mix with arithmetic */
    return (a * b) + (c << 2) + ((x & 0x7) << 5);
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* char assignment in loop - may use partial register */
        char c = chars[i] + i;
        result += c;
        
        /* short assignment - partial 16-bit update */
        short s = shorts[i] * 2;
        result += s;
        
        /* Mix types to force promotions and partial writes */
        int temp = c + s;
        chars[i] = temp & 0xFF;          /* STRICT_LOW_PART for byte store */
        shorts[i] = (temp >> 8) & 0xFFFF; /* STRICT_LOW_PART for short store */
    }
    
    return result;
}

/* Volatile partial writes */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write of short - preserves high bits in register */
        *ptr = (short)(i * 3);
        sum += *ptr;  /* Read back to prevent elimination */
        
        /* Pointer arithmetic with different types */
        volatile char *cptr = (volatile char *)ptr;
        cptr[1] = (char)(i & 0xFF);  /* Byte write to odd address */
    }
    
    return sum;
}

/* Function with small integer parameters */
unsigned int test_strict_low_part_args(short s1, char c1, short s2, char c2) {
    /* Local modifications of arguments */
    s1 = s1 + c1;
    c2 = c2 - s2;
    
    /* Mix in larger computations */
    int temp = (int)s1 * (int)c2;
    
    /* Partial writes back to smaller types */
    s2 = (temp >> 8) & 0xFFFF;
    c1 = temp & 0xFF;
    
    return s1 + s2 + c1 + c2;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t  b[4];
};

unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data */
        result += u[i].s[0];      /* SUBREG for half-word access */
        result += u[i].b[1] << 8; /* SUBREG for byte access */
        
        /* Modify through one view, read through another */
        u[i].s[1] = result & 0xFFFF;
        result += u[i].i;         /* Full word access */
    }
    
    return result;
}

/* Casting between integer sizes */
unsigned int test_subreg_casts(unsigned int *data, int len) {
    unsigned int sum = 0;
    
    for (int i = 0; i < len; i++) {
        /* Multiple casts generating SUBREGs */
        uint16_t low = (uint16_t)(data[i] & 0xFFFF);
        uint16_t high = (uint16_t)((data[i] >> 16) & 0xFFFF);
        
        /* Recombine with new computation */
        uint32_t combined = (uint32_t)low * (uint32_t)high;
        
        /* Extract bytes */
        uint8_t b0 = (uint8_t)(combined & 0xFF);
        uint8_t b1 = (uint8_t)((combined >> 8) & 0xFF);
        
        sum += b0 + (b1 << 8) + low + high;
    }
    
    return sum;
}

/* Packed structure */
struct __attribute__((packed)) packed_data {
    uint16_t a;
    uint8_t  b;
    uint16_t c;
    uint8_t  d;
};

unsigned int test_subreg_packed(struct packed_data *p, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access packed fields - may require SUBREG accesses */
        result += p[i].a;
        result += p[i].b << 4;
        result += p[i].c * 3;
        result += p[i].d;
        
        /* Modify fields */
        p[i].a = (result >> 8) & 0xFFFF;
        p[i].b = result & 0xFF;
    }
    
    return result;
}

/* ========== Complex memory references ========== */

/* Memory references with addressing modes */
unsigned int test_complex_memory(int *base, int *indexes, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex addressing: base + index computation */
        int *ptr = base + indexes[i];
        
        /* Access with offset */
        sum += ptr[0] + ptr[1] + ptr[-1];
        
        /* Bit-field extraction from memory */
        sum += (*ptr >> 4) & 0xF;
        
        /* Partial write to memory location */
        *(volatile short *)((char *)ptr + 2) = (short)(sum & 0xFFFF);
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int data_size = 100;
    unsigned int *int_data = malloc(data_size * sizeof(unsigned int));
    struct bitfield_struct bf_struct;
    union type_pun *unions = malloc(50 * sizeof(union type_pun));
    struct packed_data *packed = malloc(30 * sizeof(struct packed_data));
    char *char_data = malloc(data_size * sizeof(char));
    short *short_data = malloc(data_size * sizeof(short));
    int *index_array = malloc(data_size * sizeof(int));
    
    /* Initialize with pseudo-random data using volatile seed */
    for (int i = 0; i < data_size; i++) {
        int_data[i] = (i * 1103515245 + g_volatile_seed) & 0xFFFFFFFF;
        char_data[i] = (char)(int_data[i] & 0xFF);
        short_data[i] = (short)(int_data[i] & 0xFFFF);
        index_array[i] = (i * 3) % (data_size / 2);
        
        if (i < 50) {
            unions[i].i = int_data[i];
        }
        if (i < 30) {
            packed[i].a = int_data[i] & 0xFFFF;
            packed[i].b = (int_data[i] >> 16) & 0xFF;
            packed[i].c = (int_data[i] >> 8) & 0xFFFF;
            packed[i].d = (int_data[i] >> 24) & 0xFF;
        }
    }
    
    bf_struct.field1 = 5;
    bf_struct.field2 = 127;
    bf_struct.field3 = 2;
    bf_struct.field4 = 0xABCD;
    
    /* Run all tests to trigger different RTL patterns */
    
    /* ZERO_EXTRACT tests */
    final_result += test_zero_extract_int(int_data[0], 3);
    final_result += test_zero_extract_struct(&bf_struct, int_data[1]);
    final_result += test_zero_extract_complex(int_data[2], int_data[3]);
    
    /* STRICT_LOW_PART tests */
    final_result += test_strict_low_part_chars(char_data, short_data, 20);
    final_result += test_strict_low_part_volatile((volatile short *)&short_data[10], 15);
    final_result += test_strict_low_part_args(1000, 50, 2000, 100);
    
    /* SUBREG tests */
    final_result += test_subreg_union(unions, 25);
    final_result += test_subreg_casts(int_data, 20);
    final_result += test_subreg_packed(packed, 15);
    
    /* Complex memory reference test */
    final_result += test_complex_memory(int_data, index_array, 25);
    
    /* Additional mixed test to ensure all patterns are exercised */
    for (int i = 0; i < 10; i++) {
        /* Mix bit-field, partial write, and SUBREG in one loop */
        unsigned int val = int_data[i];
        
        /* ZERO_EXTRACT pattern */
        unsigned int extracted = (val >> (i & 0x7)) & ((1 << 5) - 1);
        
        /* STRICT_LOW_PART pattern (through char pointer) */
        char *cptr = (char *)&val;
        cptr[1] = extracted & 0xFF;
        
        /* SUBREG pattern through union */
        union type_pun u;
        u.i = val;
        final_result += u.s[0] + u.b[2];
    }
    
    /* Clean up */
    free(int_data);
    free(unions);
    free(packed);
    free(char_data);
    free(short_data);
    free(index_array);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
