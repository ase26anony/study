/* test_resource_patterns.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_patterns.c -o test
 * For RTL debugging: gcc -O1 -da -fdump-rtl-all test_resource_patterns.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int extract_bitfield(unsigned int x, int shift, unsigned int mask) {
    /* This should generate ZERO_EXTRACT RTL */
    return (x >> shift) & mask;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

unsigned int test_bitfield_ops(struct bitfield_struct *s, unsigned int val) {
    unsigned int sum = 0;
    
    /* Bit-field assignment - may generate ZERO_EXTRACT */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    
    /* Bit-field comparison - may generate ZERO_EXTRACT */
    if (s->field3 == (val & 0x3FF)) {
        sum += s->field1;
    }
    
    /* Complex bit-field arithmetic */
    unsigned int extracted = (s->field4 << 3) | (s->field2 >> 2);
    sum += extracted;
    
    return sum;
}

/* Explicit mask and shift operations */
unsigned int test_zero_extract(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various bit extraction patterns */
        unsigned int val = arr[i] ^ g_seed; /* Prevent optimization */
        
        /* Pattern 1: Extract byte */
        sum += (val & 0xFF00) >> 8;
        
        /* Pattern 2: Extract multiple non-contiguous bits */
        unsigned int bits = ((val >> 3) & 0x7) | ((val >> 10) & 0x3) << 3;
        sum += bits;
        
        /* Pattern 3: Rotate and extract */
        unsigned int rotated = (val << 5) | (val >> 27);
        sum += rotated & 0x1F;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part(short *sarr, char *carr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Promote to int, modify, write back low part */
        short s = sarr[i];
        int temp = s + g_seed;  /* Promotion to int */
        sarr[i] = (short)(temp & 0xFFFF);  /* STRICT_LOW_PART store */
        
        /* char operations */
        char c = carr[i];
        int ctemp = c * 3;
        carr[i] = (char)(ctemp & 0xFF);  /* Another STRICT_LOW_PART */
        
        sum += sarr[i] + carr[i];
    }
    
    return sum;
}

/* Volatile pointer to force partial writes */
unsigned int test_volatile_partial(volatile short *vs, volatile char *vc, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These volatile writes may generate STRICT_LOW_PART */
        vs[i] = (short)(i * 37);
        vc[i] = (char)(i * 13);
        
        sum += vs[i] + vc[i];
    }
    
    return sum;
}

/* Inline assembly for byte register operations */
unsigned int test_asm_partial(void) {
    unsigned int result = 0;
    
    /* Force use of byte registers */
    unsigned char b1, b2;
    unsigned int dword;
    
    dword = g_seed;
    
    /* Extract and manipulate bytes */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(b1)  /* q constraint = a, b, c, or d register (byte) */
        : "r"(dword)
        : "cc"
    );
    
    asm volatile (
        "movb %%al, %0\n\t"
        : "=m"(b2)
        : 
        : "al", "cc"
    );
    
    result = b1 + b2;
    return result;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t dword;
    uint16_t words[2];
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data */
        u[i].dword = i * 0x01020304;
        
        /* SUBREG accesses */
        sum += u[i].words[0];  /* Low 16 bits */
        sum += u[i].words[1];  /* High 16 bits */
        sum += u[i].bytes[2];  /* Third byte */
    }
    
    return sum;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various casts that may generate SUBREG */
        int val = arr[i] ^ g_seed;
        
        /* Cast to smaller type */
        short s = (short)(val & 0xFFFF);
        sum += s;
        
        /* Cast to char */
        char c = (char)(val & 0xFF);
        sum += c;
        
        /* Promote back with sign extension */
        int promoted = (int)s * (int)c;
        sum += promoted & 0xFF;
    }
    
    return sum;
}

/* Packed structure */
struct __attribute__((packed)) packed_data {
    uint16_t a;
    uint8_t b;
    uint16_t c;
    uint8_t d;
};

