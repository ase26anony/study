/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
volatile unsigned int test1_global = 0x12345678;
volatile unsigned int test1_result = 0;

void test1_bitfield_extract_volatile(void) {
    volatile unsigned int val = test1_global;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions that should generate ZERO_EXTRACT */
        unsigned int bits_5_9 = (val >> 5) & ((1U << 5) - 1);  /* bits 5-9 */
        unsigned int bits_10_19 = (val >> 10) & ((1U << 10) - 1); /* bits 10-19 */
        unsigned int bits_20_27 = (val >> 20) & ((1U << 8) - 1);  /* bits 20-27 */
        
        /* Combine results to prevent elimination */
        result ^= bits_5_9;
        result += bits_10_19;
        result |= bits_20_27;
        
        /* Modify source to create variation */
        val = (val * 1103515245U + 12345U) & 0x7FFFFFFF;
        
        COMPILER_BARRIER();
    }
    
    test1_result = result;
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct packed_bitfields {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 8;
    unsigned int d : 3;
    unsigned int e : 5;
} __attribute__((packed));

volatile struct packed_bitfields test2_struct = {0};
volatile unsigned int test2_results[4] = {0};

void test2_packed_struct_operations(void) {
    volatile struct packed_bitfields s;
    
    /* Initialize with pattern */
    s.a = 0x1F;
    s.b = 0x7FF;
    s.c = 0xFF;
    s.d = 0x7;
    s.e = 0x1F;
    
    /* Nested loops for scheduling */
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 10; ++inner) {
            /* Complex bitfield operations that should generate ZERO_EXTRACT */
            unsigned int temp;
            
            /* Read bitfields - should generate ZERO_EXTRACT for reads */
            temp = s.b;
            test2_results[0] ^= temp;
            
            temp = s.c;
            test2_results[1] += temp;
            
            /* Write with arithmetic - may generate ZERO_EXTRACT for RHS */
            s.a = (s.b + s.c) & 0x1F;  /* Mask to fit 5 bits */
            s.d = (s.e ^ s.a) & 0x7;   /* Mask to fit 3 bits */
            
            /* Rotate values */
            unsigned int save = s.e;
            s.e = s.d;
            s.d = s.c & 0x7;  /* Extract 3 bits from 8-bit field */
            s.c = s.b & 0xFF; /* Extract 8 bits from 11-bit field */
            s.b = s.a | ((save << 3) & 0x7FF);
            
            COMPILER_BARRIER();
        }
        
        /* Switch statement to create control flow */
        switch (s.a & 0x7) {  /* Use low 3 bits */
            case 0: test2_results[2] += 1; break;
            case 1: test2_results[2] += 2; break;
            case 2: test2_results[2] += 3; break;
            case 3: test2_results[2] += 4; break;
            case 4: test2_results[2] += 5; break;
            case 5: test2_results[2] += 6; break;
            case 6: test2_results[2] += 7; break;
            case 7: test2_results[2] += 8; break;
        }
    }
    
    test2_results[3] = s.a + s.b + s.c + s.d + s.e;
}

/* ========== Test 3: Conditional partial stores for STRICT_LOW_PART ========== */
volatile unsigned int test3_data = 0xDEADBEEF;
volatile unsigned int test3_results[2] = {0};

void test3_conditional_partial_stores(void) {
    volatile unsigned int var = test3_data;
    volatile unsigned int cond = 1;
    
    for (int i = 0; i < 50; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (cond & 1) {
            /* Update only low 8 bits */
            var = (var & ~0xFF) | ((var + i) & 0xFF);
            test3_results[0] += 1;
        }
        
        /* Conditional update of low 16 bits */
        if (cond & 2) {
            /* Update only low 16 bits */
            var = (var & ~0xFFFF) | ((var * 3) & 0xFFFF);
            test3_results[0] += 2;
        }
        
        /* Alternate between conditions */
        cond = (cond << 1) | (cond >> 31);
        
        /* Another pattern: merge operation */
        unsigned int new_low = (i * 17) & 0xFF;
        var = (var & ~0xFF) | new_low;
        
        /* Use array to create memory pressure */
        volatile unsigned int arr[4] = {var, var >> 8, var >> 16, var >> 24};
        test3_results[1] ^= arr[i & 3];
        
        COMPILER_BARRIER();
    }
    
    test3_data = var;
}

/* ========== Test 4: Inline assembly for partial register updates ========== */
volatile unsigned int test4_input = 0x12345678;
volatile unsigned int test4_output = 0;

