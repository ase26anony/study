/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t field1 : 5;
    volatile uint32_t field2 : 7;
    volatile uint32_t field3 : 9;
    volatile uint32_t field4 : 11;
} bitfield_struct;

/* 2. Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* 3. Complex memory structure */
typedef struct {
    int data[8];
    struct inner {
        int matrix[3][3];
        int *ptr_array[4];
    } *inner_ptr;
} nested_struct;

/* 4. Register variable declaration */
register uint32_t reg_var asm("r12");

/* Function with inline assembly for direct RTL influence */
static inline void manipulate_with_asm(volatile uint32_t *mem, uint32_t val) {
    /* Use 'h' modifier for high byte, 'Q' for memory */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%ah, %0\n\t"
        : "=Q"(*mem)
        : "r"(val)
        : "eax", "memory"
    );
}

/* Main function with loop combining all patterns */
int main(void) {
    volatile bitfield_struct bf = {0};
    volatile split_int split = {0};
    volatile nested_struct *ns = NULL;
    volatile int ***multi_ptr = NULL;
    volatile int counter = 0;
    
    /* Allocate and initialize complex memory structure */
    ns = (volatile nested_struct*)malloc(sizeof(nested_struct));
    if (!ns) return -1;
    
    ns->inner_ptr = (volatile struct inner*)malloc(sizeof(struct inner));
    if (!ns->inner_ptr) {
        free((void*)ns);
        return -1;
    }
    
    /* Setup pointer chain for complex MEM addressing */
    volatile int **ptr1 = (volatile int**)malloc(4 * sizeof(int*));
    volatile int *ptr2 = (volatile int*)malloc(16 * sizeof(int));
    multi_ptr = (volatile int***)malloc(3 * sizeof(int**));
    
    if (!ptr1 || !ptr2 || !multi_ptr) {
        free((void*)ptr2);
        free((void*)ptr1);
        free((void*)ns->inner_ptr);
        free((void*)ns);
        return -1;
    }
    
    for (int i = 0; i < 4; i++) {
        ptr1[i] = ptr2 + i * 4;
    }
    multi_ptr[0] = (volatile int**)ptr1;
    multi_ptr[1] = (volatile int**)ptr1;
    multi_ptr[2] = (volatile int**)ptr1;
    
    /* Initialize array data */
    for (int i = 0; i < 16; i++) {
        ptr2[i] = i * 2;
    }
    
    /* Main loop with combined patterns */
    for (reg_var = 0; reg_var < 100; reg_var++) {
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        bf.field1 = (reg_var & 0x1F);          /* 5-bit field */
        bf.field2 = ((reg_var >> 5) & 0x7F);   /* 7-bit field */
        bf.field3 = ((reg_var >> 12) & 0x1FF); /* 9-bit field */
        bf.field4 = ((reg_var >> 21) & 0x7FF); /* 11-bit field */
        
        /* 2. STRICT_LOW_PART via union/pointer */
        if (reg_var % 3 == 0) {
            split.parts.low = reg_var & 0xFFFF;      /* Update low 16 bits */
        } else if (reg_var % 3 == 1) {
            split.bytes[1] = (reg_var >> 8) & 0xFF;  /* Update single byte */
        } else {
            /* Pointer cast for partial update */
            *((volatile uint16_t*)&split.full) = reg_var & 0xFFFF;
        }
        
        /* 3. SUBREG via register variable operations */
        volatile uint16_t short_var;
        /* Truncation operation that may generate SUBREG */
        short_var = reg_var & 0xFFFF;
        
        /* Mix with inline assembly */
        manipulate_with_asm(&split.full, reg_var);
        
        /* 4. Complex MEM addressing with pointer chains */
        if (ns && ns->inner_ptr) {
            /* Multi-level dereferencing: ns->inner_ptr->matrix[i][j] */
            int i = reg_var % 3;
            int j = (reg_var / 3) % 3;
            ns->inner_ptr->matrix[i][j] = reg_var;
            
            /* Even more complex: ***multi_ptr */
            if (multi_ptr && multi_ptr[0] && multi_ptr[0][i]) {
                multi_ptr[0][i][j] = reg_var * 2;
            }
            
            /* Array indexing with non-constant expression */
            volatile int idx = (reg_var * 7) % 8;
            ns->data[idx] = reg_var;
        }
        
        /* Conditional context */
        switch (reg_var % 4) {
            case 0:
                counter += bf.field1;
                break;
            case 1:
                counter += split.parts.low;
                break;
            case 2:
                counter += short_var;
                break;
            case 3:
                if (ns) {
                    counter += ns->data[reg_var % 8];
                }
                break;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(counter));
    }
    
    /* Use results to prevent dead code elimination */
    int result = bf.field1 + bf.field2 + bf.field3 + bf.field4;
    result += split.full;
    result += counter;
    
    if (ns) {
        for (int i = 0; i < 8; i++) {
            result += ns->data[i];
        }
        free((void*)ns->inner_ptr);
        free((void*)ns);
    }
    
    free((void*)ptr2);
    free((void*)ptr1);
    free((void*)multi_ptr);
    
    return result & 0xFF;  /* Return non-zero result */
}
