/* Program to generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM RTL patterns */
#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* Global variables for memory operations */
unsigned int g_bitfield_target = 0x12345678;
int g_array[256];
long long g_large_value = 0x1122334455667788ULL;

/* ===== ZERO_EXTRACT patterns ===== */

/* Method 1: Direct bitfield extraction using shifts */
int extract_bits_shift(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for the bit range */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

/* Method 2: Struct with bitfields */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

int extract_bitfield(struct bitfield_struct *s) {
    /* Taking address and accessing bitfield may generate ZERO_EXTRACT */
    unsigned int val = s->mid8;  /* Access specific bit range */
    return val;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Method 1: Writing only low part of a variable */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    /* Write only the low byte, preserving high bytes */
    *p = (*p & ~0xFF) | v;  /* STRICT_LOW_PART pattern */
}

/* Method 2: Cast to smaller type */
void write_low_half(int32_t *x) {
    /* Direct write to low 16 bits */
    *(int16_t*)x = 0xABCD;  /* Should generate STRICT_LOW_PART */
}

/* ===== SUBREG patterns ===== */

/* Method 1: Union for type aliasing */
union type_aliasing {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int subreg_via_union(union type_aliasing *u) {
    /* Access part of larger register */
    u->halves[0] = 0x1234;  /* SUBREG access to low half */
    return u->bytes[1];     /* SUBREG access to byte within word */
}

/* Method 2: Pointer casting between different sizes */
int64_t subreg_via_cast(int64_t *ll) {
    /* Access 32-bit part of 64-bit value */
    int32_t partial = *(int32_t*)ll;  /* SUBREG pattern */
    return partial * 2;
}

/* ===== Complex MEM patterns ===== */

/* Method 1: Array with complex indexing */
int complex_mem_access(int *base, int idx1, int idx2, int idx3) {
    /* Complex address calculation: base + (idx1 + idx2*4 + idx3*16) */
    return base[idx1 + idx2 * 4 + idx3 * 16];  /* Complex MEM addressing */
}

/* Method 2: Struct with array and pointer arithmetic */
struct nested_array {
    int data[10][10];
    int padding;
};

int nested_mem_access(struct nested_array *s, int i, int j) {
    /* Multi-dimensional array access */
    return s->data[i][j];  /* Complex MEM with structure offset */
}

/* Method 3: Pointer arithmetic with multiple components */
int pointer_arithmetic(int *base, int offset1, int offset2) {
    /* Complex address: base + (offset1 << 2) + (offset2 & 0xF) */
    int *addr = base + (offset1 << 2) + (offset2 & 0xF);
    return *addr;
}

/* ===== Combined function with control flow ===== */

int combined_operations(int iterations) {
    int result = 0;
    union type_aliasing u;
    struct bitfield_struct bf = {0};
    struct nested_array nested;
    
    /* Initialize data */
    u.full = 0;
    bf.low8 = 0xAA;
    bf.mid8 = 0xBB;
    bf.high16 = 0xCCDD;
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile to control flow unpredictably */
        if (g_volatile_flag & (1 << (i & 3))) {
            /* ZERO_EXTRACT pattern */
            result ^= extract_bits_shift(&g_bitfield_target);
            result ^= extract_bitfield(&bf);
        } else {
            /* STRICT_LOW_PART pattern */
            set_low_byte((volatile unsigned int*)&g_bitfield_target, i & 0xFF);
            write_low_half((int32_t*)&result);
        }
        
        /* SUBREG pattern (always executed) */
        result += subreg_via_union(&u);
        result += subreg_via_cast(&g_large_value);
        
        /* Complex MEM patterns */
        if (i & 1) {
            result += complex_mem_access(g_array, i & 15, (i >> 4) & 3, (i >> 6) & 1);
        } else {
            result += nested_mem_access(&nested, i % 10, (i + 1) % 10);
            result += pointer_arithmetic(g_array, i & 31, (i >> 5) & 15);
        }
        
        /* Update volatile to affect next iteration */
        g_volatile_counter++;
    }
    
    return result;
}

/* ===== Helper functions for additional coverage ===== */

void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
}

int mixed_size_operations(void) {
    /* Mix operations of different sizes to generate SUBREG */
    char c = 0x12;
    short s = 0x1234;
    int i = 0x12345678;
    long long ll = 0x123456789ABCDEF0ULL;
    
    /* Various casts and accesses */
    i = (int)s + (int)c;          /* Promote smaller to larger */
    s = (short)(i & 0xFFFF);      /* Truncate to smaller */
    ll = ll + (long long)i;       /* Mix sizes in operation */
    
    /* Access parts of larger types */
    int32_t part32 = *(int32_t*)&ll;      /* SUBREG: get low 32 bits of 64-bit */
    int16_t part16 = *(int16_t*)&i;       /* SUBREG: get low 16 bits of 32-bit */
    
    return part32 + part16;
}

/* ===== Main function ===== */

int main(void) {
    int final_result = 0;
    
    /* Initialize data */
    init_arrays();
    
    /* Execute combined operations multiple times */
    for (int pass = 0; pass < 3; pass++) {
        g_volatile_flag = (pass + 1) * 0x1234;
        
        /* Call combined function */
        final_result += combined_operations(10 + pass * 5);
        
        /* Additional mixed operations */
        final_result ^= mixed_size_operations();
        
        /* Direct bitfield manipulation */
        struct bitfield_struct local_bf;
        local_bf.low8 = final_result & 0xFF;
        local_bf.mid8 = (final_result >> 8) & 0xFF;
        local_bf.high16 = (final_result >> 16) & 0xFFFF;
        
        /* Force ZERO_EXTRACT pattern */
        final_result += extract_bitfield(&local_bf);
        
        /* Force STRICT_LOW_PART pattern */
        set_low_byte((volatile unsigned int*)&final_result, 
                    (final_result + pass) & 0xFF);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (volatile counter: %u)\n", 
           final_result, (unsigned int)g_volatile_counter);
    
    return final_result & 0x7F;  /* Return non-zero value */
}
