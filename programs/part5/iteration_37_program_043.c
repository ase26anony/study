/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Requirement 1: Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} bitfield_struct;

/* Requirement 2: Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Requirement 3: Vector type for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Requirement 4: Complex structure for memory addressing */
typedef struct node {
    volatile int value;
    volatile struct node* next;
    volatile int array[3][4];
} node_t;

/* Requirement 6: Inline assembly helpers */
static inline void clobber_registers(void) {
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
}

static inline uint16_t read_low_part(volatile uint32_t* val) {
    uint16_t result;
    asm volatile("movw %1, %0" : "=r"(result) : "m"(*val));
    return result;
}

int main(void) {
    /* Initialize variables */
    volatile bitfield_struct bf = {0};
    volatile split_int si = {.full = 0x12345678};
    volatile v4si vec = {1, 2, 3, 4};
    volatile int temp;
    
    /* Requirement 4: Complex memory structures */
    node_t nodes[4];
    for (int i = 0; i < 4; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                nodes[i].array[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    volatile node_t* current = &nodes[0];
    volatile int* volatile ptr_array[2];
    ptr_array[0] = &nodes[0].value;
    ptr_array[1] = &nodes[1].value;
    
    /* Requirement 3: Register variable for SUBREG */
    register int reg_var asm("eax") = 0xABCDEF12;
    
    /* Loop with conditional branches */
    volatile int counter = 0;
    volatile int exit_condition = 100;
    
    while (counter++ < exit_condition) {
        /* Requirement 5: Conditional context */
        if (counter % 3 == 0) {
            /* Requirement 1: Bitfield assignment (ZERO_EXTRACT) */
            bf.field1 = (counter & 0x7);
            bf.field3 = (counter * 2) & 0xFF;
        } else if (counter % 3 == 1) {
            /* Requirement 2: STRICT_LOW_PART via union */
            si.parts.low = counter & 0xFFFF;
            
            /* Alternative STRICT_LOW_PART via pointer cast */
            *((volatile uint16_t*)&si.full) = (counter * 3) & 0xFFFF;
        } else {
            /* Requirement 2: Another STRICT_LOW_PART pattern */
            si.bytes[1] = counter & 0xFF;
        }
        
        /* Requirement 3: SUBREG operations with vector */
        temp = vec[2];  /* Extract element - may generate SUBREG */
        vec[1] = temp + counter;
        
        /* Requirement 3: SUBREG with register variable */
        reg_var = (reg_var & 0xFFFF0000) | (counter & 0xFFFF);
        volatile uint16_t low_part = reg_var;  /* Truncation */
        
        /* Requirement 4: Complex memory addressing (MEM) */
        if (current && current->next) {
            /* Multi-level pointer dereferencing */
            temp = current->next->array[1][2];
            
            /* Complex array indexing */
            int idx1 = counter % 3;
            int idx2 = (counter * 7) % 4;
            temp = current->array[idx1][idx2];
            
            /* Pointer chain */
            temp = ptr_array[counter % 2][0];
            
            /* Update through complex address */
            current->array[(counter / 10) % 3][(counter / 5) % 4] = temp + 1;
        }
        
        /* Requirement 6: Inline assembly influencing RTL */
        asm volatile(
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (si.full)
            : "r" (counter)
            : "eax", "cc"
        );
        
        /* More inline assembly with constraints */
        uint32_t dummy;
        asm volatile(
            "movl %%eax, %0\n\t"
            "movw %%ax, %1\n\t"
            : "=r" (dummy), "=Q" (si.bytes[0])
            : 
            : "eax"
        );
        
        /* Clobber to prevent optimization */
        clobber_registers();
        
        /* Switch statement for more control flow */
        switch (counter % 4) {
            case 0:
                bf.field2 = (bf.field1 + 1) & 0x1F;
                break;
            case 1:
                si.parts.high = read_low_part(&si.full);
                break;
            case 2:
                reg_var = (reg_var >> 8) | (reg_var << 24);
                break;
            case 3:
                if (current) {
                    current = current->next;
                }
                break;
        }
    }
    
    /* Combine results to prevent dead code elimination */
    volatile int result = 
        bf.field1 + bf.field2 + bf.field3 + bf.field4 +
        si.full + 
        vec[0] + vec[1] + vec[2] + vec[3] +
        reg_var +
        (current ? current->value : 0) +
        temp;
    
    return result % 256;
}