unsigned int test_packed_struct(struct packed_data *pd, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access packed fields - may require SUBREG accesses */
        pd[i].a = (i * 3) & 0xFFFF;
        pd[i].b = (i * 5) & 0xFF;
        pd[i].c = (i * 7) & 0xFFFF;
        pd[i].d = (i * 11) & 0xFF;
        
        sum += pd[i].a + pd[i].b + pd[i].c + pd[i].d;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */

/* Memory references with addressing modes */
unsigned int test_complex_memref(int *base, int *index_arr, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex addressing: base + scaled index */
        int idx = index_arr[i] & 0xF;  /* Prevent large indices */
        
        /* Memory load with complex address */
        int val = base[idx * 2] ^ g_seed;
        
        /* Bit extraction from memory value */
        unsigned int extracted = (val >> (idx * 2)) & 0x3;
        
        /* Store back with partial update */
        short *short_ptr = (short *)&base[idx];
        *short_ptr = (short)(extracted * 17);  /* STRICT_LOW_PART store */
        
        sum += extracted + *short_ptr;
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(void) {
    const int SIZE = 256;
    unsigned int final_sum = 0;
    
    /* Initialize test data */
    unsigned int *arr = malloc(SIZE * sizeof(unsigned int));
    short *sarr = malloc(SIZE * sizeof(short));
    char *carr = malloc(SIZE * sizeof(char));
    union type_pun *unions = malloc(SIZE * sizeof(union type_pun));
    struct packed_data *packed = malloc(SIZE * sizeof(struct packed_data));
    int *index_arr = malloc(SIZE * sizeof(int));
    struct bitfield_struct bf_struct = {0};
    
    volatile short *volatile_shorts = malloc(SIZE * sizeof(short));
    volatile char *volatile_chars = malloc(SIZE * sizeof(char));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 1103515245 + 12345;
        sarr[i] = (short)(i * 37);
        carr[i] = (char)(i * 13);
        index_arr[i] = i * 7;
        
        if (volatile_shorts) volatile_shorts[i] = 0;
        if (volatile_chars) volatile_chars[i] = 0;
    }
    
    /* Test ZERO_EXTRACT patterns */
    final_sum += test_zero_extract(arr, SIZE);
    
    bf_struct.field1 = 5;
    bf_struct.field2 = 42;
    bf_struct.field3 = 511;
    bf_struct.field4 = 1023;
    final_sum += test_bitfield_ops(&bf_struct, g_seed);
    
    /* Test STRICT_LOW_PART patterns */
    final_sum += test_strict_low_part(sarr, carr, SIZE);
    
    if (volatile_shorts && volatile_chars) {
        final_sum += test_volatile_partial(volatile_shorts, volatile_chars, SIZE);
    }
    
    final_sum += test_asm_partial();
    
    /* Test SUBREG patterns */
    if (unions) {
        final_sum += test_subreg_union(unions, SIZE);
    }
    
    final_sum += test_subreg_casts(arr, SIZE);
    
    if (packed) {
        final_sum += test_packed_struct(packed, SIZE);
    }
    
    /* Test complex memory references */
    final_sum += test_complex_memref((int *)arr, index_arr, SIZE);
    
    /* Additional mixed pattern to increase coverage */
    for (int i = 0; i < 100; i++) {
        /* Mixed operations in loop */
        unsigned int val = arr[i] ^ g_seed;
        
        /* ZERO_EXTRACT pattern */
        unsigned int bits = (val >> 4) & 0xF;
        
        /* Convert to short (potential SUBREG) */
        short s = (short)(bits * 17);
        
        /* Modify low part only (potential STRICT_LOW_PART) */
        s = (short)((s + i) & 0xFF);
        
        /* Store to memory with complex address */
        short *ptr = (short *)&arr[(i * 3) % SIZE];
        *ptr = s;
        
        final_sum += *ptr + bits;
    }
    
    /* Cleanup */
    free(arr);
    free(sarr);
    free(carr);
    free(unions);
    free(packed);
    free(index_arr);
    free((void *)volatile_shorts);
    free((void *)volatile_chars);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_sum);
    return (int)(final_sum & 0x7FFFFFFF);
}
