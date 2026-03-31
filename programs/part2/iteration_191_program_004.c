/* Test program to generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ===== ZERO_EXTRACT patterns ===== */

/* Bit-field extraction using shift/mask - may generate ZERO_EXTRACT */
unsigned int extract_bits_ze(volatile unsigned int *p) {
    /* Multiple extractions to increase chances */
    unsigned int val = *p;
    unsigned int result = 0;
    
    /* Extract bits 8-15 */
    result |= (val >> 8) & 0xFF;
    
    /* Extract bits 16-23 */
    result |= (val >> 16) & 0xFF;
    
    /* Extract bits 0-7 */
    result |= val & 0xFF;
    
    return result;
}

/* Bit-field struct for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int f1 : 4;
    unsigned int f2 : 8;
    unsigned int f3 : 12;
    unsigned int f4 : 8;
};

unsigned int read_bitfields(struct bitfield_struct *s) {
    /* Taking address and accessing bit-fields */
    unsigned int result = s->f1;
    result += s->f2;
    result += s->f3;
    result += s->f4;
    return result;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Writing only low part of a variable */
void write_low_part(volatile unsigned int *p, unsigned char v) {
    /* Clear low byte, then set it */
    *p = (*p & ~0xFF) | v;
}

/* Using smaller type assignment */
void strict_low_via_cast(volatile uint32_t *p) {
    /* Write to low 16 bits */
    *(uint16_t *)p = 0x1234;
    
    /* Write to low 8 bits */
    *(uint8_t *)p = 0xAB;
}

/* ===== SUBREG patterns ===== */

/* Union for SUBREG patterns */
union mixed_types {
    int32_t i32;
    int16_t s16[2];
    int8_t s8[4];
    float f32;
};

int subreg_via_union(union mixed_types *u) {
    /* Access parts of larger type */
    u->s16[0] = 100;
    u->s16[1] = 200;
    
    /* Mixed type access */
    int result = u->s8[0] + u->s8[1] + u->s8[2] + u->s8[3];
    
    /* Type punning */
    u->f32 = 3.14f;
    result += u->s16[0];
    
    return result;
}

/* Pointer casting for SUBREG */
int subreg_via_cast(volatile uint64_t *pll) {
    uint64_t ll = *pll;
    
    /* Access different parts */
    uint32_t low = *(uint32_t *)&ll;
    uint32_t high = *((uint32_t *)&ll + 1);
    
    return low + high;
}

/* ===== Complex MEM patterns ===== */

/* Struct with array for complex addressing */
struct complex_mem {
    int arr[100];
    int pad;
    int arr2[50];
};

int complex_mem_access(struct complex_mem *cm, int i, int j) {
    /* Complex addressing with multiple indices */
    int result = cm->arr[i * 3 + j * 7];
    
    /* More complex: base + scaled index + offset */
    result += cm->arr2[(i << 2) + j + 5];
    
    /* Pointer arithmetic */
    int *ptr = &cm->arr[10];
    result += ptr[i * 2 - j];
    
    return result;
}

/* Array with non-trivial indexing */
int array_complex_index(int *base, int idx1, int idx2, int idx3) {
    /* Multiple index calculations */
    int index = (idx1 * idx2) + (idx3 << 3);
    
    /* Conditional addressing */
    if (g_volatile_flag) {
        index = idx1 + idx2 * 4 + idx3 * 8;
    }
    
    return base[index & 0x3F]; /* Bound check */
}

/* ===== Combined function with control flow ===== */

