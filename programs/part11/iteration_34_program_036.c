/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integers ========== */
volatile unsigned int test1_global = 0xDEADBEEF;
volatile unsigned int test1_result = 0;

void test_bitfield_extract_volatile(void) {
    volatile unsigned int val = test1_global;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions with different widths and positions */
        unsigned int bits_5_9 = (val >> 5) & ((1U << 5) - 1);    /* bits 5-9 */
        unsigned int bits_10_20 = (val >> 10) & ((1U << 11) - 1); /* bits 10-20 */
        unsigned int bits_0_4 = (val >> 0) & ((1U << 5) - 1);    /* bits 0-4 */
        
        /* Combine extractions with arithmetic */
        result = (bits_5_9 * bits_10_20) + bits_0_4;
        
        /* Use result to prevent dead code elimination */
        test1_result ^= result;
        
        /* Modify val slightly each iteration */
        val = (val * 1103515245U + 12345U) & 0xFFFFFFFFU;
    }
    
    COMPILER_BARRIER();
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct packed_bitfields {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 7;
    unsigned int d : 9;
} __attribute__((packed));

volatile struct packed_bitfields test2_struct = {0};
volatile unsigned int test2_result = 0;

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s;
    
    /* Initialize with pattern */
    s.a = 0x1F;  /* 5 bits max */
    s.b = 0x7FF; /* 11 bits max */
    s.c = 0x7F;  /* 7 bits max */
    s.d = 0x1FF; /* 9 bits max */
    
    /* Nested loop for scheduling */
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 2; ++j) {
            /* Complex bitfield operations */
            unsigned int temp = s.b + s.c;
            s.a = (temp & 0x1F);  /* Store only low 5 bits */
            
            /* Extract and combine bitfields */
            unsigned int combined = (s.d << 16) | (s.c << 9) | (s.b << 5) | s.a;
            
            /* Conditional update based on bitfield */
            if (s.a > 16) {
                s.d = (s.d + 1) & 0x1FF;
            }
            
            test2_result += combined;
        }
        
        /* Rotate bitfields */
        unsigned int tmp_a = s.a;
        s.a = s.b & 0x1F;
        s.b = (s.b >> 5) | ((s.c & 0x3F) << 6);
        s.c = (s.c >> 6) | ((s.d & 0x7) << 3);
        s.d = (s.d >> 3) | (tmp_a << 6);
    }
    
    test2_struct = s;
    COMPILER_BARRIER();
}

/* ========== Test 3: Inline assembly for partial register updates ========== */
volatile unsigned int test3_input = 0x12345678;
volatile unsigned int test3_output = 0;

void test_inline_asm_partial_store(void) {
    volatile unsigned int in = test3_input;
    volatile unsigned int out = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that hints at partial register updates */
        unsigned int temp;
        
        /* Update only low byte */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (in), "i" (0xFF)
        );
        
        /* Update only low 16 bits */
        unsigned int temp2;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp2)
            : "r" (in), "i" (0xFFFF)
        );
        
        /* Combine results */
        out = (out << 1) ^ (temp + temp2);
        
        /* Modify input */
        in = (in * 1664525U + 1013904223U) & 0xFFFFFFFFU;
    }
    
    test3_output = out;
    COMPILER_BARRIER();
}

/* ========== Test 4: Conditional merge operations ========== */
volatile unsigned int test4_var = 0x87654321;
volatile unsigned int test4_result = 0;

void test_conditional_merge(void) {
    volatile unsigned int var = test4_var;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 200; ++i) {
        volatile int condition = (i & 0x3F);  /* Varying condition */
        
        if (condition < 32) {
            /* Update only low byte conditionally */
            unsigned char new_byte = (i * 7) & 0xFF;
            var = (var & ~0xFF) | (new_byte & 0xFF);
        } else if (condition < 48) {
            /* Update only low 16 bits */
            unsigned short new_word = (i * 13) & 0xFFFF;
            var = (var & ~0xFFFF) | (new_word & 0xFFFF);
        } else {
            /* Update bits 16-23 */
            unsigned int middle_byte = (i * 19) & 0xFF;
            var = (var & ~0xFF0000) | ((middle_byte << 16) & 0xFF0000);
        }
        
        /* Switch based on low bits of var */
        switch (var & 0x7) {
            case 0: result += var; break;
            case 1: result += var >> 8; break;
            case 2: result += var >> 16; break;
            case 3: result += var >> 24; break;
            case 4: result ^= var; break;
            case 5: result ^= ~var; break;
            case 6: result = (result << 3) | (var & 0x7); break;
            case 7: result = (result >> 3) ^ var; break;
        }
    }
    
    test4_result = result;
    test4_var = var;
    COMPILER_BARRIER();
}

/* ========== Test 5: Mixed operations with memory and registers ========== */
volatile unsigned int test5_array[64];
volatile unsigned int test5_result = 0;

void test_mixed_operations(void) {
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        test5_array[i] = i * 0x01010101U;
    }
    
    /* Use register variables to encourage register allocation */
    register unsigned int reg1 asm ("r12") = 0;
    register unsigned int reg2 asm ("r13") = 0;
    
    volatile unsigned int* ptr = (volatile unsigned int*)test5_array;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory access with pointer arithmetic */
        unsigned int mem_val = ptr[i & 0x3F];
        
        /* Bitfield extraction from memory value */
        unsigned int extracted = (mem_val >> (i & 0x1F)) & ((1U << 8) - 1);
        
        /* Partial store back to memory through char pointer */
        volatile unsigned char* char_ptr = (volatile unsigned char*)&ptr[i & 0x3F];
        char_ptr[0] = extracted;
        char_ptr[1] = extracted ^ 0x55;
        
        /* Register operations with partial updates */
        if (i & 1) {
            reg1 = (reg1 & ~0xFF) | (extracted & 0xFF);
        } else {
            reg2 = (reg2 & ~0xFF00) | ((extracted << 8) & 0xFF00);
        }
        
        /* Complex expression that might generate ZERO_EXTRACT */
        unsigned int combined = ((reg1 & 0xF) << 28) | 
                               ((reg2 & 0xFFF) << 16) | 
                               (mem_val & 0xFFFF);
        
        test5_result += combined;
    }
    
    /* Use registers to prevent optimization */
    asm volatile ("" : : "r" (reg1), "r" (reg2));
    COMPILER_BARRIER();
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int final_result = 0;
    
    /* Parse command line to select tests */
    int run_all = (argc <= 1);
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    if (run_all || test_to_run == 1) {
        printf("Running test 1: Bitfield extraction from volatile integers\n");
        test_bitfield_extract_volatile();
        final_result ^= test1_result;
    }
    
    if (run_all || test_to_run == 2) {
        printf("Running test 2: Packed struct with bitfields\n");
        test_packed_struct_bitfields();
        final_result ^= test2_result;
    }
    
    if (run_all || test_to_run == 3) {
        printf("Running test 3: Inline assembly for partial register updates\n");
        test_inline_asm_partial_store();
        final_result ^= test3_output;
    }
    
    if (run_all || test_to_run == 4) {
        printf("Running test 4: Conditional merge operations\n");
        test_conditional_merge();
        final_result ^= test4_result;
    }
    
    if (run_all || test_to_run == 5) {
        printf("Running test 5: Mixed operations with memory and registers\n");
        test_mixed_operations();
        final_result ^= test5_result;
    }
    
    /* Print final result to prevent dead code elimination */
    printf("Final checksum: 0x%08X\n", final_result);
    
    /* Use result in return value to ensure it's not optimized away */
    return (final_result & 0xFF) == 0 ? 0 : 1;
}
