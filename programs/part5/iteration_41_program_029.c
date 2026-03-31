/* test_resource_patterns.c
 * 
 * This program generates specific RTL patterns to test uncovered code paths
 * in GCC's resource.cc, specifically targeting:
 * - ZERO_EXTRACT expressions (bit-field operations)
 * - STRICT_LOW_PART expressions (partial register updates)
 * - SUBREG expressions (register sub-parts)
 * - Complex memory addressing modes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Bit-field extraction from integers */
unsigned int test_zero_extract_int(unsigned int x, unsigned int y) {
    unsigned int result = 0;
    
    /* Various bit-field extraction patterns */
    result += (x >> 3) & 0x1F;           /* Extract 5 bits */
    result += (y & 0xFF00) >> 8;         /* Extract middle 8 bits */
    result += ((x + y) >> 2) & 0x7;      /* Extract 3 bits */
    result += (x >> 10) & 0xFFFF;        /* Extract 16 bits */
    
    /* Chain extractions to prevent optimization */
    result = (result >> 4) & 0xF;
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments (generate ZERO_EXTRACT in SET_DEST) */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0xFF;
    s->field3 = (val >> 13) & 0x7;
    s->field4 = (val >> 16) & 0xFFFF;
    
    /* Bit-field comparisons and arithmetic */
    if (s->field1 == 0x10) result += 1;
    if (s->field2 > 0x80) result += 2;
    result += s->field3 * 3;
    result += s->field4 >> 4;
    
    /* Complex expression with multiple extracts */
    result += ((s->field1 << 3) | s->field2) & 0xFF;
    
    return result;
}

/* Function 3: Array processing with bit-field operations */
unsigned int test_zero_extract_array(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Different extract patterns based on array index */
        unsigned int val = arr[i] + g_volatile_seed;
        
        if (i % 4 == 0) {
            sum += (val >> 0) & 0x1F;      /* 5 bits */
        } else if (i % 4 == 1) {
            sum += (val >> 5) & 0xFF;      /* 8 bits */
        } else if (i % 4 == 2) {
            sum += (val >> 13) & 0x7;      /* 3 bits */
        } else {
            sum += (val >> 16) & 0xFFFF;   /* 16 bits */
        }
        
        /* Update array with extracted value (creates SET_DEST with ZERO_EXTRACT) */
        arr[i] = (arr[i] & ~0x1F) | (sum & 0x1F);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 4: Partial register updates with char/short */
unsigned int test_strict_low_part_charshort(unsigned int init_val) {
    unsigned int result = init_val;
    
    /* char operations that may generate STRICT_LOW_PART */
    volatile char *char_ptr = (volatile char *)&result;
    char_ptr[0] = (init_val + 1) & 0xFF;      /* Update low byte */
    char_ptr[1] = (init_val + 2) & 0xFF;      /* Update second byte */
    
    /* short operations */
    volatile short *short_ptr = (volatile short *)&result;
    short_ptr[1] = (init_val >> 16) + 3;      /* Update high short (may use STRICT_LOW_PART) */
    
    /* Mix of operations to prevent optimization */
    for (int i = 0; i < 4; i++) {
        char_ptr[i] += i + g_volatile_seed;
    }
    
    return result;
}

/* Function 5: Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned short low_part;
    unsigned char byte_part;
    
    /* Inline assembly that operates on partial registers */
    __asm__ volatile (
        "movw %1, %0\n\t"          /* Copy low 16 bits */
        : "=r"(low_part)
        : "r"(x)
        : /* No clobbers */
    );
    
    /* Another partial update */
    __asm__ volatile (
        "movb %b1, %b0\n\t"        /* Copy low 8 bits */
        : "+r"(x)
        : "ri"(g_volatile_seed)
        : /* No clobbers */
    );
    
    byte_part = (x >> 8) & 0xFF;
    
    return low_part + byte_part + x;
}

/* Function 6: Pointer casting for partial accesses */
unsigned int test_strict_low_part_pointers(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Cast to volatile short pointer for partial write */
        volatile short *sptr = (volatile short *)&arr[i];
        sptr[0] = (arr[i] + i) & 0xFFFF;          /* Low 16 bits */
        sptr[1] = (arr[i] >> 16) + i;             /* High 16 bits */
        
        /* Cast to volatile char pointer */
        volatile char *cptr = (volatile char *)&arr[i];
        for (int j = 0; j < 4; j++) {
            cptr[j] += (i + j) & 0xFF;
        }
        
        sum += arr[i];
    }
    
    return sum;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning (generates SUBREG) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function 7: Union-based SUBREG patterns */
