/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management subsystem, specifically targeting:
 * - ZERO_EXTRACT expressions (bit-field operations)
 * - STRICT_LOW_PART expressions (partial register updates)
 * - SUBREG expressions (register sub-parts)
 * - Complex memory references
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function 1: Bit-field extraction from integers */
int test_zero_extract_int(int x, int shift, int mask_width) {
    int result = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; i++) {
        /* Varying shift to prevent constant folding */
        int current_shift = (shift + i) % 16;
        int mask = (1 << mask_width) - 1;
        
        /* Bit-field extract: (x >> shift) & mask */
        int extracted = (x >> current_shift) & mask;
        
        /* Use result to prevent dead code elimination */
        result ^= extracted;
        
        /* Another pattern: combined mask and shift */
        extracted = (x & 0xFF00) >> 8;
        result += extracted;
        
        /* Yet another: extract multiple fields */
        extracted = (x >> 16) & 0xFFFF;
        result |= extracted;
    }
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int value) {
    unsigned int result = 0;
    
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    s->field1 = value & 0x1F;
    s->field2 = (value >> 5) & 0x7F;
    s->field3 = (value >> 12) & 0x3FF;
    
    /* Bit-field comparison - should generate ZERO_EXTRACT */
    if (s->field1 == 0x10) {
        result += 1;
    }
    
    /* Complex bit-field arithmetic */
    result += s->field2 * 2;
    result += s->field3 >> 1;
    
    /* Nested bit-field operations */
    s->field4 = (s->field1 << 5) | s->field2;
    
    return result;
}

/* Function 3: Explicit bit-field operations on arrays */
int test_zero_extract_array(int *arr, int size) {
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various bit-field extraction patterns */
        int val = arr[i];
        
        /* Pattern 1: Extract 4-bit field starting at bit 8 */
        int field1 = (val >> 8) & 0xF;
        
        /* Pattern 2: Extract 6-bit field starting at bit 16 */
        int field2 = (val >> 16) & 0x3F;
        
        /* Pattern 3: Extract 10-bit field starting at bit 0 */
        int field3 = val & 0x3FF;
        
        /* Combine with arithmetic to prevent optimization */
        sum += field1 * field2 + field3;
        
        /* Update array with bit-field insert */
        arr[i] = (arr[i] & ~0xF00) | ((field1 << 8) & 0xF00);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 4: Partial register updates with small types */
int test_strict_low_part(short *short_arr, char *char_arr, int size) {
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART
         * when the values are promoted to int in registers */
        short s_val = short_arr[i];
        char c_val = char_arr[i];
        
        /* Partial register updates */
        s_val = (s_val + i) & 0x7FFF;
        c_val = (c_val * 3) & 0x7F;
        
        /* Store back - should preserve upper bits */
        short_arr[i] = s_val;
        char_arr[i] = c_val;
        
        /* Use volatile to force actual stores */
        if (g_volatile_seed > 0) {
            volatile short *vs = &short_arr[i];
            volatile char *vc = &char_arr[i];
            *vs = s_val + 1;
            *vc = c_val - 1;
        }
        
        result += s_val + c_val;
    }
    
    return result;
}

/* Function 5: Inline assembly for partial register access */
int test_strict_low_part_asm(int x) {
    short result_short = 0;
    char result_char = 0;
    
    /* Inline assembly that operates on partial registers */
    asm volatile (
        /* Move low 16 bits */
        "movw %1, %0\n\t"
        : "=r"(result_short)
        : "r"(x)
        : 
    );
    
    asm volatile (
        /* Move low 8 bits */
        "movb %b1, %b0\n\t"
        : "=r"(result_char)
        : "r"(x)
        : 
    );
    
    /* Force use of results */
    return result_short + result_char;
}

