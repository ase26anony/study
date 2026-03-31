/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

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
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Requirement 3: Vector extension for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Requirement 4: Complex memory structures */
typedef struct node {
    int data;
    struct node* next;
    int* array;
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Requirement 6: Inline assembly helpers */
static inline void clobber_registers(void) {
    __asm__ volatile (
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        : : : "eax", "ebx", "ecx", "edx", "memory"
    );
}

static inline uint8_t read_low_byte(uint32_t val) {
    uint8_t result;
    __asm__ volatile (
        "movb %b1, %0\n\t"
        : "=r" (result)
        : "r" (val)
        : "memory"
    );
    return result;
}

int main(void) {
    /* Initialize variables */
    bitfield_struct bf = {0};
    split_int si = {.full = 0x12345678};
    v4si vec = {1, 2, 3, 4};
    
    /* Requirement 3: Register variable for SUBREG */
    register int reg_var asm("eax") = 0xABCDEF12;
    
    /* Requirement 4: Complex memory structure */
    int*** multi_array = malloc(10 * sizeof(int**));
    for (int i = 0; i < 10; i++) {
        multi_array[i] = malloc(10 * sizeof(int*));
        for (int j = 0; j < 10; j++) {
            multi_array[i][j] = malloc(10 * sizeof(int));
            for (int k = 0; k < 10; k++) {
                multi_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    node_t* node_chain = malloc(sizeof(node_t));
    node_chain->array = malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++) {
        node_chain->array[i] = i * 2;
    }
    
    /* Main loop combining all requirements */
    for (volatile int i = 0; i < 100; i++) {
        /* Requirement 1: Bitfield assignments (ZERO_EXTRACT) */
        bf.field1 = (i & 0x7);           /* 3-bit field */
        bf.field2 = ((i >> 3) & 0x1F);   /* 5-bit field */
        bf.field3 = ((i >> 8) & 0xFF);   /* 8-bit field */
        bf.field4 = ((i >> 16) & 0xFFFF);/* 16-bit field */
        
        /* Requirement 2: STRICT_LOW_PART operations */
        if (i % 3 == 0) {
            si.parts.low = i & 0xFFFF;      /* Update low 16 bits */
        } else if (i % 3 == 1) {
            si.bytes[1] = (i >> 8) & 0xFF;  /* Update middle byte */
        } else {
            ((volatile uint16_t*)&si.full)[0] = i & 0xFFFF; /* Another low-part update */
        }
        
        /* Requirement 3: SUBREG operations with register variable */
        reg_var = (reg_var + i) & 0xFFFF;  /* Truncation to 16 bits */
        
        /* Mix with vector operations */
        v4si temp_vec = vec + (v4si){i, i, i, i};
        int element = temp_vec[2];  /* Extract element (potential SUBREG) */
        
        /* Requirement 4: Complex memory addressing */
        if (i % 7 == 0) {
            /* Multi-level pointer dereferencing */
            int complex_val = multi_array[i % 10][(i + 1) % 10][(i + 2) % 10];
            node_chain->array[i % 100] = complex_val + i;
            
            /* Chain of pointer accesses */
            int* ptr = &node_chain->array[i % 100];
            *ptr = (*ptr + multi_array[(i + 3) % 10][(i + 4) % 10][(i + 5) % 10]) & 0xFF;
        }
        
        /* Requirement 6: Inline assembly influencing RTL */
        clobber_registers();
        
        /* Use register variable in computation */
        uint8_t low_byte = read_low_byte(reg_var);
        si.bytes[0] = low_byte;
        
        /* Conditional based on operations */
        if ((bf.field1 + bf.field2) > 20) {
            global_counter += element;
        } else {
            global_counter -= node_chain->array[i % 100];
        }
        
        /* Switch statement for additional complexity */
        switch (i % 4) {
            case 0:
                bf.field3 = global_counter & 0xFF;
                break;
            case 1:
                si.parts.high = (global_counter >> 16) & 0xFFFF;
                break;
            case 2:
                reg_var = (reg_var << 4) | (global_counter & 0xF);
                break;
            case 3:
                multi_array[i % 10][(i + 1) % 10][(i + 2) % 10] = 
                    node_chain->array[(i + 3) % 100] + reg_var;
                break;
        }
    }
    
    /* Compute final result to prevent dead code elimination */
    global_result = bf.field1 + bf.field2 + bf.field3 + bf.field4;
    global_result += si.full;
    global_result += reg_var;
    global_result += multi_array[0][0][0];
    global_result += node_chain->array[0];
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            free(multi_array[i][j]);
        }
        free(multi_array[i]);
    }
    free(multi_array);
    free(node_chain->array);
    free(node_chain);
    
    return global_result & 0xFF;
}