unsigned int test_subreg_union(union type_pun *u, unsigned int val) {
    unsigned int result = 0;
    
    u->full = val;
    
    /* Access different views of the same register (generates SUBREG) */
    result += u->halves[0];          /* Low 16 bits */
    result += u->halves[1] << 16;    /* High 16 bits */
    
    /* Byte access */
    for (int i = 0; i < 4; i++) {
        result += u->bytes[i] << (i * 4);
    }
    
    /* Structure member access */
    u->parts.low = (val + 1) & 0xFFFF;
    u->parts.high = (val >> 16) + 2;
    
    result += u->full;
    
    return result;
}

/* Function 8: Casting between types of different sizes */
unsigned int test_subreg_casting(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Cast to smaller types and back (generates SUBREG) */
        uint16_t short_val = (uint16_t)(arr[i] & 0xFFFF);
        uint8_t byte_val = (uint8_t)(arr[i] >> 24);
        
        /* Operations on sub-parts */
        short_val += i & 0xFF;
        byte_val += (i >> 8) & 0xFF;
        
        /* Recombine with original */
        arr[i] = (arr[i] & 0x00FFFF00) | 
                 ((unsigned int)short_val) | 
                 ((unsigned int)byte_val << 24);
        
        sum += arr[i];
    }
    
    return sum;
}

/* Function 9: SIMD-like operations using unions */
unsigned int test_subreg_simd_like(unsigned int a, unsigned int b) {
    union {
        unsigned int words[2];
        unsigned long long dword;
    } u;
    
    u.words[0] = a;
    u.words[1] = b;
    
    /* Access as different types (generates SUBREG) */
    unsigned long long temp = u.dword;
    temp += (temp >> 32) | (temp << 32);  /* Swap halves */
    
    u.dword = temp;
    
    /* Access individual words again */
    return u.words[0] + u.words[1];
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Function 10: Combine patterns with complex addressing */
unsigned int test_complex_addressing(struct bitfield_struct *structs, 
                                     union type_pun *unions,
                                     unsigned int *arr,
                                     int size) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing: array + struct + union */
        unsigned int idx = (i + g_volatile_seed) % size;
        
        /* Memory access with index calculation */
        structs[idx].field1 = arr[i] & 0x1F;
        structs[idx].field2 = (arr[i] >> 5) & 0xFF;
        
        /* Union access with pointer arithmetic */
        unions[idx].halves[0] = structs[idx].field1;
        unions[idx].halves[1] = structs[idx].field2;
        
        /* Update array with complex expression */
        arr[i] = (arr[i] & ~0xFF) | 
                 ((unions[idx].full >> 8) & 0xFF) |
                 ((structs[idx].field3 & 0x7) << 24);
        
        result += arr[i] + structs[idx].field4 + unions[idx].parts.low;
    }
    
    return result;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with volatile to prevent constant folding */
    int data_size = 100;
    unsigned int *test_array = (unsigned int*)malloc(data_size * sizeof(unsigned int));
    struct bitfield_struct *test_structs = (struct bitfield_struct*)malloc(data_size * sizeof(struct bitfield_struct));
    union type_pun *test_unions = (union type_pun*)malloc(data_size * sizeof(union type_pun));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < data_size; i++) {
        test_array[i] = (i * 1103515245 + 12345) & 0xFFFFFFFF;
        test_unions[i].full = test_array[i];
        test_structs[i].field1 = i & 0x1F;
        test_structs[i].field2 = (i >> 5) & 0xFF;
        test_structs[i].field3 = (i >> 13) & 0x7;
        test_structs[i].field4 = (i >> 16) & 0xFFFF;
    }
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    final_result += test_zero_extract_int(test_array[0], test_array[1]);
    final_result += test_zero_extract_struct(&test_structs[0], test_array[2]);
    final_result += test_zero_extract_array(test_array, data_size / 10);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part_charshort(test_array[3]);
    final_result += test_strict_low_part_asm(test_array[4]);
    final_result += test_strict_low_part_pointers(test_array, data_size / 10);
    
    /* Test SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    final_result += test_subreg_union(&test_unions[0], test_array[5]);
    final_result += test_subreg_casting(test_array, data_size / 10);
    final_result += test_subreg_simd_like(test_array[6], test_array[7]);
    
    /* Test combined patterns with complex addressing */
    printf("Testing complex addressing patterns...\n");
    final_result += test_complex_addressing(test_structs, test_unions, test_array, data_size / 5);
    
    /* Use volatile to ensure operations aren't optimized away */
    g_volatile_seed = final_result;
    
    /* Clean up */
    free(test_array);
    free(test_structs);
    free(test_unions);
    
    printf("Final result: %u\n", final_result);
    
    /* Return non-zero to indicate success */
    return (final_result != 0) ? 0 : 1;
}
