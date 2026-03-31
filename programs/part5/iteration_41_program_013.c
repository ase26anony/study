/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile = 0;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT for bit-field operations */
    unsigned int mask = (1u << 5) - 1;  /* 5-bit mask */
    unsigned int result = 0;
    
    /* Multiple extractions to ensure they're not optimized away */
    result += (x >> shift) & mask;          /* Simple shift+mask */
    result += (x & 0xFF00) >> 8;            /* Explicit mask and shift */
    result += (x & 0xF0F0) >> 4;            /* Another pattern */
    
    /* Use volatile to prevent constant folding */
    result += g_volatile & 0x1F;
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 3;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x07;
    s->field3 = (val >> 8) & 0xFF;
    
    /* Bit-field comparisons */
    if (s->field1 == 0x10) result += 1;
    if (s->field2 > 0x03) result += 2;
    if (s->field3 != 0) result += s->field3;
    
    /* Complex bit-field expression */
    result += (s->field1 << 3) | s->field2;
    
    return result;
}

/* ========== STRICT_LOW_PART Patterns ========== */

short test_strict_low_part_short(short *arr, int size) {
    short result = 0;
    int i;
    
    /* Partial register updates through pointer */
    for (i = 0; i < size; i++) {
        /* Writing to short should generate STRICT_LOW_PART */
        short temp = arr[i] + i;
        arr[i] = temp;  /* Store only low 16 bits */
        result += temp;
        
        /* Volatile pointer write */
        volatile short *vptr = (volatile short *)&arr[i];
        *vptr = temp * 2;
        result += *vptr;
    }
    
    return result;
}

char test_strict_low_part_char(char *arr, int size) {
    char result = 0;
    int i;
    
    /* char operations that may use partial registers */
    for (i = 0; i < size; i++) {
        /* Promote to int, modify, then store back as char */
        int temp = arr[i];
        temp = (temp * 3 + i) & 0xFF;
        arr[i] = (char)temp;  /* Store only low 8 bits */
        result += arr[i];
    }
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    {
        char var = result;
        int val = var * 2;
        /* Byte register operation */
        asm volatile (
            "movb %1, %0\n\t"
            : "=q"(var)
            : "r"((char)val)
            : "cc"
        );
        result = var;
    }
    
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t b[4];
};

uint32_t test_subreg_union(union type_pun *u, uint32_t val) {
    uint32_t result = 0;
    
    u->i = val;
    
    /* Access sub-parts through union - should generate SUBREG */
    result += u->s[0];          /* Low 16 bits */
    result += u->s[1] << 16;    /* High 16 bits */
    result += u->b[2] << 8;     /* Third byte */
    
    /* Cast between different sizes */
    uint16_t low_half = (uint16_t)(u->i & 0xFFFF);
    uint16_t high_half = (uint16_t)((u->i >> 16) & 0xFFFF);
    result += low_half + high_half;
    
    return result;
}

/* SIMD-like operations using manual packing */
uint64_t test_subreg_packed(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    /* Pack four 16-bit values into 64-bit */
    uint64_t packed = ((uint64_t)(a & 0xFFFF) << 0) |
                      ((uint64_t)(b & 0xFFFF) << 16) |
                      ((uint64_t)(c & 0xFFFF) << 32) |
                      ((uint64_t)(d & 0xFFFF) << 48);
    
    /* Extract individual elements - may use SUBREG */
    uint16_t elem1 = (packed >> 0) & 0xFFFF;
    uint16_t elem2 = (packed >> 16) & 0xFFFF;
    uint16_t elem3 = (packed >> 32) & 0xFFFF;
    uint16_t elem4 = (packed >> 48) & 0xFFFF;
    
    return elem1 + elem2 + elem3 + elem4;
}

/* ========== Complex Memory References ========== */

/* Structure with mixed types for complex addressing */
struct mixed_data {
    int data[16];
    short shorts[32];
    char bytes[64];
    struct bitfield_struct bitfields[4];
};

int test_complex_memory(struct mixed_data *md, int index) {
    int result = 0;
    
    /* Complex addressing modes */
    result += md->data[index * 2];                     /* Array with index */
    result += md->shorts[index + 1] * 2;               /* Different type */
    result += md->bytes[index * 4] & 0x0F;             /* Byte access */
    
    /* Pointer arithmetic */
    int *ptr = &md->data[index];
    result += *(ptr + 1) - *ptr;                       /* Pointer difference */
    
    /* Structure member with bit-field */
    md->bitfields[index % 4].field1 = index & 0x1F;
    result += md->bitfields[index % 4].field1;
    
    return result;
}

/* ========== Main Test Function ========== */

int main(int argc, char **argv) {
    int i;
    unsigned int checksum = 0;
    
    /* Initialize test data */
    struct bitfield_struct bfs = {0};
    union type_pun upun;
    struct mixed_data md;
    
    /* Initialize arrays */
    short short_arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    char char_arr[16];
    for (i = 0; i < 16; i++) {
        char_arr[i] = (char)(i * 3);
    }
    
    /* Initialize mixed data */
    for (i = 0; i < 16; i++) {
        md.data[i] = i * 10;
    }
    for (i = 0; i < 32; i++) {
        md.shorts[i] = (short)(i * 5);
    }
    for (i = 0; i < 64; i++) {
        md.bytes[i] = (char)i;
    }
    
    /* Use command line argument to prevent constant folding */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Test ZERO_EXTRACT patterns */
    checksum += test_zero_extract_int(base, 3);
    checksum += test_zero_extract_struct(&bfs, base + 1);
    
    /* Test STRICT_LOW_PART patterns */
    checksum += test_strict_low_part_short(short_arr, 8);
    checksum += test_strict_low_part_char(char_arr, 16);
    
    /* Test SUBREG patterns */
    checksum += test_subreg_union(&upun, base + 2);
    checksum += test_subreg_packed(base, base + 1, base + 2, base + 3);
    
    /* Test complex memory references */
    for (i = 0; i < 4; i++) {
        checksum += test_complex_memory(&md, i);
    }
    
    /* Add volatile to final result */
    checksum += g_volatile;
    
    printf("Checksum: %u\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
