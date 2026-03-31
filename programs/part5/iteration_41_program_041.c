/* test_resource_coverage.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x) {
    unsigned int sum = 0;
    
    /* Multiple bit-field extractions with varying widths */
    sum += (x >> 0) & 0x1F;        /* Extract bits 0-4 */
    sum += (x >> 5) & 0x3F;        /* Extract bits 5-10 (6 bits) */
    sum += (x >> 11) & 0x7FF;      /* Extract bits 11-21 (11 bits) */
    sum += (x >> 22) & 0x3FF;      /* Extract bits 22-31 (10 bits) */
    
    /* Combined mask and shift (common ZERO_EXTRACT pattern) */
    sum += (x & 0xFF00) >> 8;      /* Extract middle byte */
    sum += (x & 0xF0F0F0F0) >> 4;  /* Complex bit pattern */
    
    return sum;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field reads generate ZERO_EXTRACT */
        sum += s->a;
        sum += s->b;
        sum += s->c;
        sum += s->d;
        
        /* Bit-field comparisons */
        if (s->a == 3) sum += 1;
        if (s->b > 10) sum += 2;
        if (s->c != 0) sum += s->c;
        
        /* Modify to prevent optimization */
        s->a = (s->a + 1) & 0x7;
        s->b = (s->b * 2) & 0x1F;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with small types */
unsigned int test_strict_low_part(int base) {
    unsigned int sum = 0;
    volatile short vs;  /* Force memory access */
    
    /* char/short operations that may use partial registers */
    char c = (char)(base & 0xFF);
    short s = (short)(base & 0xFFFF);
    
    for (int i = 0; i < 100; i++) {
        /* Operations that update only part of a register */
        c = (c + i) & 0xFF;          /* Only low 8 bits matter */
        s = (s - i * 2) & 0xFFFF;    /* Only low 16 bits matter */
        
        /* Use volatile to force actual stores */
        vs = s;
        
        /* Pointer to volatile short - may generate STRICT_LOW_PART store */
        volatile short *ps = &vs;
        *ps = (short)(*ps + c);
        
        sum += c + s;
    }
    
    return sum;
}

/* Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(int x) {
    unsigned short result;
    
    /* Assembly that operates on byte/word registers */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r" (result)
        : "r" ((unsigned short)x)
        : "%ax"
    );
    
    return result;
}

/* Array of small types for memory operations */
unsigned int test_strict_low_part_mem(char *data, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i += 2) {
        /* Cast to short pointer - store may use STRICT_LOW_PART */
        short *sp = (short *)(data + i);
        *sp = (short)(*sp + i + g_volatile_seed);
        
        /* Also test char operations */
        data[i] = (char)(data[i] ^ 0x55);
        
        sum += *sp + data[i];
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t i;
    uint16_t s[2];
    uint8_t b[4];
};

unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same register */
        sum += u->s[0];      /* Low 16 bits */
        sum += u->s[1];      /* High 16 bits */
        sum += u->b[2];      /* Third byte */
        
        /* Modify through different views */
        u->s[0] = (u->s[0] + 1) & 0xFFFF;
        u->b[3] ^= 0xFF;
        
        /* Complex expression with casts */
        uint16_t temp = (uint16_t)(u->i >> 8);
        sum += temp;
    }
    
    return sum;
}

/* SIMD-like operations using integer registers */
unsigned int test_subreg_simd(uint32_t x) {
    unsigned int sum = 0;
    
    /* Extract and manipulate byte-sized parts */
    uint8_t b0 = (x >> 0) & 0xFF;
    uint8_t b1 = (x >> 8) & 0xFF;
    uint8_t b2 = (x >> 16) & 0xFF;
    uint8_t b3 = (x >> 24) & 0xFF;
    
    /* Recombine with shifts - generates SUBREG operations */
    sum = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
    
    /* More complex byte manipulation */
    b0 = (b0 + b1) & 0xFF;
    b2 = (b2 ^ b3) & 0xFF;
    
    return sum + (b0 << 8) + b2;
}

/* ========== Combined patterns with memory references ========== */

/* Complex addressing modes with bit-field operations */
unsigned int test_combined(struct bitfield_struct *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Array access with index (complex address) */
        struct bitfield_struct *current = &arr[i];
        
        /* Bit-field extract from memory location */
        sum += current->c;
        
        /* Modify through pointer with offset */
        current->b = (current->b + i) & 0x1F;
        
        /* Cast to different type for SUBREG access */
        uint16_t *short_ptr = (uint16_t *)current;
        *short_ptr = (*short_ptr + sum) & 0xFFFF;
    }
    
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    unsigned int total_sum = 0;
    
    /* Initialize test data with some randomness */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Test ZERO_EXTRACT patterns */
    unsigned int test_val = g_volatile_seed * 12345;
    total_sum += test_zero_extract_int(test_val);
    
    struct bitfield_struct bf = {1, 5, 42, 1000};
    total_sum += test_zero_extract_struct(&bf, iterations % 50);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part(g_volatile_seed);
    total_sum += test_strict_low_part_asm(g_volatile_seed);
    
    char data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = (char)(i ^ g_volatile_seed);
    }
    total_sum += test_strict_low_part_mem(data, 256);
    
    /* Test SUBREG patterns */
    union type_pun u;
    u.i = 0xDEADBEEF;
    total_sum += test_subreg_union(&u, iterations % 20);
    
    total_sum += test_subreg_simd(0x12345678);
    
    /* Test combined patterns */
    struct bitfield_struct arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i].a = i & 0x7;
        arr[i].b = (i * 3) & 0x1F;
        arr[i].c = (i * 7) & 0xFF;
        arr[i].d = i * 100;
    }
    total_sum += test_combined(arr, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Total checksum: %u\n", total_sum);
    
    return (total_sum > 0) ? 0 : 1;
}
