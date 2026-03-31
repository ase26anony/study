/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers */

#include <stdint.h>
#include <stdlib.h>

/* Requirement 1: Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t low : 5;
    volatile uint32_t middle : 11;
    volatile uint32_t high : 16;
} bitfield_struct;

/* Requirement 3: Vector extension for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Requirement 2: Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Requirement 4: Complex memory structures */
typedef struct node {
    int data;
    struct node *next;
    int array[3][3];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Requirement 6: Inline assembly helpers */
static inline void clobber_registers(void) {
    /* Clobber specific registers to force reloads */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
}

static inline uint16_t read_low_half(uint32_t val) {
    uint16_t result;
    /* Force use of low part of register */
    asm volatile("movw %w1, %w0" : "=r"(result) : "r"(val));
    return result;
}

int main(void) {
    /* Requirement 1: Bitfield initialization */
    bitfield_struct bf = {0};
    volatile bitfield_struct *bf_ptr = &bf;
    
    /* Requirement 2: Split integer initialization */
    split_int si = {0};
    volatile split_int *si_ptr = &si;
    
    /* Requirement 3: Register variable for SUBREG */
    register uint32_t reg_var asm("ebx") = 0x12345678;
    volatile uint32_t *reg_ref = &reg_var;
    
    /* Requirement 3: Vector variable */
    v4si vec = {1, 2, 3, 4};
    volatile v4si *vec_ptr = &vec;
    
    /* Requirement 4: Complex memory structure */
    node_t nodes[4];
    node_t *current = &nodes[0];
    
    /* Initialize linked structure */
    for (int i = 0; i < 3; i++) {
        nodes[i].next = &nodes[i + 1];
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                nodes[i].array[j][k] = i * 9 + j * 3 + k;
            }
        }
    }
    nodes[3].next = NULL;
    
    /* Requirement 5: Main loop with all constructs */
    for (int i = 0; i < 100; i++) {
        /* Requirement 5: Conditional context */
        if (i % 3 == 0) {
            /* Requirement 1: Bitfield assignment (ZERO_EXTRACT) */
            bf_ptr->low = (i & 0x1F);
            bf_ptr->middle = ((i * 7) & 0x7FF);
            bf_ptr->high = ((i * 13) & 0xFFFF);
            
            /* Combine with inline assembly */
            clobber_registers();
        }
        else if (i % 3 == 1) {
            /* Requirement 2: STRICT_LOW_PART via union */
            si_ptr->parts.low = (uint16_t)(i * 11);
            si_ptr->bytes[2] = (uint8_t)(i * 3);
            
            /* Alternative: Pointer cast for STRICT_LOW_PART */
            *((volatile uint16_t*)&si.full) = (uint16_t)(i * 17);
        }
        else {
            /* Requirement 2: More STRICT_LOW_PART patterns */
            si_ptr->parts.high = (uint16_t)(i * 5);
            *((volatile uint8_t*)&si.full + 1) = (uint8_t)(i * 7);
        }
        
        /* Requirement 3: SUBREG operations with register variable */
        {
            /* Force truncation through smaller type operation */
            uint16_t temp = read_low_half(reg_var);
            temp += i;
            
            /* Vector element access (triggers SUBREG) */
            int vec_elem = vec_ptr[0][i % 4];
            vec_elem += temp;
            
            /* Update register variable through pointer */
            *reg_ref = (*reg_ref & 0xFFFF0000) | temp;
            
            /* More SUBREG: access high byte */
            uint8_t high_byte = (*reg_ref >> 24) & 0xFF;
            high_byte ^= vec_elem & 0xFF;
            *reg_ref = (*reg_ref & 0x00FFFFFF) | (high_byte << 24);
        }
        
        /* Requirement 4: Complex memory addressing (MEM) */
        {
            /* Multi-level pointer dereferencing */
            int val1 = current->array[(i / 3) % 3][i % 3];
            
            /* Chain of pointer accesses */
            node_t *next = current->next;
            if (next) {
                int val2 = next->array[(i / 2) % 3][(i + 1) % 3];
                current->data = val1 + val2;
                
                /* Even more complex addressing */
                next->array[0][0] = current->array[2][2] + 
                                   next->array[1][1] + 
                                   bf_ptr->low;
            }
            
            /* Update current pointer with complex expression */
            current = &nodes[(current->data + i) % 4];
        }
        
        /* Requirement 6: Inline assembly tying everything together */
        asm volatile(
            "addl %1, %0\n\t"
            "rorw $4, %w0\n\t"
            : "+r"(reg_var)
            : "r"(si.full & 0xFF)
            : "cc"
        );
        
        /* Mix with memory access */
        asm volatile(
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m"(current->data)
            : "r"(bf_ptr->middle)
            : "%eax", "memory"
        );
        
        /* Update global state to prevent elimination */
        global_counter++;
        global_result ^= bf_ptr->low ^ si.parts.low ^ (reg_var & 0xFF) ^ current->data;
    }
    
    /* Additional switch statement for more conditional complexity */
    switch (global_result & 0x7) {
        case 0:
            /* More bitfield operations */
            bf.high = (bf.low << 3) | (bf.middle & 0x7);
            break;
        case 1:
            /* More STRICT_LOW_PART */
            *((volatile uint16_t*)&si.full + 1) = bf.middle;
            break;
        case 2:
            /* More SUBREG through vector */
            vec[0] = vec[1] + vec[2] - vec[3];
            break;
        case 3:
            /* Complex memory chain */
            nodes[0].next->next->array[1][1] = global_result;
            break;
        default:
            /* Mixed operations */
            reg_var = (si.parts.high << 16) | bf.middle;
            current->array[0][0] = reg_var;
            break;
    }
    
    /* Final computation using all variables */
    int final_result = 
        (int)bf.low + 
        (int)bf.middle + 
        (int)bf.high +
        si.parts.low +
        si.parts.high +
        (reg_var & 0xFFFFFF) +
        vec[0] + vec[1] + vec[2] + vec[3] +
        current->data +
        global_counter;
    
    return final_result % 256;
}
