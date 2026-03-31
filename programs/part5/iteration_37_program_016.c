/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t low : 5;
    volatile uint32_t mid : 11;
    volatile uint32_t high : 16;
} bitfield_t;

/* Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int_t;

/* Complex memory structure */
typedef struct {
    int data[8];
    struct node *next;
} node;

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int *global_ptr;

/* Function with inline assembly for register constraints */
static inline void manipulate_registers(uint32_t *a, uint32_t *b) {
    register uint32_t reg_a asm("eax") = *a;
    register uint32_t reg_b asm("ebx") = *b;
    
    /* Inline assembly that forces subregister operations */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "movb %%al, %%bl\n\t"      /* Low byte extraction */
        "movw %%ax, %%cx\n\t"      /* Low word extraction */
        : "+r" (reg_a), "+r" (reg_b)
        :
        : "ecx", "memory"
    );
    
    *a = reg_a;
    *b = reg_b;
}

int main(void) {
    /* Initialize bitfield struct */
    bitfield_t bf = {0};
    volatile bitfield_t *bf_ptr = &bf;
    
    /* Initialize split integer */
    split_int_t split = {0};
    volatile split_int_t *split_ptr = &split;
    
    /* Initialize register variables */
    register uint32_t reg_var asm("edx") = 0x12345678;
    volatile uint32_t *reg_var_ptr = &reg_var;
    
    /* Initialize complex memory structures */
    node *head = malloc(sizeof(node));
    node *current = head;
    
    for (int i = 0; i < 8; i++) {
        current->data[i] = i * 100;
    }
    current->next = malloc(sizeof(node));
    current = current->next;
    
    for (int i = 0; i < 8; i++) {
        current->data[i] = i * 200;
    }
    current->next = NULL;
    
    /* Initialize vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    volatile v4si *vec_ptr = &vec;
    
    /* Multi-dimensional array for complex MEM addressing */
    int matrix[4][8][16];
    volatile int (*matrix_ptr)[8][16] = matrix;
    
    /* Main loop targeting all uncovered expressions */
    for (int i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT via bitfield assignments */
        bf_ptr->low = (i & 0x1F);           /* 5-bit field */
        bf_ptr->mid = (i * 37) & 0x7FF;     /* 11-bit field */
        bf_ptr->high = (i * 73) & 0xFFFF;   /* 16-bit field */
        
        /* 2. STRICT_LOW_PART via partial register updates */
        split_ptr->parts.low = i * 2;       /* Updates low 16 bits */
        split_ptr->bytes[1] = i & 0xFF;     /* Updates single byte */
        
        /* 3. SUBREG via register variable operations */
        reg_var = (reg_var * 1103515245 + 12345) & 0x7FFFFFFF;
        uint16_t low_word = reg_var & 0xFFFF;  /* Forces truncation */
        uint8_t low_byte = reg_var & 0xFF;     /* More truncation */
        
        /* Mix with inline assembly */
        manipulate_registers((uint32_t*)&reg_var, (uint32_t*)&split.full);
        
        /* 4. Complex MEM addressing modes */
        /* Multi-level pointer dereferencing */
        int val1 = head->next->data[i % 8];
        
        /* Array indexing with non-constant expressions */
        int val2 = matrix_ptr[(i / 4) % 4][(i / 2) % 8][i % 16];
        
        /* Structure pointer chain with computation */
        matrix_ptr[(i / 4) % 4][(i / 2) % 8][i % 16] = 
            val1 + val2 + bf_ptr->low + split_ptr->parts.low;
        
        /* 5. Vector operations for SUBREG extraction */
        int vec_element = vec_ptr[0][i % 4];  /* Element extraction */
        vec_ptr[0][(i + 1) % 4] = vec_element + i;
        
        /* Conditional branch to create more RTL complexity */
        if (i % 3 == 0) {
            /* Additional bitfield manipulation */
            bf_ptr->mid = (bf_ptr->mid ^ bf_ptr->low) & 0x7FF;
            split_ptr->parts.high = bf_ptr->high;
        } else if (i % 3 == 1) {
            /* More complex memory addressing */
            int ***triple_ptr = (int***)matrix_ptr;
            triple_ptr[0][(i / 8) % 8][i % 16] += vec_element;
        } else {
            /* Register-intensive operations */
            asm volatile (
                "movl %0, %%eax\n\t"
                "shrl $8, %%eax\n\t"      /* Creates SUBREG for high bytes */
                "movb %%al, %1\n\t"
                : 
                : "r" (reg_var), "m" (split.bytes[2])
                : "eax", "memory"
            );
        }
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                bf_ptr->low = (bf_ptr->low + 1) & 0x1F;
                break;
            case 1:
                split_ptr->parts.low = (split_ptr->parts.low * 3) & 0xFFFF;
                break;
            case 2:
                /* Pointer arithmetic creating complex addresses */
                int *ptr = &matrix[0][0][0];
                ptr[(i * 13) % 512] = reg_var;
                break;
            case 3:
                /* Combined operations */
                reg_var = (reg_var << 4) | (bf_ptr->low & 0xF);
                break;
        }
        
        /* Update global to prevent elimination */
        global_counter += bf_ptr->low + split_ptr->parts.low + 
                         (reg_var & 0xFF) + vec_element;
    }
    
    /* Clean up */
    free(head->next);
    free(head);
    
    /* Return value based on all operations */
    return (global_counter & 0xFF) | 
           ((bf.high & 0xFF) << 8) | 
           ((split.parts.low & 0xFF) << 16) |
           ((reg_var & 0xFF) << 24);
}
