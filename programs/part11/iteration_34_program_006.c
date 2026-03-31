/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_result = 0;
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0x89ABCDEF;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) - likely ZERO_EXTRACT */
        mask = (1U << 8) - 1;
        result = (source >> 4) & mask;
        test1_result ^= result;
        
        /* Extract bits 16-23 */
        result = (source >> 16) & 0xFF;
        test1_result ^= result;
        
        /* Extract bits 0-7 with variable width */
        int width = 8;
        mask = (1U << width) - 1;
        result = source & mask;
        test1_result ^= result;
        
        COMPILER_BARRIER();
    }
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
struct __attribute__((packed)) packed_bitfields {
    unsigned int header : 4;
    unsigned int data   : 20;
    unsigned int footer : 8;
};

volatile unsigned int test2_result = 0;
void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    s.header = 0xA;
    s.data = 0x12345;
    s.footer = 0xBC;
    
    volatile unsigned int temp = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        temp = s.data;
        test2_result += temp;
        
        /* Write to bitfields */
        s.data = (s.data + i) & 0xFFFFF;
        
        /* Complex bitfield operation */
        s.footer = (s.header + s.data) & 0xFF;
        
        /* Nested extraction */
        temp = s.header | (s.footer << 4);
        test2_result ^= temp;
        
        COMPILER_BARRIER();
    }
}

/* Test 3: Inline assembly for partial register store (STRICT_LOW_PART) */
volatile unsigned int test3_result = 0;
void test_inline_asm_partial_store(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned int new_val;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline asm that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (new_val)
            : "r" (reg), "i" (0xFF)  /* Only keep low byte */
        );
        test3_result += new_val;
        
        /* Another asm pattern */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3\n\t"
            : "=r" (reg)
            : "r" (reg), "i" (~0xFF), "r" (i & 0xFF)
            : "cc"
        );
        
        /* Store only low 16 bits */
        asm volatile (
            "uxth %0, %1\n\t"
            : "=r" (new_val)
            : "r" (reg + i)
        );
        test3_result ^= new_val;
        
        COMPILER_BARRIER();
    }
}

/* Test 4: Conditional merge operations (STRICT_LOW_PART) */
volatile unsigned int test4_result = 0;
void test_conditional_merge(void) {
    volatile unsigned int var = 0x87654321;
    volatile unsigned int cond = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte */
        if (cond) {
            /* Pattern: (var & ~mask) | (new & mask) */
            var = (var & ~0xFF) | ((i + 0xAB) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            var = (var & ~0xFFFF) | ((var + 0x1234) & 0xFFFF);
        }
        
        /* Switch based on bitfield extraction */
        unsigned int low_nibble = var & 0xF;
        switch (low_nibble) {
            case 0: var |= 0x100; break;
            case 1: var &= ~0x100; break;
            case 2: var ^= 0x200; break;
            default: var += 0x300; break;
        }
        
        test4_result += var;
        cond = !cond;
        COMPILER_BARRIER();
    }
}

/* Test 5: Mixed operations with memory references */
volatile unsigned int test5_result = 0;
void test_mixed_with_memory(void) {
    /* Array to force memory operations */
    volatile unsigned int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 0x01010101;
    }
    
    /* Packed struct in array */
    struct __attribute__((packed)) mixed_fields {
        unsigned short low : 6;
        unsigned short high : 10;
    };
    
    volatile struct mixed_fields mf[16];
    
    /* Register variable to encourage register allocation */
    register unsigned int reg_var asm ("r12") = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Bitfield extraction from array element */
        unsigned int elem = array[i % 256];
        unsigned int extracted = (elem >> 8) & 0x3F;  /* 6 bits */
        
        /* Store to packed struct bitfield */
        mf[i % 16].low = extracted & 0x3F;
        mf[i % 16].high = (extracted >> 6) & 0x3FF;
        
        /* Partial store through char pointer */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&array[i % 256];
        byte_ptr[1] = (i * 7) & 0xFF;  /* Modify only one byte */
        
        /* Complex operation mixing everything */
        reg_var = (reg_var & ~0xFF00) | ((mf[i % 16].high << 8) & 0xFF00);
        reg_var = (reg_var & ~0x3F) | (mf[i % 16].low & 0x3F);
        
        test5_result += reg_var + array[i % 256];
        COMPILER_BARRIER();
    }
}

/* Test 6: Nested loops with bitfield operations */
volatile unsigned int test6_result = 0;
void test_nested_loops_bitfields(void) {
    struct __attribute__((packed)) nested_bitfields {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 12;
        unsigned int d : 12;
    };
    
    volatile struct nested_bitfields nb = {0};
    volatile unsigned int temp_array[10];
    
    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 20; j += 2) {
            /* Write to bitfields */
            nb.a = (i + j) & 0x7;
            nb.b = (i * j) & 0x1F;
            
            /* Read and combine bitfields */
            unsigned int combined = (nb.c << 8) | nb.b;
            
            /* Conditional partial update */
            if (combined & 1) {
                nb.d = (nb.d & ~0xFF) | (i & 0xFF);
            }
            
            /* Extract and store */
            temp_array[j % 10] = nb.a | (nb.b << 3);
            
            test6_result += combined + nb.d;
        }
        COMPILER_BARRIER();
    }
}

/* Main driver function */
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
    
    printf("Starting resource pattern tests...\n");
    
    if (run_all || (test_mask & 0x01)) {
        test_bitfield_extract_volatile();
        printf("Test 1 complete: result = 0x%08X\n", test1_result);
    }
    
    if (run_all || (test_mask & 0x02)) {
        test_packed_struct_bitfields();
        printf("Test 2 complete: result = 0x%08X\n", test2_result);
    }
    
    if (run_all || (test_mask & 0x04)) {
        test_inline_asm_partial_store();
        printf("Test 3 complete: result = 0x%08X\n", test3_result);
    }
    
    if (run_all || (test_mask & 0x08)) {
        test_conditional_merge();
        printf("Test 4 complete: result = 0x%08X\n", test4_result);
    }
    
    if (run_all || (test_mask & 0x10)) {
        test_mixed_with_memory();
        printf("Test 5 complete: result = 0x%08X\n", test5_result);
    }
    
    if (run_all || (test_mask & 0x20)) {
        test_nested_loops_bitfields();
        printf("Test 6 complete: result = 0x%08X\n", test6_result);
    }
    
    /* Aggregate result to prevent dead code elimination */
    volatile unsigned int final_result = 
        test1_result + test2_result + test3_result + 
        test4_result + test5_result + test6_result;
    
    printf("Final aggregate result: 0x%08X\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
