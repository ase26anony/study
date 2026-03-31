/* test_resource.c - Coverage test for ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
volatile unsigned int test1_global = 0xDEADBEEF;
volatile unsigned int test1_result = 0;

void test_bitfield_extract_volatile(void) {
    volatile unsigned int val = test1_global;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions - should generate ZERO_EXTRACT */
        unsigned int bits_5_9 = (val >> 5) & ((1U << 5) - 1);  /* bits 5-9 */
        unsigned int bits_10_19 = (val >> 10) & ((1U << 10) - 1); /* bits 10-19 */
        unsigned int bits_20_31 = (val >> 20) & ((1U << 12) - 1); /* bits 20-31 */
        
        /* Combine results with arithmetic */
        result = (bits_5_9 * bits_10_19) | bits_20_31;
        
        /* Modify source to create variation */
        val ^= (i << 3);
        
        COMPILER_BARRIER();
    }
    
    test1_result = result;
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct __attribute__((packed)) packed_bitfields {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 7;
    unsigned int d : 9;
};

volatile struct packed_bitfields test2_struct = {0};
volatile unsigned int test2_results[4] = {0};

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s;
    
    /* Initialize with pattern */
    s.a = 0x1F;
    s.b = 0x7FF;
    s.c = 0x7F;
    s.d = 0x1FF;
    
    /* Nested loops for scheduling */
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 10; ++inner) {
            /* Complex bitfield operations - should generate ZERO_EXTRACT */
            unsigned int temp = s.b;
            s.a = (temp + inner) & 0x1F;  /* Write to bitfield */
            
            temp = s.c;
            s.b = (temp * outer) & 0x7FF;
            
            /* Read multiple bitfields in expression */
            test2_results[0] = s.a;
            test2_results[1] = s.b;
            test2_results[2] = s.c;
            test2_results[3] = s.d;
            
            /* Conditional update based on bitfield */
            if (s.a & 0x10) {
                s.d = (s.d >> 1) | 0x100;
            }
            
            COMPILER_BARRIER();
        }
    }
}

/* ========== Test 3: Inline assembly for partial register updates ========== */
volatile unsigned int test3_input = 0x12345678;
volatile unsigned int test3_output = 0;

void test_inline_asm_partial_store(void) {
    volatile unsigned int in = test3_input;
    volatile unsigned int out = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that might generate STRICT_LOW_PART */
        /* Update only low 8 bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(out)
            : "r"(in), "i"(0xFF)
        );
        
        /* Update only low 16 bits */
        unsigned int temp;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(temp)
            : "r"(in), "i"(0xFFFF)
        );
        out |= temp;
        
        /* Rotate and mask */
        in = (in << 1) | (in >> 31);
        
        COMPILER_BARRIER();
    }
    
    test3_output = out;
}

/* ========== Test 4: Conditional merge operations ========== */
volatile unsigned int test4_var = 0x87654321;
volatile unsigned int test4_results[100];

void test_conditional_merge(void) {
    volatile unsigned int var = test4_var;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        unsigned char new_byte = i & 0xFF;
        
        if (i % 3 == 0) {
            /* Update only low byte */
            var = (var & ~0xFF) | (new_byte & 0xFF);
        } else if (i % 3 == 1) {
            /* Update only low word */
            unsigned short new_word = i & 0xFFFF;
            var = (var & ~0xFFFF) | (new_word & 0xFFFF);
        } else {
            /* Update bits 8-15 */
            unsigned char mid_byte = (i >> 4) & 0xFF;
            var = (var & ~0xFF00) | ((mid_byte << 8) & 0xFF00);
        }
        
        test4_results[i] = var;
        
        /* Switch based on low bits for control flow */
        switch (var & 0x7) {
            case 0: var ^= 0xAAAAAAAA; break;
            case 1: var += 0x11111111; break;
            case 2: var = (var >> 4) | (var << 28); break;
            default: var = ~var; break;
        }
        
        COMPILER_BARRIER();
    }
}

/* ========== Test 5: Pointer-based partial updates ========== */
volatile unsigned int test5_buffer[256];
volatile unsigned int test5_sum = 0;

