/* Target: resource.cc mark_referenced_resources function
 * Specifically targeting ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P handling
 */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Complex struct with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART operations */
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex nested structure for memory addressing */
struct level3 {
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *sub;
    volatile int extra;
};

struct level1 {
    volatile struct level2 *chain;
    volatile int index;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct g_bf = {0};
volatile union type_pun g_pun = {0};
volatile v4si g_vector = {0};
volatile struct level1 *g_mem_chain = NULL;

/* Function with inline assembly for SUBREG operations */
int subreg_operations(volatile int param) {
    /* Explicit register variable for SUBREG operations */
    register int reg_var asm("eax") = param;
    register short short_part asm("ax");
    
    /* Inline assembly that forces SUBREG usage */
    asm volatile (
        "movw %%ax, %0\n\t"
        "movl %1, %%eax\n\t"
        : "=m" (short_part)
        : "r" (reg_var)
        : "eax"
    );
    
    /* Operations that may generate SUBREG */
    volatile char char_val = (reg_var & 0xFF);
    volatile short short_val = (reg_var >> 8) & 0xFFFF;
    
    /* More inline assembly with constraints */
    asm volatile (
        "addb $1, %h0\n\t"  /* 'h' modifier for high byte */
        : "+r" (reg_var)
        :
        : "cc"
    );
    
    return char_val + short_val + short_part;
}

/* Function with complex memory addressing */
int complex_memory_access(volatile struct level1 *chain, int idx1, int idx2) {
    if (!chain || !chain->chain) return 0;
    
    /* Multi-level pointer dereferencing for complex MEM */
    volatile int result = chain->chain->sub->data[idx1 * idx2 % 4];
    
    /* Array indexing with non-constant expressions */
    volatile int *ptr_array[3];
    ptr_array[0] = &chain->chain->sub->data[0];
    ptr_array[1] = &chain->chain->sub->data[1];
    ptr_array[2] = &chain->chain->sub->data[2];
    
    /* Chain of accesses */
    result += ptr_array[idx1 % 3][0];
    result += chain->chain->extra;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int i, j, result = 0;
    volatile int loop_counter = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize memory chain */
    volatile struct level3 l3 = {{1, 2, 3, 4}};
    volatile struct level2 l2 = {&l3, 5};
    volatile struct level1 l1 = {&l2, 0};
    g_mem_chain = &l1;
    
    /* Initialize vector */
    g_vector = (v4si){10, 20, 30, 40};
    
    /* Main loop with combined operations */
    for (i = 0; i < loop_counter; i++) {
        /* Conditional context */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignments */
            g_bf.a = (i & 0x7);           /* 3-bit field */
            g_bf.b = ((i >> 3) & 0x1F);   /* 5-bit field */
            g_bf.c = ((i >> 8) & 0xFF);   /* 8-bit field */
            g_bf.d = ((i >> 16) & 0xFFFF); /* 16-bit field */
            
            result += g_bf.a + g_bf.b;
        }
        else if (i % 3 == 1) {
            /* STRICT_LOW_PART: Partial register updates */
            g_pun.parts.low = (i & 0xFFFF);
            g_pun.bytes[2] = (i >> 16) & 0xFF;
            
            /* Pointer cast for STRICT_LOW_PART */
            *((volatile short*)&g_pun.full + 1) = (i >> 8) & 0xFFFF;
            
            result += g_pun.parts.low;
        }
        else {
            /* SUBREG operations */
            result += subreg_operations(i);
            
            /* Vector operations that may generate SUBREG */
            volatile int elem = g_vector[i % 4];
            g_vector[i % 4] = elem + i;
        }
        
        /* Complex memory addressing (always executed) */
        j = complex_memory_access(g_mem_chain, i, i + 1);
        result += j;
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                /* More bitfield operations */
                g_bf.a = (result & 0x7);
                break;
            case 1:
                /* More partial updates */
                g_pun.bytes[1] = (result & 0xFF);
                break;
            case 2:
                /* Vector element access */
                g_vector[2] = result % 100;
                break;
            case 3:
                /* Memory chain update */
                if (g_mem_chain && g_mem_chain->chain && g_mem_chain->chain->sub) {
                    g_mem_chain->chain->sub->data[0] = result;
                }
                break;
        }
        
        /* Inline assembly that clobbers registers used in C expressions */
        asm volatile (
            "movl %%eax, %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            : 
            : 
            : "eax", "ecx", "cc"
        );
    }
    
    /* Final computation using all modified variables */
    volatile int final_result = 
        g_bf.a + g_bf.b + g_bf.c + g_bf.d +
        g_pun.full +
        g_vector[0] + g_vector[1] + g_vector[2] + g_vector[3];
    
    if (g_mem_chain && g_mem_chain->chain && g_mem_chain->chain->sub) {
        final_result += g_mem_chain->chain->sub->data[0];
    }
    
    /* Return non-zero to prevent optimization */
    return (final_result > 0) ? 0 : 1;
}
