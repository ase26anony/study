/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent excessive optimization */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function 1: Bit-field extraction from structure */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* These generate ZERO_EXTRACT for bit-field access */
        sum += s->field1;
        sum += s->field2 << 3;
        sum += (s->field3 & 0x3F) | (s->field4 << 6);
        
        /* Force recomputation to prevent loop elimination */
        s->field1 = (s->field1 + g_volatile_seed) & 0x1F;
        s->field2 = (s->field2 * 3) & 0x7F;
    }
    return sum;
}

/* Function 2: Manual bit-field extraction using shifts and masks */
unsigned int test_zero_extract_manual(uint32_t *arr, int size) {
    unsigned int sum = 0;
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT RTL */
        uint32_t val = arr[i];
        
        /* Extract various bit ranges */
        unsigned int low5 = (val >> 0) & 0x1F;      /* bits 0-4 */
        unsigned int mid8 = (val >> 5) & 0xFF;      /* bits 5-12 */
        unsigned int high10 = (val >> 13) & 0x3FF;  /* bits 13-22 */
        
        /* Complex expression combining extracts */
        sum += (low5 * mid8) | (high10 << 8);
        
        /* Update array with extracted values to prevent dead code */
        arr[i] = (low5 << 24) | (mid8 << 16) | (high10 << 6);
    }
    return sum;
}

/* Function 3: Bit-field comparison */
int test_zero_extract_compare(struct BitFieldStruct *s, int iterations) {
    int count = 0;
    for (int i = 0; i < iterations; i++) {
        /* Comparisons generate ZERO_EXTRACT */
        if (s->field1 == 0x0F) count++;
        if (s->field2 > 0x20) count--;
        if ((s->field3 & 0x1F) == 0x10) count += 2;
        
        /* Modify structure */
        s->field1 = (s->field1 + i) & 0x1F;
        s->field3 = (s->field3 ^ g_volatile_seed) & 0x3FF;
    }
    return count;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 4: Partial register updates with small types */
unsigned int test_strict_low_part(int iterations) {
    unsigned int sum = 0;
    volatile short v_short;  /* volatile to force memory ops */
    volatile char v_char;
    
    /* Register will hold int, but we write only low parts */
    for (int i = 0; i < iterations; i++) {
        int temp = i * 3 + g_volatile_seed;
        
        /* These should generate STRICT_LOW_PART for partial writes */
        v_short = (short)(temp & 0xFFFF);
        v_char = (char)(temp & 0xFF);
        
        /* Read back and use */
        sum += (unsigned int)v_short;
        sum += (unsigned int)v_char << 16;
    }
    return sum;
}

/* Function 5: Pointer to small volatile type */
unsigned int test_strict_low_part_volatile_ptr(uint32_t *base, int size) {
    unsigned int sum = 0;
    
    /* Cast to volatile short pointer - stores generate STRICT_LOW_PART */
    volatile short *sptr = (volatile short *)base;
    
    for (int i = 0; i < size * 2; i++) {
        /* Partial write to memory */
        sptr[i] = (short)((i * 7 + g_volatile_seed) & 0xFFFF);
        
        /* Read back through different type */
        sum += ((uint32_t *)sptr)[i/2];
    }
    return sum;
}

/* Function 6: Inline assembly for byte register access (x86-specific) */
#ifdef __x86_64__
unsigned int test_strict_low_part_asm(int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        unsigned char byte_val;
        unsigned short word_val;
        
        /* Inline assembly that operates on partial registers */
        __asm__ volatile (
            "movb %1, %0\n\t"
            : "=q"(byte_val)  /* q = a, b, c, or d register (byte-addressable) */
            : "r"((unsigned char)(i & 0xFF))
        );
        
        __asm__ volatile (
            "movw %1, %0\n\t"
            : "=r"(word_val)
            : "r"((unsigned short)(i & 0xFFFF))
        );
        
        sum += byte_val;
        sum += word_val;
    }
    return sum;
}
#endif

