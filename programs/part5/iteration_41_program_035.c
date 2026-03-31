/* test_resource_patterns.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_patterns.c -o test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all test_resource_patterns.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift, unsigned int mask) {
    /* This should generate ZERO_EXTRACT for bit-field extraction */
    unsigned int result = 0;
    
    /* Multiple extraction patterns */
    result += (x >> shift) & mask;                    /* Simple extract */
    result += (x & 0xFF00) >> 8;                      /* Mask then shift */
    result += ((x >> 4) & 0x0F0F0F0F) + ((x >> 8) & 0x00FF00FF);
    
    /* Prevent optimization */
    if (g_volatile_seed & 1) {
        result = (result >> 2) & 0x3FFFFFFF;
    }
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x07;
    
    /* Bit-field comparisons */
    if (s->field1 == 10) result += 1;
    if (s->field2 > 50) result += 2;
    if (s->field3 != 0) result += 4;
    
    /* Complex bit-field expression */
    result += (s->field1 << s->field3) | (s->field2 >> 1);
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        char c = chars[i];
        short s = shorts[i];
        
        /* Promote and modify */
        int temp_c = c;
        temp_c = (temp_c * 3 + 1) & 0xFF;  /* Only low 8 bits matter */
        chars[i] = temp_c;                 /* Partial write back */
        
        int temp_s = s;
        temp_s = (temp_s * 5 - 2) & 0xFFFF; /* Only low 16 bits matter */
        shorts[i] = temp_s;                 /* Partial write back */
        
        checksum += temp_c + temp_s;
    }
    
    return checksum;
}

/* Volatile pointer writes */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write of short - should preserve high bits if in register */
        *ptr = (short)(i * 7 + g_volatile_seed);
        sum += *ptr;  /* Read back */
        
        /* Change pointer slightly to prevent optimization */
        ptr = (volatile short *)((char *)ptr + 1);
    }
    
    return sum;
}

/* Inline assembly for byte register access */
unsigned int test_strict_low_part_asm(unsigned char *data, int len) {
    unsigned int result = 0;
    
    for (int i = 0; i < len; i++) {
        unsigned char val = data[i];
        unsigned char new_val;
        
        /* Inline assembly that operates on byte register */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %0"
            : "=r" (new_val)
            : "r" (val)
            : "%al"
        );
        
        data[i] = new_val;
        result += new_val;
    }
    
    return result;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t  b[4];
};

unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different parts of the same register */
        sum += u[i].s[0];      /* Low 16 bits */
        sum += u[i].s[1];      /* High 16 bits */
        sum += u[i].b[2];      /* Third byte */
        
        /* Modify through one view, read through another */
        u[i].s[1] = (u[i].s[0] * 3) & 0xFFFF;
        sum += u[i].i;         /* Read full 32-bit */
    }
    
    return sum;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(int *ints, short *shorts, int count) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Casts that may generate SUBREG */
        short s = (short)(ints[i] & 0xFFFF);
        char c = (char)(ints[i] >> 16);
        
        /* Operations on the cast results */
        shorts[i] = s + c;
        checksum += (int)s * 2 + (int)c * 3;
        
        /* More complex casting chain */
        int temp = ints[i];
        short low = (short)temp;
        short high = (short)(temp >> 16);
        checksum += low * high;
    }
    
    return checksum;
}

/* ========== Complex memory references ========== */

/* Structure with mixed types for complex addressing */
struct mixed_data {
    int a;
    short b;
    char c;
    int d;
};

unsigned int test_complex_memory(struct mixed_data *data, int *indices, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        int idx = indices[i] & 0xF;  /* Prevent out-of-bounds */
        
        /* Complex memory addressing */
        result += data[idx].a;
        result += data[idx].b * 2;
        result += data[idx].c * 3;
        
        /* Modify through pointer with offset */
        short *ptr = &data[idx].b;
        *ptr = (*ptr + result) & 0x7FFF;
        
        /* Array-like access within structure */
        char *char_ptr = (char *)&data[idx];
        for (int j = 0; j < 4; j++) {
            result += char_ptr[j];
        }
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int data_size = 100;
    unsigned int *int_data = malloc(data_size * sizeof(unsigned int));
    short *short_data = malloc(data_size * sizeof(short));
    char *char_data = malloc(data_size * sizeof(char));
    struct bitfield_struct *bfs = malloc(10 * sizeof(struct bitfield_struct));
    union type_pun *unions = malloc(20 * sizeof(union type_pun));
    struct mixed_data *mixed = malloc(16 * sizeof(struct mixed_data));
    int *indices = malloc(data_size * sizeof(int));
    
    /* Seed with pseudo-random but compiler-unpredictable values */
    for (int i = 0; i < data_size; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        short_data[i] = (short)(int_data[i] & 0xFFFF);
        char_data[i] = (char)(int_data[i] & 0xFF);
        indices[i] = i;
    }
    
    for (int i = 0; i < 10; i++) {
        bfs[i].field1 = i & 0x1F;
        bfs[i].field2 = (i * 3) & 0x7F;
        bfs[i].field3 = (i >> 2) & 0x07;
        bfs[i].field4 = i * 1000;
    }
    
    for (int i = 0; i < 20; i++) {
        unions[i].i = i * 0x01020304;
    }
    
    for (int i = 0; i < 16; i++) {
        mixed[i].a = i * 100;
        mixed[i].b = (short)(i * 50);
        mixed[i].c = (char)(i * 10);
        mixed[i].d = i * 1000;
    }
    
    /* Run all tests to trigger different RTL patterns */
    final_result += test_zero_extract_int(int_data[0], 4, 0x0F0F0F0F);
    final_result += test_zero_extract_struct(&bfs[g_volatile_seed % 10], 
                                            g_volatile_seed);
    
    final_result += test_strict_low_part_chars(char_data, short_data, data_size / 2);
    final_result += test_strict_low_part_volatile((volatile short *)&short_data[10], 20);
    final_result += test_strict_low_part_asm((unsigned char *)char_data, data_size / 4);
    
    final_result += test_subreg_union(unions, 10);
    final_result += test_subreg_casts(int_data, short_data, data_size / 4);
    
    final_result += test_complex_memory(mixed, indices, 15);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    
    /* Cleanup */
    free(int_data);
    free(short_data);
    free(char_data);
    free(bfs);
    free(unions);
    free(mixed);
    free(indices);
    
    return (final_result & 0xFF);  /* Return non-zero to indicate success */
}
