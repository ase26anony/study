/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates code patterns
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global/volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;
volatile unsigned int g_bitfield_source = 0xDEADBEEF;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_shift_mask(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Bit-field struct with address taken */
struct bitfield_s {
    unsigned int f1 : 4;
    unsigned int f2 : 8;
    unsigned int f3 : 4;
    unsigned int f4 : 16;
};

unsigned int extract_from_bitfield(struct bitfield_s *s) {
    /* Taking address and accessing bitfield may generate ZERO_EXTRACT */
    unsigned int val1 = s->f2;  /* 8-bit field */
    unsigned int val2 = s->f4;  /* 16-bit field */
    return val1 + val2;
}

/* Pattern 3: Multiple extractions in a loop */
unsigned int extract_multiple_bits(volatile unsigned int *p, int count) {
    unsigned int result = 0;
    for (int i = 0; i < count; i++) {
        /* Different extraction patterns */
        result ^= (*p >> (i * 3)) & 0x7;  /* Extract 3 bits at varying positions */
    }
    return result;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Pattern 1: Writing to low byte of a larger integer */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    /* This may generate STRICT_LOW_PART when writing only low 8 bits */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast and assignment to smaller type */
void write_low_half(int32_t *x, int16_t v) {
    /* Writing 16-bit value to what might be a 32-bit register */
    *(int16_t*)x = v;
}

/* Pattern 3: Inline assembly that suggests low-part operation */
void low_part_asm(volatile uint32_t *p) {
    /* Using inline assembly to hint at low-part operations */
    uint32_t val;
    asm volatile ("movl %1, %0\n\t"
                  "andl $0xFFFF, %0"
                  : "=r"(val)
                  : "m"(*p)
                  : "cc");
    *p = val;
}

/* ==================== SUBREG patterns ==================== */

/* Pattern 1: Union for type punning */
union type_punning_u {
    int64_t ll;
    int32_t i[2];
    int16_t s[4];
    int8_t  c[8];
};

int32_t access_via_subreg_union(union type_punning_u *u, int index) {
    /* Accessing parts of larger type through smaller views */
    u->s[1] = 0x1234;      /* Write to 16-bit part */
    u->c[3] = 0xAB;        /* Write to 8-bit part */
    return u->i[index];    /* Read 32-bit part */
}

/* Pattern 2: Pointer casting between different sizes */
int32_t pointer_cast_subreg(volatile int64_t *src) {
    int32_t result;
    /* Cast 64-bit pointer to 32-bit pointer */
    int32_t *p32 = (int32_t*)src;
    result = p32[0] + p32[1];
    return result;
}

/* Pattern 3: Mixed-size operations in expressions */
int64_t mixed_size_ops(int32_t a, int16_t b, int8_t c) {
    /* Operations mixing different integer sizes */
    int64_t result = a;          /* 32-bit to 64-bit */
    result += b;                 /* 16-bit to 64-bit */
    result *= c;                 /* 8-bit to 64-bit */
    return result;
}

/* ==================== Complex MEM patterns ==================== */

/* Pattern 1: Array with complex indexing */
struct array_container {
    int data[256];
    int padding[16];
};

int complex_array_indexing(struct array_container *cont, 
                          int idx1, int idx2, int idx3) {
    /* Complex address calculation with multiple terms */
    return cont->data[idx1 * 4 + idx2 * 8 + idx3];
}

/* Pattern 2: Nested struct with pointer arithmetic */
struct inner_s {
    int x;
    int y;
    int z;
};

struct outer_s {
    struct inner_s items[32];
    int metadata[8];
};

int nested_struct_access(struct outer_s *outer, int i, int j) {
    /* Complex memory addressing through nested structures */
    return outer->items[i].x + outer->items[j].y;
}

/* Pattern 3: Multiple memory references in addressing mode */
int multi_index_access(int *base, int *offsets, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* base[offsets[i]] creates complex addressing */
        sum += base[offsets[i]];
    }
    return sum;
}

/* ==================== Combined patterns ==================== */

/* Function that combines multiple patterns */
unsigned int combined_patterns(volatile unsigned int trigger) {
    unsigned int result = 0;
    
    /* ZERO_EXTRACT pattern */
    result ^= extract_bits_shift_mask(&g_bitfield_source);
    
    /* STRICT_LOW_PART pattern */
    set_low_byte((volatile unsigned int*)&result, trigger & 0xFF);
    
    /* SUBREG pattern via union */
    union type_punning_u u;
    u.ll = 0x1122334455667788ULL;
    result += access_via_subreg_union(&u, trigger & 1);
    
    /* Complex MEM pattern */
    static struct array_container container;
    int idx = (trigger >> 8) & 0xFF;
    result += complex_array_indexing(&container, idx, idx/2, idx/4);
    
    return result;
}

/* Main driver with control flow */
int main(int argc, char **argv) {
    unsigned int final_result = 0;
    int i, iterations;
    
    /* Use command line or volatile to determine iterations */
    iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Initialize some data structures */
    struct bitfield_s bf = {0x1, 0x23, 0x4, 0x5678};
    union type_punning_u u;
    struct array_container container;
    struct outer_s outer;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 256; i++) {
        container.data[i] = i * 3;
    }
    
    for (i = 0; i < 32; i++) {
        outer.items[i].x = i * 2;
        outer.items[i].y = i * 3;
        outer.items[i].z = i * 5;
    }
    
    /* Main loop with various pattern usages */
    for (i = 0; i < iterations; i++) {
        g_volatile_counter++;
        
        /* Use volatile flag to create unpredictable control flow */
        if (g_volatile_flag & 1) {
            /* ZERO_EXTRACT patterns */
            final_result ^= extract_from_bitfield(&bf);
            final_result += extract_multiple_bits(&g_bitfield_source, 4);
        }
        
        if (g_volatile_flag & 2) {
            /* STRICT_LOW_PART patterns */
            set_low_byte(&g_bitfield_source, i & 0xFF);
            write_low_half((int32_t*)&final_result, i & 0xFFFF);
        }
        
        if (g_volatile_flag & 4) {
            /* SUBREG patterns */
            u.ll = final_result * 0x123456789ABCDEFULL;
            final_result += pointer_cast_subreg(&u.ll);
            final_result ^= mixed_size_ops(i, i*2, i*3) & 0xFFFFFFFF;
        }
        
        if (g_volatile_flag & 8) {
            /* Complex MEM patterns */
            final_result += nested_struct_access(&outer, i % 32, (i * 7) % 32);
            
            int offsets[] = {1, 3, 7, 15, 31};
            final_result += multi_index_access(container.data, offsets, 5);
        }
        
        /* Combined patterns function */
        final_result += combined_patterns(g_volatile_counter);
        
        /* Modify volatile flag pseudo-randomly */
        g_volatile_flag = (g_volatile_flag * 1103515245 + 12345) & 0xFF;
    }
    
    /* Ensure result is used */
    printf("Final result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
