/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 8;
    volatile unsigned int f4 : 16;
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
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int extra;
};

struct level1 {
    volatile struct level2 *l2_array[8];
    volatile int count;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    volatile struct bitfields bf = {0};
    
    /* 2. Union for STRICT_LOW_PART */
    union split_int split = {0};
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    register short reg_short asm("ax");
    
    /* 4. Complex memory structures */
    struct level3 l3_obj = {{1, 2, 3, 4}};
    struct level2 l2_obj = {&l3_obj, 42};
    struct level1 l1_obj;
    
    /* Initialize pointer chain */
    for (int i = 0; i < 8; i++) {
        l1_obj.l2_array[i] = &l2_obj;
    }
    l1_obj.count = 8;
    
    /* 5. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* 6. Multi-dimensional array for complex MEM addressing */
    volatile int md_array[4][8][16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                md_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    volatile int *ptr1, *ptr2, *ptr3;
    volatile int idx1, idx2, idx3;
    
    /* Main loop with combined operations */
    for (int i = 0; i < 100; i++) {
        /* Loop condition based on operations */
        if (i >= 100) break;
        
        /* 1. ZERO_EXTRACT: Bitfield assignments with non-byte-aligned fields */
        bf.f2 = (i * 3) & 0x1F;      /* 5-bit field */
        bf.f3 = (i * 7) & 0xFF;      /* 8-bit field */
        bf.f4 = (i * 13) & 0xFFFF;   /* 16-bit field */
        
        /* Force compiler to handle bitfield as integer */
        global_counter += *(volatile unsigned int*)&bf;
        
        /* 2. STRICT_LOW_PART: Partial register updates */
        /* Update low 16 bits only */
        split.parts.low = (i * 11) & 0xFFFF;
        
        /* Alternative: pointer casting for partial update */
        volatile uint16_t *short_ptr = (volatile uint16_t*)&split.full;
        short_ptr[0] = (i * 17) & 0xFFFF;  /* Low part */
        short_ptr[1] = (i * 19) & 0xFFFF;  /* High part */
        
        /* 3. SUBREG: Register variable operations with truncation */
        /* Operation that requires truncation to 16 bits */
        reg_short = (reg_var >> 8) & 0xFFFF;
        
        /* Vector operations that may generate SUBREG */
        vec = vec + vec2;
        volatile int vec_element = vec[1];  /* Extract element */
        
        /* 4. Complex MEM addressing modes */
        /* Multi-level pointer dereferencing */
        idx1 = (i * 2) % 4;
        idx2 = (i * 3) % 8;
        idx3 = (i * 5) % 16;
        
        ptr1 = &md_array[idx1][idx2][idx3];
        ptr2 = &md_array[idx2][idx3][idx1];
        ptr3 = &md_array[idx3][idx1][idx2];
        
        /* Chain of memory accesses */
        *ptr1 = *ptr2 + *ptr3;
        
        /* Structure pointer chain */
        volatile int chain_val = l1_obj.l2_array[i % 8]->l3->data[i % 4];
        l1_obj.l2_array[(i + 1) % 8]->l3->data[(i + 2) % 4] = chain_val + i;
        
        /* 5. Inline assembly to influence RTL generation */
        /* Force use of specific registers and partial registers */
        asm volatile (
            "movw %w[input], %%ax\n\t"          /* Use low 16-bit subregister */
            "addw $1, %%ax\n\t"                /* Operate on subregister */
            "movw %%ax, %w[output]\n\t"        /* Output to low 16 bits */
            : [output] "=r" (reg_var)          /* Tie to register variable */
            : [input] "r" (reg_var)
            : "ax", "cc"                       /* Clobber ax and flags */
        );
        
        /* Memory addressing with constraints */
        volatile int mem_temp;
        asm volatile (
            "movl %1, %0\n\t"
            "addl $100, %0\n\t"
            : "=r" (mem_temp)
            : "m" (*ptr1)                      /* Memory constraint */
            : "cc"
        );
        
        /* 6. Conditional context to affect scheduling */
        switch (i % 4) {
            case 0:
                bf.f1 = (bf.f2 + bf.f3) & 0x7;
                break;
            case 1:
                split.parts.high = split.parts.low ^ 0xAAAA;
                break;
            case 2:
                reg_var = (reg_var << 4) | (reg_var >> 28);
                break;
            case 3:
                /* Complex memory update */
                l1_obj.l2_array[i % 8]->extra = 
                    l1_obj.l2_array[(i + 1) % 8]->l3->data[i % 4];
                break;
        }
        
        /* Prevent loop elimination */
        if (i == 50) {
            /* Create data dependency */
            split.full = global_counter;
        }
    }
    
    /* Combine all results to prevent dead code elimination */
    global_result = 
        *(volatile unsigned int*)&bf + 
        split.full + 
        reg_var + 
        vec[0] + 
        md_array[0][0][0] + 
        l1_obj.l2_array[0]->l3->data[0];
    
    return global_result & 0xFF;
}