int combined_operations(volatile int mode) {
    static union mixed_types u;
    static struct bitfield_struct bfs = {1, 2, 3, 4};
    static struct complex_mem cm;
    static unsigned int mem_buffer[64];
    
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        cm.arr[i] = i;
        if (i < 50) cm.arr2[i] = i * 2;
    }
    for (int i = 0; i < 64; i++) {
        mem_buffer[i] = i * 3;
    }
    
    /* Loop with volatile condition */
    for (volatile int i = 0; i < (mode & 3); i++) {
        g_volatile_counter++;
        
        /* ZERO_EXTRACT patterns */
        if (g_volatile_flag & 1) {
            result += extract_bits_ze(&mem_buffer[i]);
            result += read_bitfields(&bfs);
        }
        
        /* STRICT_LOW_PART patterns */
        if (g_volatile_flag & 2) {
            write_low_part(&mem_buffer[i + 1], i & 0xFF);
            strict_low_via_cast((uint32_t *)&mem_buffer[i + 2]);
        }
        
        /* SUBREG patterns */
        if (g_volatile_flag & 4) {
            result += subreg_via_union(&u);
            result += subreg_via_cast((uint64_t *)&mem_buffer[i]);
        }
        
        /* Complex MEM patterns */
        if (g_volatile_flag & 8) {
            result += complex_mem_access(&cm, i, i + 1);
            result += array_complex_index((int *)mem_buffer, i, i + 1, i + 2);
        }
    }
    
    return result;
}

/* Helper functions that emphasize specific patterns */
void emphasize_zero_extract(void) {
    volatile unsigned int buffer[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    
    for (int i = 0; i < 4; i++) {
        /* Multiple bit-field extractions */
        unsigned int val = buffer[i];
        unsigned int extracted = 
            ((val >> 24) & 0xFF) |    /* High byte */
            ((val >> 8) & 0xFF00) |   /* Middle high byte */
            ((val << 8) & 0xFF0000) | /* Middle low byte */
            ((val << 24) & 0xFF000000); /* Low byte */
        
        buffer[i] = extracted;
    }
}

void emphasize_strict_low(void) {
    volatile uint32_t values[4] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x11111111};
    
    for (int i = 0; i < 4; i++) {
        /* Write to low parts only */
        *(uint16_t *)&values[i] = 0x1234;
        *(uint8_t *)&values[i] = 0xAB;
        
        /* Conditional low-part write */
        if (g_volatile_counter & 1) {
            values[i] = (values[i] & ~0xFFFF) | (i & 0xFFFF);
        }
    }
}

void emphasize_subreg(void) {
    union {
        uint64_t u64;
        uint32_t u32[2];
        uint16_t u16[4];
        uint8_t u8[8];
    } data;
    
    data.u64 = 0x0123456789ABCDEFULL;
    
    /* Access all different sub-parts */
    uint32_t sum = 0;
    sum += data.u32[0];
    sum += data.u32[1];
    
    for (int i = 0; i < 4; i++) {
        sum += data.u16[i];
    }
    
    for (int i = 0; i < 8; i++) {
        sum += data.u8[i];
    }
    
    /* Mixed-size operations */
    data.u16[0] = data.u8[1] + data.u8[2];
    data.u32[1] = data.u16[2] * data.u16[3];
}

void emphasize_complex_mem(void) {
    static int matrix[16][16];
    volatile int indices[4] = {1, 3, 5, 7};
    
    /* Initialize matrix */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Complex memory addressing patterns */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Multi-dimensional with computed indices */
            int idx_i = (indices[i] * 3 + indices[j] * 7) & 0xF;
            int idx_j = (indices[j] * 5 + indices[i] * 11) & 0xF;
            
            total += matrix[idx_i][idx_j];
            
            /* Pointer arithmetic version */
            int *row = matrix[idx_i];
            total += *(row + idx_j);
            
            /* Even more complex: matrix[matrix[i][j]][matrix[j][i]] */
            if (g_volatile_flag) {
                int idx1 = matrix[i][j] & 0xF;
                int idx2 = matrix[j][i] & 0xF;
                total += matrix[idx1][idx2];
            }
        }
    }
}

/* Main function that exercises all patterns */
int main(void) {
    int final_result = 0;
    
    printf("Starting resource pattern generation test...\n");
    
    /* Call combined function multiple times with different modes */
    for (int mode = 0; mode < 8; mode++) {
        g_volatile_flag = mode + 1;
        final_result += combined_operations(mode);
    }
    
    /* Emphasize each pattern type */
    emphasize_zero_extract();
    emphasize_strict_low();
    emphasize_subreg();
    emphasize_complex_mem();
    
    /* Final mixed computation */
    final_result += g_volatile_counter;
    
    printf("Final result: %d\n", final_result);
    
    /* Return result to prevent optimization */
    return final_result & 0xFF;
}