/* Function 6: Volatile pointer writes for partial stores */
int test_strict_low_part_volatile(volatile short *vs, volatile char *vc, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These volatile writes should generate STRICT_LOW_PART
         * for the store instructions */
        vs[i] = (i * 3) & 0xFFFF;
        vc[i] = (i * 5) & 0xFF;
        
        /* Read back and accumulate */
        sum += vs[i];
        sum += vc[i];
    }
    
    return sum;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning (SUBREG generation) */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t c[4];
    float f;
};

/* Function 7: SUBREG patterns through unions */
int test_subreg_union(union type_pun *u, int count) {
    int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Initialize with different patterns */
        u[i].i = (i * 0x01010101) ^ 0x12345678;
        
        /* Access sub-parts - should generate SUBREG */
        uint16_t low_half = u[i].s[0];
        uint16_t high_half = u[i].s[1];
        
        uint8_t byte0 = u[i].c[0];
        uint8_t byte1 = u[i].c[1];
        uint8_t byte2 = u[i].c[2];
        uint8_t byte3 = u[i].c[3];
        
        /* Cast between types of different sizes */
        int as_int = (int)u[i].i;
        short as_short = (short)u[i].s[0];
        char as_char = (char)u[i].c[0];
        
        /* Combine results */
        result += low_half - high_half;
        result += byte0 * byte1 + byte2 * byte3;
        result += as_int & 0xFFFF;
        result += as_short;
        result += as_char;
        
        /* Type punning through pointer casting */
        float *float_ptr = (float*)&u[i].i;
        float f_val = *float_ptr;
        u[i].f = f_val + 1.0f;
    }
    
    return result;
}

/* Function 8: Explicit casts for SUBREG generation */
int test_subreg_casts(int *int_arr, short *short_arr, char *char_arr, int size) {
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Cast down to smaller types - should generate SUBREG */
        short s_val = (short)(int_arr[i] & 0xFFFF);
        char c_val = (char)(int_arr[i] & 0xFF);
        
        /* Cast up and back down */
        int temp_int = (int)s_val;
        s_val = (short)(temp_int * 2);
        
        /* Store to arrays */
        short_arr[i] = s_val;
        char_arr[i] = c_val;
        
        /* Complex expression with mixed sizes */
        sum += (int)s_val + (int)c_val + (int_arr[i] >> 16);
        
        /* Pointer casting for memory access */
        *(short*)((char*)int_arr + i * 2) = s_val;
    }
    
    return sum;
}

/* Function 9: Packed structure for SUBREG access */
struct __attribute__((packed)) packed_struct {
    char a;
    short b;
    char c;
    int d;
};

int test_subreg_packed(struct packed_struct *ps, int count) {
    int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access packed fields - may require SUBREG for misaligned access */
        ps[i].a = i & 0xFF;
        ps[i].b = (i * 2) & 0xFFFF;
        ps[i].c = (i * 3) & 0xFF;
        ps[i].d = i * 100;
        
        /* Read and combine */
        result += ps[i].a + ps[i].b + ps[i].c + ps[i].d;
        
        /* Pointer arithmetic with different types */
        char *ptr = (char*)&ps[i];
        for (int j = 0; j < sizeof(struct packed_struct); j++) {
            result += ptr[j];
        }
    }
    
    return result;
}

/* ========== COMPLEX MEMORY REFERENCES ========== */

/* Function 10: Complex addressing modes */
int test_complex_memory(int *base_arr, int *index_arr, int size) {
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex address calculation */
        int *ptr = base_arr + index_arr[i % 16];
        
        /* Dereference with offset */
        int val = ptr[g_volatile_seed & 0x3];
        
        /* Bit-field extract from memory value */
        int field = (val >> (i % 16)) & 0xF;
        
        /* Update with bit-field insert */
        val = (val & ~(0xF << 8)) | (field << 8);
        
        /* Store back through complex address */
        ptr[g_volatile_seed & 0x3] = val;
        
        sum += val;
    }
    
    return sum;
}

