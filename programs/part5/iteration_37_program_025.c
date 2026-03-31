/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdio.h>

/* Bitfield struct for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART operations */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex memory structure */
struct level3 {
    volatile int data[3][2];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2;
    volatile int idx;
};

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    struct bitfield_struct bf = {0};
    union split_int split = {0};
    v4si vec = {1, 2, 3, 4};
    
    /* Complex memory structure setup */
    struct level3 l3_obj = {{{0}}};
    struct level2 l2_obj = {&l3_obj, 1};
    struct level1 l1_obj = {&l2_obj, 0};
    
    /* Array for complex addressing */
    volatile int multi_array[4][8][2];
    
    /* Register variables for SUBREG */
    register uint32_t reg_var asm("eax") = 0x12345678;
    register uint16_t reg_short asm("ax");
    
    /* Loop with conditional branches */
    for (int i = 0; i < 100; i++) {
        global_counter++;
        
        /* 1. BITFIELD OPERATIONS (ZERO_EXTRACT) */
        if (i & 1) {
            bf.field1 = (i & 0x7);           /* 3-bit field */
            bf.field3 = (i * 3) & 0xFF;      /* 8-bit field */
        } else {
            bf.field2 = (i & 0x1F);          /* 5-bit field */
            bf.field4 = (i * 7) & 0xFFFF;    /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART operations */
        switch (i % 4) {
            case 0:
                split.parts.low = i * 2;     /* Update low 16 bits */
                break;
            case 1:
                split.bytes[1] = i + 5;      /* Update single byte */
                break;
            case 2:
                ((volatile uint16_t*)&split.full)[1] = i * 3; /* High word */
                break;
            case 3:
                split.parts.high = i / 2;    /* Update high 16 bits */
                break;
        }
        
        /* 3. SUBREG operations with register variables */
        reg_short = (reg_var >> (i % 16)) & 0xFFFF;
        
        /* Inline assembly influencing RTL generation */
        asm volatile (
            "movw %w[input], %%cx\n\t"
            "addw %%cx, %w[output]\n\t"
            : [output] "+r" (reg_short)
            : [input] "r" ((uint16_t)i)
            : "cx", "cc"
        );
        
        /* Vector operations that may generate SUBREG */
        int element = vec[i % 4];
        vec[i % 4] = element + reg_short;
        
        /* 4. COMPLEX MEMORY ADDRESSING */
        volatile int *complex_ptr;
        
        if (i & 2) {
            /* Multi-level pointer dereference */
            complex_ptr = &l1_obj.l2->l3->data[l1_obj.idx][l2_obj.offset];
            *complex_ptr += i;
            
            /* Multi-dimensional array with non-constant indices */
            multi_array[i % 4][(i * 3) % 8][i & 1] = 
                multi_array[(i + 1) % 4][(i * 5) % 8][(i + 1) & 1] + reg_short;
        } else {
            /* Different complex addressing pattern */
            complex_ptr = &multi_array[0][0][0] + 
                         (i % 4) * 8 * 2 + 
                         ((i * 7) % 8) * 2 + 
                         (i & 1);
            *complex_ptr = split.full;
        }
        
        /* 5. COMBINE OPERATIONS in conditional context */
        if (reg_short > 1000) {
            /* More bitfield operations */
            bf.field1 = reg_short & 0x7;
            bf.field4 = (reg_short * 2) & 0xFFFF;
            
            /* Additional memory chain */
            l1_obj.idx = (l1_obj.idx + 1) & 1;
            l2_obj.offset = (l2_obj.offset + 1) & 1;
        }
        
        /* Prevent loop elimination */
        if (i == 99) {
            /* Final combination of all values */
            global_result = bf.field1 + bf.field2 + bf.field3 + bf.field4 +
                           split.full + reg_var + reg_short +
                           *complex_ptr + vec[0] + vec[1] + vec[2] + vec[3];
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (counter: %d)\n", global_result, global_counter);
    
    return global_result > 0 ? 0 : 1;
}