void test_pointer_partial_updates(void) {
    /* Initialize buffer */
    for (int i = 0; i < 256; ++i) {
        test5_buffer[i] = i * 0x01010101;
    }
    
    volatile unsigned int *ptr = test5_buffer;
    volatile unsigned char *byte_ptr;
    
    for (int i = 0; i < 1000; ++i) {
        /* Update through char pointer - partial store */
        byte_ptr = (volatile unsigned char *)ptr;
        byte_ptr[1] = i & 0xFF;  /* Update only byte 1 */
        
        /* Update through short pointer */
        volatile unsigned short *short_ptr = (volatile unsigned short *)ptr;
        short_ptr[1] = (i * 3) & 0xFFFF;  /* Update only short 1 */
        
        /* Bitfield extraction from memory */
        unsigned int val = *ptr;
        unsigned int field = (val >> 8) & 0x3F;  /* Extract bits 8-13 */
        
        /* Update based on extracted field */
        *ptr = (*ptr & ~0x3F00) | ((field << 8) & 0x3F00);
        
        /* Move pointer with wrap-around */
        ptr = &test5_buffer[(ptr - test5_buffer + 1) % 256];
        
        COMPILER_BARRIER();
    }
    
    /* Compute sum to prevent elimination */
    for (int i = 0; i < 256; ++i) {
        test5_sum += test5_buffer[i];
    }
}

/* ========== Test 6: Mixed operations with register variables ========== */
volatile unsigned int test6_result = 0;

void test_mixed_operations(void) {
    register unsigned int r1 asm("r12") = 0x55555555;
    register unsigned int r2 asm("r13") = 0xAAAAAAAA;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from register variable */
        unsigned int low_bits = (r1 >> 3) & 0x1F;
        unsigned int high_bits = (r2 >> 16) & 0x7FF;
        
        /* Conditional partial update */
        if (low_bits > high_bits) {
            r1 = (r1 & ~0xFF) | ((low_bits + i) & 0xFF);
        } else {
            r2 = (r2 & ~0xFFFF) | ((high_bits * i) & 0xFFFF);
        }
        
        /* Complex expression with memory */
        volatile unsigned int *mem = &test6_result;
        *mem = r1 ^ r2;
        
        /* Rotate registers */
        unsigned int temp = r1;
        r1 = r2;
        r2 = temp ^ i;
        
        result += *mem;
        
        COMPILER_BARRIER();
    }
    
    test6_result = result;
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int run_all = 0;
    unsigned int test_mask = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_mask = atoi(argv[1]);
        }
    } else {
        run_all = 1;  /* Default: run all tests */
    }
    
    printf("Starting resource.cc coverage tests...\n");
    
    /* Run selected tests */
    if (run_all || (test_mask & 0x01)) {
        printf("Running test 1: Bitfield extraction from volatile integer\n");
        test_bitfield_extract_volatile();
    }
    
    if (run_all || (test_mask & 0x02)) {
        printf("Running test 2: Packed struct with bitfields\n");
        test_packed_struct_bitfields();
    }
    
    if (run_all || (test_mask & 0x04)) {
        printf("Running test 3: Inline assembly for partial store\n");
        test_inline_asm_partial_store();
    }
    
    if (run_all || (test_mask & 0x08)) {
        printf("Running test 4: Conditional merge operations\n");
        test_conditional_merge();
    }
    
    if (run_all || (test_mask & 0x10)) {
        printf("Running test 5: Pointer-based partial updates\n");
        test_pointer_partial_updates();
    }
    
    if (run_all || (test_mask & 0x20)) {
        printf("Running test 6: Mixed operations with register variables\n");
        test_mixed_operations();
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile unsigned int final_result = 
        test1_result + 
        test2_results[0] + test2_results[1] + test2_results[2] + test2_results[3] +
        test3_output + 
        test4_results[0] + test4_results[99] +
        test5_sum + 
        test6_result;
    
    printf("Tests completed. Final aggregate: %u\n", (unsigned int)final_result);
    
    return 0;
}
