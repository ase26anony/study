/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 11;
    volatile unsigned int field4 : 9;
} bitfield_struct;

/* For SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* For complex memory addressing */
typedef struct node {
    int data;
    struct node *next;
    int array[3][4];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
static inline void manipulate_registers(volatile int *out) {
    register int eax_var asm("eax") = *out;
    register short ax_var asm("ax");
    
    /* Force SUBREG through partial register access */
    asm volatile("movw %w1, %w0" 
                 : "=r"(ax_var) 
                 : "r"(eax_var & 0xFFFF));
    
    /* Use the result */
    *out = ax_var;
}

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    volatile bitfield_struct bf = {0};
    volatile uint32_t large_int = 0xDEADBEEF;
    
    /* 2. Union for STRICT_LOW_PART */
    union {
        volatile uint32_t full;
        volatile uint16_t parts[2];
        volatile uint8_t bytes[4];
    } reg_union = {0};
    
    /* 3. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    
    /* 4. Complex memory structures */
    node_t nodes[4];
    node_t *current = &nodes[0];
    
    /* Initialize memory structures */
    for (int i = 0; i < 4; i++) {
        nodes[i].data = i * 100;
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                nodes[i].array[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Main loop with combined operations */
    for (int i = 0; i < 100; i++) {
        /* Exit condition based on operations */
        if (global_counter > 1000) break;
        
        /* ZERO_EXTRACT: Bitfield assignments */
        bf.field1 = (i & 0x1F);           /* 5-bit field */
        bf.field2 = ((i * 3) & 0x7F);     /* 7-bit field */
        bf.field3 = ((i * 7) & 0x7FF);    /* 11-bit field */
        bf.field4 = ((i * 11) & 0x1FF);   /* 9-bit field */
        
        /* STRICT_LOW_PART: Partial register updates */
        reg_union.parts[0] = (i & 0xFFFF);        /* Low 16 bits */
        reg_union.bytes[2] = (i * 2) & 0xFF;      /* Third byte */
        
        /* Complex assignment with partial update */
        ((volatile uint16_t*)&large_int)[0] = i & 0xFFFF;
        ((volatile uint8_t*)&large_int)[2] = (i * 3) & 0xFF;
        
        /* SUBREG: Vector element extraction */
        int elem = vec[i % 4];
        volatile int temp = elem;
        
        /* Register variable manipulation */
        manipulate_registers(&temp);
        
        /* Complex MEM addressing with multiple levels */
        int val1 = current->array[(i % 3)][(i % 4)];
        int val2 = current->next ? current->next->array[(i % 2)][(i % 3)] : 0;
        int val3 = current->next && current->next->next ? 
                   current->next->next->array[0][i % 4] : 0;
        
        /* Pointer arithmetic creating complex addresses */
        int *ptr = &current->array[0][0];
        ptr += (i % 12);
        volatile int deref = *ptr;
        
        /* Update current pointer with complex addressing */
        current = (node_t*)((char*)current + sizeof(int) * (i % 3));
        if (current >= &nodes[4] || current < &nodes[0]) {
            current = &nodes[0];
        }
        
        /* Inline assembly with constraints */
        register int r1 asm("ebx") = val1;
        register int r2 asm("ecx") = val2;
        
        asm volatile(
            "addl %%ecx, %%ebx\n\t"
            "movw %%bx, %w0\n\t"
            : "=r"(temp)
            : "r"(r1), "r"(r2)
            : "cc"
        );
        
        /* Combine results */
        global_result += bf.field1 + bf.field2 + reg_union.parts[0] + 
                        deref + temp + val3;
        
        /* Update counter with conditional */
        if (global_result & 1) {
            global_counter += 2;
        } else {
            global_counter += 1;
        }
        
        /* Modify vector element (triggers SUBREG in store) */
        vec[i % 4] = global_result % 100;
        
        /* Additional memory chain */
        volatile node_t **pptr = &current;
        if (*pptr && (*pptr)->next) {
            volatile int chain_val = (*pptr)->next->data;
            global_result ^= chain_val;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = 
        (bf.field1 << 24) | 
        (bf.field2 << 16) | 
        (reg_union.parts[0] << 8) | 
        (global_result & 0xFF);
    
    /* Use all variables in return calculation */
    return final_result + 
           (large_int & 0xFF) + 
           vec[0] + 
           (current ? current->data : 0) + 
           global_counter;
}
