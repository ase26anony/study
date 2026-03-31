/* test_resource.c - Generate RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions - may generate ZERO_EXTRACT */
        unsigned int bits_4_11 = (source >> 4) & 0xFF;      /* 8 bits starting at bit 4 */
        unsigned int bits_12_19 = (source >> 12) & 0xFF;    /* 8 bits starting at bit 12 */
        unsigned int bits_20_27 = (source >> 20) & 0xFF;    /* 8 bits starting at bit 20 */
        
        /* Combine and store to volatile to prevent elimination */
        result = bits_4_11 + bits_12_19 + bits_20_27;
        
        /* Modify source to create variation */
        source = (source << 1) | (source >> 31);
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Complex bitfield operations that may generate ZERO_EXTRACT */
        unsigned int temp = s.field_b;
        s.field_a = (temp + i) & 0x1F;          /* Only keep 5 bits */
        
        /* Cross-field operations */
        s.field_c = (s.field_d >> 2) & 0x7F;    /* Extract 7 bits */
        s.field_d = (s.field_a << 4) | (s.field_c & 0xF);
        
        /* Store results */
        results[i % 4] = s.field_a + s.field_b + s.field_c + s.field_d;
    }
    
    COMPILER_BARRIER();
    printf("Test2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Update only low 8 bits */
            data = (data & ~0xFF) | ((data + i) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            data = (data & ~0xFFFF) | ((data * 3) & 0xFFFF);
        }
        
        /* Toggle flag */
        flag = !flag;
    }
    
    COMPILER_BARRIER();
    printf("Test3 data: 0x%08x\n", data);
}

/* Test 4: STRICT_LOW_PART via inline assembly hints */
void test_strict_low_part_asm(void) {
    register unsigned int reg_var asm("r12") = 0x87654321;
    volatile unsigned int output;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that operates on partial register */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (output)
            : "r" (reg_var), "i" (0x0000FFFF), "r" (i << 16)
            : /* no clobbers */
        );
        
        /* Update register variable */
        reg_var = output ^ 0x55555555;
        
        /* Another partial operation */
        asm volatile (
            "bic %0, %1, %2"
            : "=r" (output)
            : "r" (reg_var), "i" (0xFF000000)
        );
        
        reg_var = output | (i << 24);
    }
    
    COMPILER_BARRIER();
    printf("Test4 output: 0x%08x\n", output);
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields structs[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        structs[i].field_a = i * 3;
        structs[i].field_b = i * 7;
        structs[i].field_c = i * 5;
        structs[i].field_d = i * 9;
    }
    
    volatile unsigned int sum = 0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from struct array */
        unsigned int idx = i & 3;
        unsigned int extracted = structs[idx].field_b;
        
        /* Conditional partial store to array element */
        if (extracted & 1) {
            /* Update only low 16 bits of array element */
            array[idx] = (array[idx] & ~0xFFFF) | (extracted & 0xFFFF);
        }
        
        /* Bitfield manipulation */
        structs[idx].field_a = (array[idx] >> 8) & 0x1F;
        
        /* Switch based on bitfield value */
        switch (structs[idx].field_a & 0x7) {
            case 0: sum += array[idx] & 0xFF; break;
            case 1: sum += (array[idx] >> 8) & 0xFF; break;
            case 2: sum += (array[idx] >> 16) & 0xFF; break;
            case 3: sum += (array[idx] >> 24) & 0xFF; break;
            default: sum += extracted; break;
        }
        
        /* Pointer arithmetic with partial store */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&array[idx];
        byte_ptr[1] = (extracted >> 4) & 0xFF;  /* May generate partial store RTL */
    }
    
    COMPILER_BARRIER();
    printf("Test5 sum: %u\n", sum);
}

/* Test 6: Nested loops with bitfield operations */
void test_nested_loops(void) {
    volatile unsigned int matrix[4][4];
    volatile unsigned int results[4] = {0};
    
    /* Initialize matrix */
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[i][j] = (i << 8) | j;
        }
    }
    
    /* Nested loops with bitfield operations */
    for (int outer = 0; outer < 10; ++outer) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                /* Extract various bit ranges */
                unsigned int low_bits = matrix[i][j] & 0xF;
                unsigned int mid_bits = (matrix[i][j] >> 4) & 0xF;
                unsigned int high_bits = (matrix[i][j] >> 8) & 0xF;
                
                /* Conditional partial update */
                if ((i + j) % 2 == 0) {
                    /* Update only specific bits */
                    matrix[i][j] = (matrix[i][j] & ~0xF0) | ((low_bits + mid_bits) << 4);
                }
                
                /* Accumulate results using bitfield extraction */
                results[i] += (low_bits << 16) | (mid_bits << 8) | high_bits;
            }
        }
    }
    
    COMPILER_BARRIER();
    printf("Test6 results: ");
    for (int i = 0; i < 4; ++i) {
        printf("%08x ", results[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int run_all = 0;
    int test_num = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_num = atoi(argv[1]);
        }
    } else {
        run_all = 1;  /* Default: run all tests */
    }
    
    /* Run selected tests */
    if (run_all || test_num == 1) test_zero_extract_volatile();
    if (run_all || test_num == 2) test_zero_extract_struct();
    if (run_all || test_num == 3) test_strict_low_part_conditional();
    if (run_all || test_num == 4) test_strict_low_part_asm();
    if (run_all || test_num == 5) test_mixed_operations();
    if (run_all || test_num == 6) test_nested_loops();
    
    /* Final compiler barrier */
    COMPILER_BARRIER();
    
    return 0;
}
