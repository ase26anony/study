/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */
/* For RTL analysis: add -dP -fdump-rtl-all -fdump-rtl-expand */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int f1 : 3;    /* Not byte-aligned */
    volatile unsigned int f2 : 5;    /* Crosses byte boundary */
    volatile unsigned int f3 : 12;   /* Multiple bytes */
    volatile unsigned int f4 : 8;    /* Byte-aligned but in middle */
    volatile unsigned int f5 : 4;    /* End of structure */
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

/* 3. Complex nested structure for memory addressing */
typedef struct node {
    volatile int value;
    volatile struct node* next;
    volatile int array[3][4];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for SUBREG operations */
int process_with_subreg(volatile int x) {
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = x;
    volatile int result;
    
    /* Inline assembly that uses partial registers */
    asm volatile (
        "movw %%ax, %[low]\n\t"           /* STRICT_LOW_PART style */
        "shrl $16, %%eax\n\t"             /* SUBREG for high part */
        "movb %%al, %[high]\n\t"          /* Another SUBREG */
        : [low] "=m" (((volatile short*)&result)[0]),
          [high] "=m" (((volatile char*)&result)[2])
        : "a" (reg_var)
        : "cc"
    );
    
    return result;
}

/* Function with complex memory addressing */
int complex_memory_access(node_t* volatile ptr, int i, int j) {
    /* 4. Multi-level pointer dereferencing */
    volatile int val = ptr->next->next->array[i][j];
    val += ptr->array[1][2];
    val += ptr->next->value;
    
    /* More complex addressing */
    volatile int* addr = &ptr->next->array[0][0];
    addr += i * 4 + j;
    return *addr + val;
}

int main() {
    /* Initialize variables */
    volatile bitfield_struct bf = {0};
    volatile split_int split = {0};
    volatile node_t nodes[4];
    volatile int i, j, k;
    
    /* Initialize node chain */
    for (i = 0; i < 4; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 3) ? &nodes[i + 1] : &nodes[0];
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 4; k++) {
                nodes[i].array[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Main loop with all required constructs */
    for (i = 0; i < 100; i++) {
        /* 5. Conditional context */
        if (i % 3 == 0) {
            /* 1. Bitfield assignments (ZERO_EXTRACT) */
            bf.f1 = (i & 0x7);                    /* 3-bit field */
            bf.f2 = ((i >> 3) & 0x1F);            /* 5-bit field crossing boundary */
            bf.f3 = ((i * 7) & 0xFFF);            /* 12-bit field */
            bf.f4 = ((i + 5) & 0xFF);             /* 8-bit field */
            bf.f5 = ((i >> 1) & 0xF);             /* 4-bit field */
            
            global_counter += bf.f1 + bf.f2;
        } 
        else if (i % 3 == 1) {
            /* 2. STRICT_LOW_PART assignments */
            split.parts.low = i * 3;              /* Updates low 16 bits */
            split.bytes[1] = i & 0xFF;            /* Updates middle byte */
            
            /* Another STRICT_LOW_PART pattern */
            ((volatile short*)&split.full)[0] = i * 2;
            ((volatile char*)&split.full)[3] = i;
            
            global_counter += split.parts.low;
        }
        else {
            /* 3. SUBREG operations via function call */
            volatile int subreg_result = process_with_subreg(i * 17);
            
            /* 4. Complex memory addressing */
            volatile int mem_result = complex_memory_access(
                &nodes[i % 4], 
                (i / 4) % 3, 
                (i / 2) % 4
            );
            
            global_counter += subreg_result + mem_result;
        }
        
        /* 6. Additional inline assembly with constraints */
        volatile int temp = i * 11;
        register int r1 asm("ebx") = temp;
        register int r2 asm("ecx") = i;
        
        asm volatile (
            "addl %%ecx, %%ebx\n\t"
            "movb %%bl, %[byte]\n\t"              /* SUBREG extract */
            "movw %%bx, %[word]\n\t"              /* STRICT_LOW_PART */
            : [byte] "=m" (split.bytes[0]),
              [word] "=m" (((volatile short*)&temp)[0])
            : "b" (r1), "c" (r2)
            : "cc"
        );
        
        /* Combine all results */
        global_result ^= bf.f3 ^ split.full ^ temp;
        
        /* Loop exit condition based on operations */
        if (global_counter > 10000) {
            break;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = 
        global_result + 
        global_counter + 
        bf.f5 + 
        split.parts.high +
        nodes[0].array[0][0];
    
    return final_result % 256;
}