void test4_inline_asm_partial_updates(void) {
    register unsigned int r1 asm("r8") = test4_input;
    register unsigned int r2 asm("r9") = 0;
    
    for (int i = 0; i < 25; ++i) {
        /* Inline assembly that hints at partial register updates */
        /* Note: Actual constraints may vary by architecture */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (r2)
            : "r" (r1), "i" (0x0000FFFF), "r" (i & 0xFFFF)
            : /* no clobbers */
        );
        
        /* Another asm pattern */
        unsigned int mask = 0xFF00FF00;
        unsigned int value = r2;
        asm volatile (
            "bic %0, %1, %2\n\t"  /* Clear bits */
            "and %0, %0, %3"      /* Mask bits */
            : "=r" (value)
            : "r" (value), "r" (mask), "r" (~mask)
            : /* no clobbers */
        );
        
        r1 = (r1 << 1) | (r1 >> 31);  /* Rotate */
        test4_output ^= r2 + value;
        
        COMPILER_BARRIER();
    }
}

/* ========== Test 5: Mixed operations with memory references ========== */
#define ARRAY_SIZE 16
volatile unsigned int test5_array[ARRAY_SIZE];
volatile unsigned int test5_sum = 0;

void test5_mixed_operations(void) {
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        test5_array[i] = i * 0x11111111;
    }
    
    volatile unsigned int accumulator = 0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 100; ++i) {
        /* Array access creates MEM_P references */
        unsigned int idx = (i * 7) % ARRAY_SIZE;
        volatile unsigned int* ptr = &test5_array[idx];
        
        /* Bitfield extraction from memory */
        unsigned int val = *ptr;
        unsigned int low_bits = (val >> 4) & 0xF;  /* ZERO_EXTRACT candidate */
        unsigned int high_bits = (val >> 24) & 0xF;
        
        /* Partial store back to memory - may generate STRICT_LOW_PART */
        *ptr = (*ptr & ~0xF) | ((low_bits + high_bits) & 0xF);
        
        /* Register variable for pressure */
        register unsigned int reg_var asm("r10") = accumulator;
        reg_var = (reg_var & ~0xFFF) | (val & 0xFFF);
        
        /* Another bitfield pattern */
        struct {
            unsigned int x : 12;
            unsigned int y : 12;
            unsigned int z : 8;
        } __attribute__((packed)) local_struct;
        
        local_struct.x = val & 0xFFF;
        local_struct.y = (val >> 12) & 0xFFF;
        local_struct.z = (val >> 24) & 0xFF;
        
        /* Update accumulator with mixed operations */
        accumulator ^= reg_var;
        accumulator += local_struct.x;
        accumulator |= local_struct.y << 4;
        accumulator &= ~(0xFF << 16);
        accumulator |= (local_struct.z & 0x3F) << 16;
        
        COMPILER_BARRIER();
    }
    
    test5_sum = accumulator;
    
    /* Final array checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        checksum ^= test5_array[i];
    }
    test5_sum ^= checksum;
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int run_all = 0;
    unsigned int test_mask = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
            test_mask = 0x1F;  /* Run all 5 tests */
        } else {
            test_mask = atoi(argv[1]) & 0x1F;
        }
    } else {
        /* Default: run all tests */
        run_all = 1;
        test_mask = 0x1F;
    }
    
    printf("Running tests with mask: 0x%02X\n", test_mask);
    
    /* Run selected tests */
    if (test_mask & 0x01) {
        printf("Running Test 1: Bitfield extraction from volatile integer\n");
        test1_bitfield_extract_volatile();
    }
    
    if (test_mask & 0x02) {
        printf("Running Test 2: Packed struct with bitfields\n");
        test2_packed_struct_operations();
    }
    
    if (test_mask & 0x04) {
        printf("Running Test 3: Conditional partial stores\n");
        test3_conditional_partial_stores();
    }
    
    if (test_mask & 0x08) {
        printf("Running Test 4: Inline assembly partial updates\n");
        test4_inline_asm_partial_updates();
    }
    
    if (test_mask & 0x10) {
        printf("Running Test 5: Mixed operations with memory references\n");
        test5_mixed_operations();
    }
    
    /* Aggregate results to prevent dead code elimination */
    volatile unsigned int final_result = 0;
    final_result += test1_result;
    final_result += test2_results[0] + test2_results[1] + test2_results[2] + test2_results[3];
    final_result += test3_results[0] + test3_results[1];
    final_result += test4_output;
    final_result += test5_sum;
    
    /* Use result in a way that can't be optimized away */
    if (final_result != 0xDEADBEEF) {  /* Arbitrary check */
        printf("Final aggregate result: 0x%08X\n", final_result);
    } else {
        printf("Unexpected result value\n");
    }
    
    return 0;
}
