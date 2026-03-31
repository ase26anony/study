/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory addressing modes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field extraction from integer */
int test_zero_extract_int(unsigned int x) {
    int sum = 0;
    /* Multiple bit-field extractions with varying widths */
    sum += (x >> 3) & 0x1F;      /* Extract 5 bits */
    sum += (x >> 8) & 0xFF;      /* Extract 8 bits */
    sum += (x >> 16) & 0x7;      /* Extract 3 bits */
    sum += (x >> 20) & 0x3FF;    /* Extract 10 bits */
    return sum;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int b : 8;
    unsigned int c : 3;
    unsigned int d : 10;
    unsigned int e : 6;
};

int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    int sum = 0;
    
    /* Bit-field assignments that may generate ZERO_EXTRACT */
    s->a = val & 0x1F;
    s->b = (val >> 5) & 0xFF;
    s->c = (val >> 13) & 0x7;
    s->d = (val >> 16) & 0x3FF;
    s->e = (val >> 26) & 0x3F;
    
    /* Bit-field comparisons and arithmetic */
    if (s->a == 0x10) sum += 1;
    if (s->b > 0x80) sum += 2;
    sum += s->c * 2;
    sum += s->d / 3;
    sum += s->e << 1;
    
    return sum;
}

/* Complex bit-field manipulation with memory */
int test_zero_extract_memory(unsigned int *array, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Extract different bit ranges from array elements */
        unsigned int val = array[i] ^ g_seed; /* Prevent optimization */
        sum += (val & 0xFF00) >> 8;          /* Middle byte */
        sum += (val & 0xF0F0F0F0) >> 4;      /* Scattered bits */
        sum += (val >> 24) & 0x7F;           /* Top 7 bits */
        
        /* Store extracted bits back to memory */
        array[i] = ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) |
                   ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
    }
    return sum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates with char/short */
int test_strict_low_part_chars(char *data, int size) {
    int sum = 0;
    volatile char vc; /* Force actual memory operations */
    
    for (int i = 0; i < size; i++) {
        /* char operations that may use partial registers */
        char c = data[i];
        vc = c;  /* Volatile write */
        c = vc;  /* Volatile read */
        
        /* Promote to int, modify, store back low part */
        int temp = c;
        temp = (temp * 3 + 7) & 0xFF;
        data[i] = temp;  /* STRICT_LOW_PART for byte store */
        sum += temp;
    }
    return sum;
}

/* Short operations with volatile */
int test_strict_low_part_shorts(short *array, int size) {
    int sum = 0;
    volatile short vs;
    
    for (int i = 0; i < size; i++) {
        /* Operations on short that may generate partial register updates */
        short s = array[i] ^ g_seed; /* Prevent optimization */
        vs = s;
        s = vs;
        
        /* Arithmetic that stays within 16 bits */
        s = (s * 13 + 17) & 0xFFFF;
        array[i] = s;  /* Potential STRICT_LOW_PART for 16-bit store */
        sum += s;
    }
    return sum;
}

/* Inline assembly for explicit partial register access */
int test_strict_low_part_asm(int x) {
    short result;
    
    /* Assembly that explicitly works with partial registers */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        : "r" ((short)x)
        : "%ax"
    );
    
    return result;
}

/* Pointer to volatile short */
int test_strict_low_part_volatile_ptr(volatile short *ptr, int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        *ptr = (short)(i * 7 + 3);  /* Direct store to volatile short */
        sum += *ptr;                 /* Force read after write */
        ptr = (volatile short *)((char *)ptr + 1); /* Odd alignment */
    }
    return sum;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t b[4];
};

int test_subreg_union(union type_pun *u, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data */
        u->i = (u->i * 1103515245 + 12345) ^ g_seed;
        
        /* SUBREG patterns from accessing sub-parts */
        sum += u->s[0];      /* Low 16 bits */
        sum += u->s[1];      /* High 16 bits */
        sum += u->b[0] * 3;  /* First byte */
        sum += u->b[2] << 1; /* Third byte */
        
        /* Cast between types */
        int16_t low_half = (int16_t)(u->i & 0xFFFF);
        int16_t high_half = (int16_t)((u->i >> 16) & 0xFFFF);
        sum += low_half + high_half;
    }
    return sum;
}

