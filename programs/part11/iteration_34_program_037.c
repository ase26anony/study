/* test_resource.c - Test program for GCC RTL resource coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
volatile unsigned int test1_result = 0;

void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    /* Multiple bitfield extractions to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) - should generate ZERO_EXTRACT */
        mask = (1U << 8) - 1;
        result = (source >> 4) & mask;
        test1_result ^= result;  /* Combine results */
        
        /* Extract bits 16-23 */
        result = (source >> 16) & 0xFF;
        test1_result += result;
        
        /* Extract bits 0-3 */
        result = source & 0x0F;
        test1_result |= result;
        
        COMPILER_BARRIER();
    }
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

volatile unsigned int test2_result = 0;

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int temp = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield reads - should generate ZERO_EXTRACT */
        temp = s.field_b;  /* Read 11-bit field */
        test2_result += temp;
        
        temp = s.field_d;  /* Read 9-bit field */
        test2_result ^= temp;
        
        /* Bitfield assignment with extraction */
        s.field_a = (s.field_b + i) & 0x1F;  /* Complex pattern */
        temp = s.field_a;
        test2_result |= temp;
        
        /* Nested extraction */
        s.field_c = (s.field_d >> 3) & 0x7F;
        temp = s.field_c;
        test2_result += temp;
        
        COMPILER_BARRIER();
    }
}

/* ========== Test 3: Inline assembly for partial register updates ========== */
volatile unsigned int test3_result = 0;

void test_inline_asm_partial_store(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int new_val;
    
    for (int i = 0; i < 100; ++i) {
        /* Update only low byte using inline assembly */
        new_val = i & 0xFF;
        
        /* This may generate STRICT_LOW_PART in RTL */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (reg)
            : "r" (new_val), "i" (0xFF)
            : /* No clobbers */
        );
        
        test3_result += reg;
        
        /* Another partial update - low 16 bits */
        new_val = (i * 3) & 0xFFFF;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (reg)
            : "r" (new_val), "i" (0xFFFF)
        );
        
        test3_result ^= reg;
        
        COMPILER_BARRIER();
    }
}

/* ========== Test 4: Conditional merge operations ========== */
volatile unsigned int test4_result = 0;

void test_conditional_merge(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned int cond = 1;
    volatile unsigned char *byte_ptr;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (cond) {
            /* Update only low 8 bits, preserve high bits */
            var = (var & ~0xFF) | ((i * 7) & 0xFF);
        }
        test4_result += var;
        
        /* Update low 16 bits conditionally */
        if (i % 2) {
            var = (var & ~0xFFFF) | ((i * 13) & 0xFFFF);
        }
        test4_result ^= var;
        
        /* Pointer-based partial store */
        byte_ptr = (volatile unsigned char *)&var;
        byte_ptr[1] = (i * 5) & 0xFF;  /* Modify second byte */
        
        test4_result |= var;
        
        cond = !cond;  /* Toggle condition */
        COMPILER_BARRIER();
    }
}

/* ========== Test 5: Complex mixed operations with arrays ========== */
volatile unsigned int test5_result = 0;

void test_complex_mixed_operations(void) {
    /* Use register variables to encourage register allocation */
    register unsigned int r1 asm("r12") = 0;
    register unsigned int r2 asm("r13") = 0;
    
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 4; ++i) {
        struct_array[i].field_a = i;
        struct_array[i].field_b = i * 3;
        struct_array[i].field_c = i * 5;
        struct_array[i].field_d = i * 7;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from array element */
        unsigned int idx = i & 0xF;
        r1 = (array[idx] >> 8) & 0xFFF;  /* 12-bit extraction */
        
        /* Bitfield read from struct array */
        r2 = struct_array[idx & 0x3].field_b;
        
        /* Conditional merge */
        if (r1 > r2) {
            array[idx] = (array[idx] & ~0xFFFF) | (r1 & 0xFFFF);
        }
        
        /* Switch based on bitfield value - creates control flow */
        switch (struct_array[idx & 0x3].field_a & 0x7) {
            case 0:
                r1 = (r1 << 3) | 0x1;
                break;
            case 1:
                r1 = (r1 >> 2) & 0x3FF;
                break;
            case 2:
                r1 = r1 ^ r2;
                break;
            default:
                r1 = r1 + r2;
                break;
        }
        
        /* Pointer arithmetic and memory access */
        volatile unsigned int *ptr = &array[(i + 1) & 0xF];
        *ptr = (*ptr & ~0xFF00) | ((r1 << 8) & 0xFF00);
        
        test5_result += r1 + r2 + array[idx];
        COMPILER_BARRIER();
    }
}

/* ========== Test 6: Nested loops with bitfield operations ========== */
volatile unsigned int test6_result = 0;

void test_nested_loops_bitfields(void) {
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } __attribute__((packed)) data[8];
    
    /* Initialize */
    for (int i = 0; i < 8; ++i) {
        data[i].a = i & 0x7;
        data[i].b = (i * 2) & 0x1F;
        data[i].c = (i * 10) & 0x3FF;
        data[i].d = (i * 50) & 0x3FFF;
    }
    
    /* Nested loops to increase scheduling complexity */
    for (int outer = 0; outer < 10; ++outer) {
        volatile unsigned int acc = 0;
        
        for (int inner = 0; inner < 20; ++inner) {
            int idx = (outer + inner) & 0x7;
            
            /* Multiple bitfield extractions */
            unsigned int val1 = data[idx].c;  /* Should be ZERO_EXTRACT */
            unsigned int val2 = data[idx].d;
            
            /* Conditional partial update */
            if (val1 > val2) {
                /* Update only part of the bitfield */
                data[idx].b = (data[idx].b + 1) & 0x1F;
            }
            
            /* Extract and combine */
            acc += (val1 << 4) | data[idx].a;
            acc ^= (val2 >> 2) & 0xFFF;
            
            COMPILER_BARRIER();
        }
        
        test6_result += acc;
    }
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int run_all = 0;
    unsigned int test_mask = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
            test_mask = 0x3F;  /* Run all 6 tests */
        } else {
            test_mask = atoi(argv[1]);
        }
    } else {
        /* Default: run all tests */
        run_all = 1;
        test_mask = 0x3F;
    }
    
    printf("Running tests with mask: 0x%X\n", test_mask);
    
    /* Run selected tests */
    if (run_all || (test_mask & 0x01)) {
        printf("Running Test 1: Bitfield extract from volatile...\n");
        test_bitfield_extract_volatile();
    }
    
    if (run_all || (test_mask & 0x02)) {
        printf("Running Test 2: Packed struct bitfields...\n");
        test_packed_struct_bitfields();
    }
    
    if (run_all || (test_mask & 0x04)) {
        printf("Running Test 3: Inline assembly partial store...\n");
        test_inline_asm_partial_store();
    }
    
    if (run_all || (test_mask & 0x08)) {
        printf("Running Test 4: Conditional merge operations...\n");
        test_conditional_merge();
    }
    
    if (run_all || (test_mask & 0x10)) {
        printf("Running Test 5: Complex mixed operations...\n");
        test_complex_mixed_operations();
    }
    
    if (run_all || (test_mask & 0x20)) {
        printf("Running Test 6: Nested loops with bitfields...\n");
        test_nested_loops_bitfields();
    }
    
    /* Combine all results to prevent dead code elimination */
    volatile unsigned int final_result = 
        test1_result + test2_result + test3_result + 
        test4_result + test5_result + test6_result;
    
    printf("Final combined result: %u (0x%08X)\n", 
           final_result, final_result);
    
    return 0;
}
