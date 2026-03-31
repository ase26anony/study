/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 8;
    volatile unsigned int f4 : 16;
} __attribute__((packed));

/* 2. Union for STRICT_LOW_PART operations */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* 3. Complex memory structure */
struct nested {
    volatile int data[4][4];
    volatile struct nested *next;
};

/* 4. Register variable for SUBREG */
register volatile uint32_t reg_var asm ("r12");

/* 5. Vector type for SUBREG operations */
typedef int v4si __attribute__ ((vector_size (16)));

/* Function with inline assembly to influence RTL generation */
static inline void asm_constraints(volatile uint32_t *mem, uint32_t val) {
    /* Use 'h' modifier for high byte, 'Q' for memory */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%ah, %0\n\t"
        : "=Q" (*mem)
        : "r" (val)
        : "eax", "memory"
    );
}

int main(int argc, char *argv[]) {
    volatile int result = 0;
    
    /* Initialize variables */
    struct bitfields bf = {0};
    union split_int si = {0};
    volatile v4si vec = {1, 2, 3, 4};
    volatile uint32_t *dyn_mem = (uint32_t*)malloc(64 * sizeof(uint32_t));
    
    /* Complex memory structure */
    struct nested n1, n2;
    n1.next = &n2;
    n2.next = &n1;
    
    /* Initialize array data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            n1.data[i][j] = i * 4 + j;
            n2.data[i][j] = 16 + i * 4 + j;
        }
    }
    
    /* Loop with mixed operations */
    for (volatile int i = 0; i < (argc > 1 ? atoi(argv[1]) : 100); i++) {
        /* 1. ZERO_EXTRACT: Bitfield assignments */
        bf.f1 = (i & 0x7);           /* 3-bit field */
        bf.f3 = (i * 3) & 0xFF;      /* 8-bit field */
        bf.f4 = (i * 7) & 0xFFFF;    /* 16-bit field */
        
        /* 2. STRICT_LOW_PART: Partial register update */
        si.parts.low = (i * 5) & 0xFFFF;
        si.parts.high = (i * 11) & 0xFFFF;
        
        /* Alternative STRICT_LOW_PART via pointer */
        volatile uint32_t *pint = &si.full;
        ((volatile uint16_t*)pint)[0] = (i * 13) & 0xFFFF;
        
        /* 3. SUBREG: Register variable operations */
        reg_var = i * 17;
        volatile uint16_t reg_low = reg_var & 0xFFFF;  /* Implicit truncation */
        
        /* Vector SUBREG operations */
        volatile int vec_elem = vec[i % 4];  /* Element extraction */
        vec[i % 4] = i * 19;                 /* Element insertion */
        
        /* 4. Complex MEM addressing */
        /* Multi-level pointer dereferencing */
        volatile int mem_val = n1.next->next->data[i % 4][(i / 4) % 4];
        
        /* Array indexing with non-constant expressions */
        dyn_mem[(i * 23) % 64] = mem_val + vec_elem;
        
        /* Chain of pointer operations */
        volatile struct nested *cur = &n1;
        for (int j = 0; j < (i % 3); j++) {
            cur = cur->next;
        }
        cur->data[i % 4][(i + 1) % 4] = i * 29;
        
        /* 5. Inline assembly with constraints */
        asm_constraints(&dyn_mem[i % 16], i * 31);
        
        /* Conditional to create branching */
        if (i & 1) {
            /* More bitfield ops in branch */
            bf.f2 = (i * 37) & 0x1F;
            result += bf.f1 + bf.f2;
        } else {
            /* Different MEM access pattern */
            result += n2.data[(i * 41) % 4][(i * 43) % 4];
        }
        
        /* Switch for additional control flow */
        switch (i % 4) {
            case 0:
                si.full = reg_var + vec_elem;
                break;
            case 1:
                reg_var = si.full * 2;
                break;
            case 2:
                /* Mixed operation */
                bf.f3 = (reg_var >> 8) & 0xFF;
                break;
            case 3:
                /* Complex address calculation */
                volatile int *addr = &cur->data[0][0] + (i % 16);
                *addr = result & 0xFF;
                break;
        }
        
        /* Prevent loop optimization */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    /* Combine all results */
    result += bf.f1 + bf.f2 + bf.f3 + bf.f4;
    result += si.full;
    result += reg_var;
    result += vec[0] + vec[1] + vec[2] + vec[3];
    result += n1.data[0][0] + n2.data[0][0];
    result += dyn_mem[0] + dyn_mem[63];
    
    free((void*)dyn_mem);
    
    return result & 0xFF;  /* Return non-zero to prevent optimization */
}