/* Function 11: Combined patterns */
int test_combined_patterns(struct bitfield_struct *bfs, 
                          union type_pun *up, 
                          short *short_arr,
                          int *int_arr,
                          int count) {
    int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* ZERO_EXTRACT pattern */
        unsigned int bf_val = (bfs[i].field1 << 10) | bfs[i].field3;
        
        /* STRICT_LOW_PART pattern */
        short_arr[i] = (short)(bf_val & 0xFFFF);
        
        /* SUBREG pattern */
        up[i].s[0] = short_arr[i];
        up[i].s[1] = short_arr[i] ^ 0xFFFF;
        
        /* Complex memory reference */
        int_arr[i * 2] = up[i].i;
        int_arr[i * 2 + 1] = bf_val;
        
        /* Combine results */
        result += bfs[i].field2;
        result += short_arr[i];
        result += up[i].c[0] + up[i].c[1];
        result += int_arr[i * 2] & 0xFF;
    }
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    int total_result = 0;
    
    /* Initialize test data */
    const int ARRAY_SIZE = 256;
    const int STRUCT_COUNT = 64;
    
    /* Allocate and initialize arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short *short_array = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    int *index_array = (int*)malloc(16 * sizeof(int));
    
    struct bitfield_struct *bf_structs = 
        (struct bitfield_struct*)malloc(STRUCT_COUNT * sizeof(struct bitfield_struct));
    
    union type_pun *unions = 
        (union type_pun*)malloc(STRUCT_COUNT * sizeof(union type_pun));
    
    struct packed_struct *packed_structs = 
        (struct packed_struct*)malloc(STRUCT_COUNT * sizeof(struct packed_struct));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        short_array[i] = (short)(int_array[i] & 0xFFFF);
        char_array[i] = (char)(int_array[i] & 0xFF);
    }
    
    for (int i = 0; i < 16; i++) {
        index_array[i] = (i * 7) % (ARRAY_SIZE / 4);
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        bf_structs[i].field1 = i & 0x1F;
        bf_structs[i].field2 = (i * 3) & 0x7F;
        bf_structs[i].field3 = (i * 5) & 0x3FF;
        bf_structs[i].field4 = (i * 7) & 0x3FF;
        
        unions[i].i = (i * 0x01010101) ^ 0xABCDEF01;
        
        packed_structs[i].a = i & 0xFF;
        packed_structs[i].b = (i * 2) & 0xFFFF;
        packed_structs[i].c = (i * 3) & 0xFF;
        packed_structs[i].d = i * 100;
    }
    
    /* Run all tests and accumulate results */
    
    /* ZERO_EXTRACT tests */
    total_result ^= test_zero_extract_int(g_volatile_seed, 3, 5);
    total_result += test_zero_extract_struct(&bf_structs[0], g_volatile_seed);
    total_result += test_zero_extract_array(int_array, ARRAY_SIZE / 4);
    
    /* STRICT_LOW_PART tests */
    total_result += test_strict_low_part(short_array, char_array, ARRAY_SIZE / 2);
    total_result ^= test_strict_low_part_asm(g_volatile_seed);
    
    volatile short *vol_short = short_array;
    volatile char *vol_char = char_array;
    total_result += test_strict_low_part_volatile(vol_short, vol_char, ARRAY_SIZE / 4);
    
    /* SUBREG tests */
    total_result += test_subreg_union(unions, STRUCT_COUNT / 2);
    total_result += test_subreg_casts(int_array, short_array, char_array, ARRAY_SIZE / 4);
    total_result += test_subreg_packed(packed_structs, STRUCT_COUNT / 2);
    
    /* Complex memory tests */
    total_result += test_complex_memory(int_array, index_array, ARRAY_SIZE / 8);
    
    /* Combined patterns test */
    total_result += test_combined_patterns(bf_structs, unions, short_array, 
                                          int_array, STRUCT_COUNT / 4);
    
    /* Clean up */
    free(int_array);
    free(short_array);
    free(char_array);
    free(index_array);
    free(bf_structs);
    free(unions);
    free(packed_structs);
    
    /* Print result to prevent optimization */
    printf("Total result: %d\n", total_result);
    
    return total_result & 0xFF;
}
