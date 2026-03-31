/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;
    unsigned int mode : 3;
    unsigned int data : 12;
    unsigned int pad : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bf;
    unsigned int raw;
    unsigned short parts[2];
};

/* Force complex memory addressing with volatile */
#define ARRAY_SIZE 128
volatile int complex_array[ARRAY_SIZE];

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile int i, j, k;
    volatile long long multi_word;      /* For SUBREG generation on 32-bit targets */
    volatile double fp_multi_word;      /* Another multi-word type */
    volatile union mixed_access mixer;
    
    /* Initialize with volatile to prevent constant folding */
    mixer.raw = 0;
    multi_word = (argc > 1) ? (long long)atoi(argv[1]) : 0x123456789ABCDEF0LL;
    fp_multi_word = (argc > 2) ? atof(argv[2]) : 3.141592653589793;
    
    /* Initialize array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        complex_array[i] = i * 3 + 1;
    }
    
    /* Loop with volatile counter to force analysis in scheduling passes */
    volatile int loop_limit = (argc > 3) ? atoi(argv[3]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int result_accumulator = 0;
    
    for (volatile int iter = 0; iter < loop_limit; iter++) {
        /* ===== BIT-FIELD MANIPULATIONS ===== */
        /* These may generate ZERO_EXTRACT or STRICT_LOW_PART RTL */
        
        /* Direct bit-field assignment */
        mixer.bf.flag = iter & 1;
        mixer.bf.mode = (iter >> 1) & 0x7;
        mixer.bf.data = (iter * 7) & 0xFFF;
        
        /* Manual bit extraction using masks and shifts (alternative path to ZERO_EXTRACT) */
        unsigned int extracted_bits = (mixer.raw >> 4) & 0xFF;  /* Extract bits 4-11 */
        
        /* Combine bit-fields with arithmetic */
        mixer.bf.data = mixer.bf.data + ((mixer.bf.mode << 8) | extracted_bits);
        
        /* ===== MULTI-WORD OPERATIONS ===== */
        /* These should generate SUBREG RTL on 32-bit architectures */
        
        /* Operations on long long (two registers on 32-bit) */
        multi_word = multi_word + ((long long)iter << 32);
        multi_word = multi_word ^ 0xFFFFFFFF00000000LL;
        
        /* Access high and low parts separately */
        unsigned int low_part = (unsigned int)(multi_word & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)((multi_word >> 32) & 0xFFFFFFFF);
        
        /* Double precision operations (also multi-word) */
        fp_multi_word = fp_multi_word * 1.01 + (double)iter;
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        /* Create addressing modes that might produce MEM with complex XEXP */
        
        /* Non-linear array indexing */
        int idx = (mixer.bf.data * iter) % ARRAY_SIZE;
        int stride = 3;
        
        /* Complex addressing: array[base + index*stride + offset] */
        volatile int *addr = &complex_array[(idx * stride + 5) % ARRAY_SIZE];
        
        /* Read-modify-write with bit manipulation on memory */
        *addr = (*addr & 0xFFFF0000) | (mixer.bf.data & 0xFFFF);
        
        /* Another complex access pattern */
        int idx2 = ((iter * 17 + mixer.bf.mode * 13) & 0x7F);
        complex_array[idx2] = complex_array[idx2] ^ (low_part & 0xFF);
        
        /* ===== CONTROL FLOW BASED ON BIT-FIELD RESULTS ===== */
        /* Force data-dependent branching */
        
        if (mixer.bf.flag) {
            /* When flag is 1, do different operations */
            multi_word = multi_word >> 16;
            idx = (idx + high_part) % ARRAY_SIZE;
            complex_array[idx] = complex_array[idx] + iter;
        } else {
            /* When flag is 0 */
            multi_word = multi_word << 8;
            if ((mixer.bf.data & 0x800) != 0) {
                /* Nested condition based on bit-field */
                fp_multi_word = fp_multi_word / 1.5;
            }
        }
        
        /* Switch based on mode field (3 bits = 8 possible values) */
        switch (mixer.bf.mode & 0x7) {
            case 0:
                mixer.bf.data = mixer.bf.data + complex_array[iter % 16];
                break;
            case 1:
                multi_word = multi_word - 0x1000;
                break;
            case 2:
                fp_multi_word = fp_multi_word - 1.0;
                break;
            case 3:
                /* Bit manipulation on memory with complex address */
                int idx3 = (low_part ^ high_part) % ARRAY_SIZE;
                complex_array[idx3] = complex_array[idx3] | 0x80000000;
                break;
            case 4:
                /* Extract and manipulate specific bits */
                mixer.bf.data = (mixer.bf.data & 0xF0F) | ((extracted_bits & 0xF) << 4);
                break;
            default:
                mixer.bf.data = mixer.bf.data ^ 0x0FF0;
                break;
        }
        
        /* Accumulate results to prevent dead code elimination */
        result_accumulator += mixer.bf.data;
        result_accumulator += low_part & 0xFF;
        result_accumulator += complex_array[iter % 8];
        
        /* Force memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Additional bit-field manipulation after loop */
    struct bitfield_pack final_bf;
    final_bf.flag = (result_accumulator >> 0) & 1;
    final_bf.mode = (result_accumulator >> 1) & 0x7;
    final_bf.data = (result_accumulator >> 4) & 0xFFF;
    
    /* One more complex memory access */
    int final_idx = ((result_accumulator * 31) & 0x7F);
    complex_array[final_idx] = complex_array[final_idx] ^ final_bf.data;
    
    /* Use inline assembly to potentially generate specific RTL patterns */
    /* This asm operates on low 32 bits of multi_word, potentially creating STRICT_LOW_PART */
    unsigned int low_word;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $0x100, %0\n\t"
        : "=r" (low_word)
        : "r" ((unsigned int)(multi_word & 0xFFFFFFFF))
        : "cc"
    );
    
    result_accumulator += low_word;
    result_accumulator += (int)fp_multi_word;
    
    /* Print result to prevent optimization */
    printf("Result: %d (0x%08x)\n", result_accumulator, result_accumulator);
    
    return result_accumulator & 0xFF;
}
