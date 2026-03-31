/* This program is designed to trigger specific RTL patterns in GCC's resource
   tracking pass during optimization. It creates operations that generate
   ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM expressions in RTL. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
long long global_ll = 0x123456789ABCDEF0LL;

/* Structs for bit-field and memory access patterns */
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
};

struct ComplexMem {
    int data[64];
    int pad[16];
    struct BitFieldStruct bf;
};

/* Union for SUBREG pattern generation */
union MixedTypes {
    int32_t full;
    int16_t halves[2];
    uint8_t bytes[4];
};

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Function 1: Direct bit-field extraction using shift and mask */
int extract_bits_shift(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for the bit range */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

/* Function 2: Bit-field extraction via struct member */
unsigned int extract_bitfield_member(struct BitFieldStruct *bfs) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    unsigned int val = bfs->mid16;  /* 16-bit bit-field access */
    return val * 2;
}

/* Function 3: Complex bit extraction with conditionals */
int conditional_extract(volatile unsigned int *p, int condition) {
    int result = 0;
    if (condition) {
        /* Extract multiple bit ranges */
        result = (*p & 0xF0) >> 4;      /* Bits 4-7 */
        result += (*p >> 16) & 0xFF;    /* Bits 16-23 */
    } else {
        result = (*p >> 24) & 0xF;      /* Bits 24-27 */
    }
    return result;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Function 4: Write to low byte of a larger integer */
void set_low_byte_volatile(volatile unsigned int *p, unsigned char v) {
    /* This pattern may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;  /* Only modify low 8 bits */
}

/* Function 5: Cast and assignment to partial register */
void set_low_half_via_cast(int32_t *x, int16_t v) {
    /* Writing to half of a 32-bit integer */
    *(int16_t*)x = v;  /* May generate STRICT_LOW_PART */
}

/* Function 6: Inline assembly for low-part write (x86 specific) */
#ifdef __x86_64__
void set_low_part_asm(volatile uint32_t *p, uint16_t v) {
    /* Assembly that writes only to AX (low 16 bits of EAX) */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movl %%eax, %0"
        : "=m" (*p)
        : "r" (v)
        : "eax"
    );
}
#endif

/* ==================== SUBREG Patterns ==================== */

/* Function 7: Union-based type punning */
int32_t union_subreg_access(union MixedTypes *u, int index) {
    /* Access different views of the same storage */
    u->halves[0] = 0x1234;      /* Write 16-bit to low half */
    u->bytes[2] = 0xAB;         /* Write 8-bit to third byte */
    return u->full;             /* Read back 32-bit */
}

/* Function 8: Pointer casting between different sizes */
int64_t mixed_size_access(volatile long long *ll, int offset) {
    /* Access parts of 64-bit value as 32-bit and 16-bit */
    int32_t *p32 = (int32_t*)ll;
    int16_t *p16 = (int16_t*)ll;
    
    p32[0] = 0xCAFEBABE;        /* Write low 32 bits */
    p16[2] = 0xDEAD;            /* Write bits 32-47 */
    return *ll + offset;
}

/* ==================== Complex MEM Patterns ==================== */

/* Function 9: Complex addressing with multiple indices */
int complex_mem_addressing(struct ComplexMem *cm, int i, int j, int k) {
    /* Non-trivial addressing: base + array + struct member */
    return cm->data[i + j*8] + cm->pad[k] + cm->bf.low8;
}

/* Function 10: Pointer arithmetic in loop */
int pointer_arithmetic_sum(int *base, int n, int stride) {
    int sum = 0;
    int *end = base + n * stride;
    
    /* Complex addressing: base + variable offset */
    for (int *p = base; p < end; p += stride) {
        sum += *p;
        /* Add another level of indirection */
        sum += *(p + (stride % 4));
    }
    return sum;
}

/* Function 11: Multi-dimensional array access */
int multi_dim_access(int arr[][8][4], int i, int j, int k) {
    /* Addressing: arr + i*8*4 + j*4 + k */
    return arr[i][j][k] + arr[i][k][j];
}

/* ==================== Main Orchestration ==================== */

/* Main function that combines all patterns with control flow */
int main(void) {
    int result = 0;
    union MixedTypes u;
    struct ComplexMem cm = {0};
    struct BitFieldStruct bfs = {0xAA, 0xBBBB, 0xCC};
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 64; i++) {
        cm.data[i] = i * 2;
    }
    
    /* Loop with volatile condition to create complex control flow */
    for (v_counter = 0; v_counter < 10; v_counter++) {
        /* Use volatile flags to create unpredictable branches */
        if (v_flag1 || (v_counter % 3 == 0)) {
            /* ZERO_EXTRACT patterns */
            result ^= extract_bits_shift(&global_bitfield);
            result += extract_bitfield_member(&bfs);
            result |= conditional_extract(&global_bitfield, v_counter & 1);
            
            /* Modify global for next iteration */
            global_bitfield = (global_bitfield * 1103515245 + 12345) & 0xFFFFFFFF;
        }
        
        if (v_flag2 || (v_counter % 5 == 2)) {
            /* STRICT_LOW_PART patterns */
            set_low_byte_volatile(&global_bitfield, v_counter & 0xFF);
            set_low_half_via_cast((int32_t*)&global_array[v_counter % 16], 
                                 v_counter * 7);
            
            #ifdef __x86_64__
            set_low_part_asm((uint32_t*)&global_array[v_counter % 8 + 8], 
                           v_counter * 3);
            #endif
        }
        
        /* SUBREG patterns (always executed) */
        result += union_subreg_access(&u, v_counter);
        result ^= mixed_size_access(&global_ll, v_counter) & 0xFFFF;
        
        /* Complex MEM patterns */
        if (v_counter % 2 == 0) {
            result += complex_mem_addressing(&cm, 
                                           v_counter % 8, 
                                           (v_counter / 8) % 8,
                                           v_counter % 4);
            result += pointer_arithmetic_sum(global_array, 
                                           v_counter % 16 + 1, 
                                           v_counter % 4 + 1);
        }
        
        /* Multi-dimensional array access */
        static int md_array[4][8][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                for (int k = 0; k < 4; k++) {
                    md_array[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
        result += multi_dim_access(md_array, 
                                 v_counter % 4, 
                                 (v_counter / 4) % 8, 
                                 v_counter % 4);
    }
    
    /* Final computation to use all results */
    result = (result * 31) & 0x7FFFFFFF;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result;
}
