/* Cover resource.cc lines 282-290: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P patterns */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ========== ZERO_EXTRACT patterns ========== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_ze(volatile unsigned int *p) {
    /* Should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Struct with bit-fields */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
};

unsigned int read_bitfield_ze(struct bitfield_struct *s) {
    /* Accessing bit-fields often generates ZERO_EXTRACT */
    unsigned int val = s->mid8;  /* Extract middle 8 bits */
    return val;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Pattern 1: Write to low part of integer */
void set_low_part_slp(volatile unsigned int *p, unsigned char v) {
    /* Writing only low byte - may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast to smaller type */
void write_low_halfword_slp(volatile uint32_t *p, uint16_t v) {
    /* Cast and assignment to low part */
    *(uint16_t*)p = v;  /* Write to low 16 bits */
}

/* ========== SUBREG patterns ========== */

/* Pattern 1: Union for type punning */
union subreg_union {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

uint16_t access_via_union_subreg(union subreg_union *u) {
    /* Accessing smaller part of union generates SUBREG */
    u->halves[0] = 0x1234;  /* Write to low half */
    return u->halves[1];    /* Read high half */
}

/* Pattern 2: Pointer casting between different sizes */
int32_t cast_access_subreg(volatile int64_t *pll) {
    /* Cast between different-sized accesses */
    int32_t result = *(int32_t*)pll;  /* Read low 32 bits of 64-bit */
    return result;
}

/* ========== Complex MEM patterns ========== */

/* Pattern 1: Array with complex indexing */
int complex_mem_access(int *base, int idx1, int idx2, int idx3) {
    /* Complex address calculation: base + idx1 + idx2*4 + idx3*16 */
    return base[idx1 + idx2 * 4 + idx3 * 16];
}

/* Pattern 2: Struct with nested arrays */
struct nested_mem_struct {
    int matrix[10][10];
    int padding[20];
    int data[100];
};

int access_nested_mem(struct nested_mem_struct *s, int i, int j, int k) {
    /* Multiple different memory addressing patterns */
    int val1 = s->matrix[i][j];           /* 2D array access */
    int val2 = s->data[k * 3 + 5];        /* Scaled index access */
    return val1 + val2;
}

/* ========== Combined function with control flow ========== */

unsigned int combined_operations(volatile unsigned int trigger) {
    unsigned int result = 0;
    static unsigned int static_buffer[256];
    union subreg_union u;
    struct bitfield_struct bf = {0};
    struct nested_mem_struct nested;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        static_buffer[i] = i * 0x01010101;
    }
    
    u.full = 0xDEADBEEF;
    bf.mid8 = 0xAB;
    
    /* Complex control flow based on volatile input */
    if (trigger & 0x01) {
        /* ZERO_EXTRACT pattern */
        result ^= extract_bits_ze(&trigger);
        result ^= read_bitfield_ze(&bf);
    }
    
    if (trigger & 0x02) {
        /* STRICT_LOW_PART patterns */
        set_low_part_slp(&result, (trigger >> 16) & 0xFF);
        write_low_halfword_slp((uint32_t*)&result, trigger & 0xFFFF);
    }
    
    if (trigger & 0x04) {
        /* SUBREG patterns */
        result += access_via_union_subreg(&u);
        result += cast_access_subreg((int64_t*)&static_buffer[0]);
    }
    
    if (trigger & 0x08) {
        /* Complex MEM patterns */
        int idx = (trigger >> 8) & 0xFF;
        result += complex_mem_access(static_buffer, idx, idx/2, idx/4);
        result += access_nested_mem(&nested, idx%10, (idx/10)%10, idx%7);
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < 4; i++) {
        volatile int loop_flag = g_volatile_flag;
        
        if (loop_flag & (1 << i)) {
            /* Mix different patterns in loop */
            result ^= extract_bits_ze((unsigned int*)&static_buffer[i]);
            
            union subreg_union tmp;
            tmp.full = result;
            result = access_via_union_subreg(&tmp);
            
            /* Complex memory addressing in loop */
            int* ptr = &static_buffer[0];
            ptr += i * 16 + (result & 0xF);
            result += *ptr;
        }
    }
    
    return result;
}

/* ========== Main driver ========== */

int main() {
    unsigned int final_result = 0;
    
    /* Initialize volatile counter */
    g_volatile_counter = 0x12345678;
    
    /* Call combined operations multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter = (g_volatile_counter * 1103515245 + 12345) & 0x7FFFFFFF;
        g_volatile_flag = (g_volatile_counter >> 16) & 0x7;
        
        /* Each iteration provides different trigger patterns */
        unsigned int trigger = g_volatile_counter;
        final_result ^= combined_operations(trigger);
        
        /* Occasionally call individual pattern functions directly */
        if ((i % 13) == 0) {
            struct bitfield_struct bf = {0};
            bf.mid8 = i & 0xFF;
            final_result += read_bitfield_ze(&bf);
        }
        
        if ((i % 17) == 0) {
            uint64_t big_val = 0x1122334455667788ULL;
            final_result += cast_access_subreg((int64_t*)&big_val);
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: 0x%08X\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}
