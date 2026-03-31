/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Struct with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* Complex nested structure for memory addressing */
struct level3 {
    volatile int data[4][4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2_array[8];
    volatile int index;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct bf = {0};
volatile union split_int split = {0};
volatile v4si vec = {0};
volatile struct level1 *complex_ptr = NULL;

/* Function with inline assembly for SUBREG and register variables */
void process_with_subreg(volatile int *result) {
    /* Register variable for SUBREG operations */
    register int reg_var asm("eax") = *result;
    register short reg_short asm("si") = 0;
    
    /* Operations that may generate SUBREG */
    reg_short = (reg_var & 0xFFFF);  /* Truncation to 16-bit */
    
    /* Inline assembly with constraints for subregisters */
    asm volatile (
        "movw %w[short], %[temp]\n\t"
        "addl %[temp], %[var]\n\t"
        : [var] "+r" (reg_var), [temp] "=&r" (reg_var)
        : [short] "r" (reg_short)
        : "cc"
    );
    
    /* Vector operation that may involve SUBREG */
    vec[0] = reg_var;
    vec[1] = reg_var >> 8;
    
    *result = reg_var + vec[0] + vec[1];
}

int main() {
    volatile int i, j, k;
    volatile int array[256][256];
    volatile int *ptr_array[128];
    volatile int result = 0;
    
    /* Initialize pointer array */
    for (i = 0; i < 128; i++) {
        ptr_array[i] = &array[i * 2][0];
    }
    
    /* Allocate complex structure */
    volatile struct level1 l1;
    volatile struct level2 l2[8];
    volatile struct level3 l3[8];
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                l3[i].data[j][k] = i * 100 + j * 10 + k;
            }
        }
        l2[i].l3 = &l3[i];
        l2[i].offset = i * 16;
        l1.l2_array[i] = &l2[i];
    }
    l1.index = 3;
    complex_ptr = &l1;
    
    /* Main loop combining all required patterns */
    for (i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT through bitfield assignments */
        bf.field1 = (i & 0x7);                    /* 3-bit field */
        bf.field2 = ((i * 3) & 0x1F);             /* 5-bit field */
        bf.field3 = ((i * 5) & 0xFF);             /* 8-bit field */
        bf.field4 = ((i * 7) & 0xFFFF);           /* 16-bit field */
        
        /* Combine bitfields - may generate multiple ZERO_EXTRACT */
        volatile unsigned int combined = 
            (bf.field1 << 29) |
            (bf.field2 << 24) |
            (bf.field3 << 16) |
            bf.field4;
        
        /* 2. STRICT_LOW_PART through union/pointer casting */
        split.parts.low = (i & 0xFFFF);           /* Update low 16 bits */
        
        /* Alternative STRICT_LOW_PART via pointer */
        volatile uint32_t *int_ptr = (volatile uint32_t*)&split;
        volatile uint16_t *short_ptr = (volatile uint16_t*)int_ptr;
        short_ptr[1] = (i >> 16) & 0xFFFF;        /* Update high 16 bits */
        
        /* 3. SUBREG operations via function call */
        process_with_subreg(&result);
        
        /* 4. Complex memory addressing (MEM) */
        /* Multi-level pointer dereferencing */
        volatile int mem_result = 
            complex_ptr->l2_array[i % 8]->l3->data[(i / 4) % 4][i % 4];
        
        /* Array indexing with non-constant expressions */
        volatile int idx1 = (i * 13) % 256;
        volatile int idx2 = (i * 17) % 256;
        array[idx1][idx2] = mem_result + combined;
        
        /* Chain of pointer accesses */
        volatile int *chain_ptr = ptr_array[i % 128];
        chain_ptr += (i * 7) % 256;
        *chain_ptr = split.full + result;
        
        /* 5. Conditional context */
        if (split.full % 3 == 0) {
            /* More complex memory addressing in branch */
            volatile struct level2 *l2_ptr = complex_ptr->l2_array[split.full % 8];
            l2_ptr->l3->data[0][0] = *chain_ptr;
            
            /* Additional bitfield operation in branch */
            bf.field1 = (bf.field2 + bf.field3) & 0x7;
        } else if (split.full % 5 == 0) {
            /* Switch statement for additional control flow */
            switch (result & 0x3) {
                case 0:
                    bf.field2 = (mem_result >> 3) & 0x1F;
                    break;
                case 1:
                    split.parts.low = (mem_result & 0xFFFF);
                    break;
                case 2:
                    /* Inline assembly with memory constraint */
                    asm volatile (
                        "movl %[val], (%[ptr])\n\t"
                        : 
                        : [ptr] "r" (chain_ptr), [val] "r" (result)
                        : "memory"
                    );
                    break;
            }
        }
        
        /* Loop exit condition based on operations */
        result += combined + split.full + mem_result + *chain_ptr;
        if (result > 1000000) {
            result %= 1000000;
        }
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    volatile int final_result = 
        bf.field1 + bf.field2 * 256 + bf.field3 * 65536 + 
        split.parts.low + split.parts.high * 256 +
        vec[0] + vec[1] + vec[2] + vec[3] +
        complex_ptr->l2_array[0]->l3->data[0][0];
    
    return (final_result & 0xFF) + (result & 0xFF);
}