/* ========== SUBREG patterns ========== */
/* Function 7: Union for type-punning */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union TypePun *u, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Initialize full register */
        u->full = i * 0x1234567 + g_volatile_seed;
        
        /* Access sub-parts - should generate SUBREG */
        sum += u->halves[0];      /* Low 16 bits */
        sum += u->halves[1] << 8; /* High 16 bits */
        sum += u->bytes[2] << 16; /* Third byte */
        
        /* Modify through sub-part */
        u->halves[1] = (sum & 0xFFFF) ^ 0xABCD;
    }
    return sum;
}

/* Function 8: Casting between integer sizes */
unsigned int test_subreg_casts(uint32_t *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        uint32_t val = arr[i];
        
        /* Casts to smaller types generate SUBREG */
        uint16_t low16 = (uint16_t)(val & 0xFFFF);
        uint16_t high16 = (uint16_t)(val >> 16);
        uint8_t low8 = (uint8_t)(val & 0xFF);
        
        /* Operations on subregs */
        sum += (uint32_t)low16 * (uint32_t)high16;
        sum += (uint32_t)low8 << 24;
        
        /* Write back through cast */
        arr[i] = ((uint32_t)high16 << 16) | (uint32_t)low16;
    }
    return sum;
}

/* Function 9: Packed structure with mixed sizes */
struct __attribute__((packed)) PackedStruct {
    uint16_t a;
    uint8_t b;
    uint32_t c;
    uint16_t d;
};

unsigned int test_subreg_packed(struct PackedStruct *ps, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Accesses to packed members may use SUBREG */
        sum += ps->a;
        sum += ps->b << 8;
        sum += ps->c;
        sum += ps->d << 16;
        
        /* Modify members */
        ps->a = (ps->a + i) & 0xFFFF;
        ps->b = (ps->b ^ g_volatile_seed) & 0xFF;
        ps->c = ps->c * 3;
    }
    return sum;
}

/* ========== Complex memory references ========== */
/* Function 10: Combining patterns with complex addressing */
unsigned int test_complex_memory(uint32_t *base, int size, int stride) {
    unsigned int sum = 0;
    struct BitFieldStruct bfs = {0};
    
    for (int i = 0; i < size; i += stride) {
        /* Memory reference with index */
        uint32_t *ptr = &base[i];
        
        /* ZERO_EXTRACT from memory */
        unsigned int field = (*ptr >> 8) & 0xFFF;
        
        /* Cast to smaller type (SUBREG) */
        uint16_t *short_ptr = (uint16_t *)ptr;
        
        /* Partial write (STRICT_LOW_PART) */
        short_ptr[1] = (field & 0xFFFF);
        
        /* Update bit-field structure */
        bfs.field2 = (bfs.field2 + field) & 0x7F;
        
        /* Use everything in sum */
        sum += *ptr + bfs.field2;
    }
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    int iterations = 1000;
    int array_size = 100;
    unsigned int final_sum = 0;
    
    /* Initialize test data */
    uint32_t *data_array = (uint32_t *)malloc(array_size * sizeof(uint32_t));
    struct BitFieldStruct bfs = {1, 2, 3, 4};
    union TypePun pun;
    struct PackedStruct ps = {0x1234, 0x56, 0x789ABCDE, 0xFEDC};
    
    if (!data_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill array with non-constant data */
    for (int i = 0; i < array_size; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0xFFFFFFFF;
    }
    
    /* Run all tests */
    final_sum += test_zero_extract_struct(&bfs, iterations);
    final_sum += test_zero_extract_manual(data_array, array_size / 2);
    final_sum += test_zero_extract_compare(&bfs, iterations / 2);
    
    final_sum += test_strict_low_part(iterations);
    final_sum += test_strict_low_part_volatile_ptr(data_array, array_size / 4);
    
    #ifdef __x86_64__
    final_sum += test_strict_low_part_asm(iterations / 4);
    #endif
    
    final_sum += test_subreg_union(&pun, iterations);
    final_sum += test_subreg_casts(data_array, array_size);
    final_sum += test_subreg_packed(&ps, iterations / 2);
    
    final_sum += test_complex_memory(data_array, array_size, 3);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_sum);
    
    free(data_array);
    
    /* Return non-zero for success (makes it usable in test harness) */
    return (final_sum != 0) ? 0 : 1;
}
