/* Target: resource.cc - mark_referenced_resources function
 * Specifically targets ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P expressions
 */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions to generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 8;
    volatile unsigned int f4 : 4;
    volatile unsigned int f5 : 12;
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

/* Complex structure for memory addressing chains */
struct level3 {
    volatile int data[3];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2_array[4];
    volatile int idx;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    volatile struct bitfields bf = {0};
    
    /* 2. Union for STRICT_LOW_PART */
    union split_int split = {0};
    
    /* 3. Register variables for SUBREG operations */
    register uint32_t reg_var asm("eax") = 0x12345678;
    register uint16_t reg_short asm("bx") = 0;
    
    /* 4. Complex memory structures */
    struct level3 l3_objs[2] = {{{1,2,3}, {4,5,6}}};
    struct level2 l2_objs[4];
    struct level1 l1_obj;
    
    /* Initialize memory structure chain */
    for (int i = 0; i < 4; i++) {
        l2_objs[i].l3 = &l3_objs[i % 2];
        l2_objs[i].offset = i * 10;
    }
    l1_obj.l2_array[0] = &l2_objs[0];
    l1_obj.l2_array[1] = &l2_objs[1];
    l1_obj.l2_array[2] = &l2_objs[2];
    l1_obj.l2_array[3] = &l2_objs[3];
    l1_obj.idx = 0;
    
    /* 5. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context to affect scheduling */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignments with non-byte-aligned fields */
            bf.f2 = (i & 0x1F);          /* 5-bit field */
            bf.f3 = (i * 2) & 0xFF;      /* 8-bit field */
            bf.f5 = (i * 3) & 0xFFF;     /* 12-bit field */
            
            /* Use the bitfields in computation */
            global_counter += bf.f2 + bf.f3;
        }
        
        if (i % 5 == 0) {
            /* STRICT_LOW_PART: Partial register updates */
            /* Update low 16 bits only */
            split.parts.low = i & 0xFFFF;
            
            /* Update specific byte */
            split.bytes[1] = (i >> 8) & 0xFF;
            
            /* Pointer-based partial update */
            *((volatile uint16_t*)&split.full + 1) = i & 0xAAAA;
        }
        
        if (i % 7 == 0) {
            /* SUBREG: Register variable operations with truncation */
            /* Operation that requires truncation to 16-bit */
            reg_short = reg_var & 0xFFFF;
            
            /* Vector element extraction (triggers SUBREG) */
            int elem = vec[i % 4];
            vec2[i % 4] = elem * 2;
            
            /* Mix with memory operation */
            reg_var = split.full + elem;
        }
        
        if (i % 11 == 0) {
            /* Complex MEM addressing: Multi-level pointer dereferencing */
            volatile int idx1 = i % 4;
            volatile int idx2 = (i >> 2) % 3;
            
            /* Chain: l1_obj -> l2_array[] -> l2 -> l3 -> data[] */
            int val = l1_obj.l2_array[idx1]->l3->data[idx2];
            
            /* Even more complex: with computation in index */
            l1_obj.l2_array[(i + 1) % 4]->l3->data[(val + i) % 3] = i * val;
            
            /* Array with non-constant index */
            volatile int* ptr_array[4];
            for (int j = 0; j < 4; j++) {
                ptr_array[j] = &l2_objs[j].offset;
            }
            *ptr_array[i % 4] = val + i;
        }
        
        /* 6. Inline assembly to influence RTL generation */
        if (i % 13 == 0) {
            /* Assembly that uses specific register constraints */
            asm volatile (
                "movl %1, %%eax\n\t"
                "movw %%ax, %0\n\t"
                : "=r" (reg_short)
                : "r" (reg_var)
                : "%eax"
            );
            
            /* Memory constraint with register */
            asm volatile (
                "addl %1, %0\n\t"
                : "+m" (split.full)
                : "r" (i)
                : "cc"
            );
        }
        
        /* Switch statement to create more complex control flow */
        switch (i % 8) {
            case 0:
                bf.f1 = i & 0x7;
                break;
            case 1:
                split.bytes[2] = (i >> 4) & 0xFF;
                break;
            case 2:
                reg_var = (reg_var << 3) | (i & 0x7);
                break;
            case 3:
                /* Complex memory chain in switch */
                l1_obj.l2_array[2]->offset = l1_obj.l2_array[1]->l3->data[0];
                break;
            case 4:
                /* Vector operation */
                vec = vec + vec2;
                break;
            default:
                /* Mixed operation */
                bf.f4 = (split.parts.low >> 3) & 0xF;
                break;
        }
        
        /* Prevent loop elimination */
        global_result += bf.f1 + split.parts.low + reg_short;
    }
    
    /* Final computations using all variables to prevent dead code elimination */
    int final_result = 
        bf.f1 + bf.f2 * 2 + bf.f3 * 3 +
        split.full +
        reg_var +
        reg_short * 4 +
        l1_obj.l2_array[0]->l3->data[0] +
        global_result;
    
    /* Use inline assembly to ensure operations aren't optimized away */
    asm volatile ("" : : "r"(final_result));
    
    return final_result % 256;
}