/* SIMD-like operations using subregs */
int test_subreg_simd_like(uint32_t *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        uint32_t val = data[i];
        
        /* Extract and manipulate individual bytes */
        uint8_t b0 = (val >> 0) & 0xFF;
        uint8_t b1 = (val >> 8) & 0xFF;
        uint8_t b2 = (val >> 16) & 0xFF;
        uint8_t b3 = (val >> 24) & 0xFF;
        
        /* Process bytes separately */
        b0 = (b0 * 3) & 0xFF;
        b1 = (b1 + 17) & 0xFF;
        b2 = (b2 ^ 0x55) & 0xFF;
        b3 = (b3 - 23) & 0xFF;
        
        /* Recombine - may involve SUBREG operations */
        data[i] = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
        sum += b0 + b1 + b2 + b3;
    }
    return sum;
}

/* Packed structure access */
struct packed_data {
    uint16_t a;
    uint8_t b;
    uint8_t c;
    uint16_t d;
} __attribute__((packed));

int test_subreg_packed(struct packed_data *pd, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Access packed fields - may require SUBREG operations */
        sum += pd[i].a;
        sum += pd[i].b * 2;
        sum += pd[i].c << 1;
        sum += pd[i].d / 2;
        
        /* Modify fields */
        pd[i].a = (pd[i].a + sum) & 0xFFFF;
        pd[i].b = (pd[i].b ^ 0xAA) & 0xFF;
        pd[i].c = (pd[i].c * 3) & 0xFF;
        pd[i].d = (pd[i].d - 1) & 0xFFFF;
    }
    return sum;
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Array with complex indexing */
int test_complex_addressing(int *base, int *offsets, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Complex address calculation */
        int *ptr = base + (offsets[i] & 0xF);
        
        /* Access with offset */
        int val = ptr[i % 4];
        
        /* Bit-field extract from memory value */
        int extracted = (val >> (i % 16)) & ((1 << 8) - 1);
        
        /* Store back with partial update */
        ptr[i % 4] = (ptr[i % 4] & ~(0xFF << 8)) | (extracted << 8);
        
        sum += extracted;
    }
    return sum;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    int total_sum = 0;
    
    /* Initialize test data */
    unsigned int array[64];
    struct bitfield_struct bf;
    char char_data[128];
    short short_array[64];
    union type_pun u;
    struct packed_data packed[16];
    int base_array[128];
    int offsets[32];
    
    /* Seed with volatile to prevent optimization */
    srand(g_seed);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 64; i++) {
        array[i] = rand();
        short_array[i] = rand() & 0xFFFF;
        if (i < 32) offsets[i] = rand() & 0xF;
        if (i < 16) {
            packed[i].a = rand() & 0xFFFF;
            packed[i].b = rand() & 0xFF;
            packed[i].c = rand() & 0xFF;
            packed[i].d = rand() & 0xFFFF;
        }
    }
    for (int i = 0; i < 128; i++) {
        char_data[i] = rand() & 0xFF;
        base_array[i] = rand();
    }
    
    /* Test ZERO_EXTRACT patterns */
    total_sum += test_zero_extract_int(array[0]);
    total_sum += test_zero_extract_struct(&bf, array[1]);
    total_sum += test_zero_extract_memory(array, 64);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part_chars(char_data, 128);
    total_sum += test_strict_low_part_shorts(short_array, 64);
    total_sum += test_strict_low_part_asm(total_sum);
    
    volatile short vs;
    total_sum += test_strict_low_part_volatile_ptr(&vs, 10);
    
    /* Test SUBREG patterns */
    u.i = rand();
    total_sum += test_subreg_union(&u, 100);
    total_sum += test_subreg_simd_like(array, 64);
    total_sum += test_subreg_packed(packed, 16);
    
    /* Test complex addressing */
    total_sum += test_complex_addressing(base_array, offsets, 32);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", total_sum);
    
    return total_sum & 0xFF; /* Return non-zero result */
}
