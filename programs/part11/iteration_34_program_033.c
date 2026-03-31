/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0x12345678;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int mask1 = (1U << 8) - 1;
        result = (source >> 4) & mask1;
        
        /* Extract bits 16-23 with different width */
        unsigned int mask2 = (1U << 8) - 1;
        result += (source >> 16) & mask2;
        
        /* Extract bits 0-7 */
        unsigned int mask3 = 0xFF;
        result ^= (source >> 0) & mask3;
        
        /* Vary the source to prevent constant propagation */
        source += 0x111;
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) BitfieldStruct {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 7;
    unsigned int d : 9;
};

void test_packed_struct_bitfields(void) {
    volatile struct BitfieldStruct s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.a = 0x1F;
    s.b = 0x7FF;
    s.c = 0x7F;
    s.d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        results[0] = s.a;
        results[1] = s.b;
        results[2] = s.c;
        results[3] = s.d;
        
        /* Write with arithmetic - complex pattern */
        s.a = (s.b + i) & 0x1F;
        s.c = (s.d >> 2) & 0x7F;
        
        /* Cross-field operation */
        s.b = (s.a << 6) | (s.c & 0x3F);
    }
    
    COMPILER_BARRIER();
    printf("Test2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: Mixed bitfield and memory operations */
void test_mixed_bitfield_memory(void) {
    volatile unsigned int array[16];
    volatile unsigned int temp = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = 0x87654321 ^ i;
    }
    
    struct __attribute__((packed)) {
        unsigned int low : 12;
        unsigned int high : 20;
    } bitfield;
    
    for (int i = 0; i < 100; ++i) {
        /* Extract from memory, then extract bitfield */
        unsigned int val = array[i & 0xF];
        
        /* Simulate bitfield extraction */
        bitfield.low = val & 0xFFF;
        bitfield.high = (val >> 12) & 0xFFFFF;
        
        /* Recombine with different pattern */
        temp = (bitfield.high << 8) | (bitfield.low & 0xFF);
        
        /* Store back to memory */
        array[i & 0xF] = temp ^ i;
    }
    
    COMPILER_BARRIER();
    printf("Test3 array[0]: %u\n", array[0]);
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Update only low 8 bits */
            reg = (reg & ~0xFF) | ((i + 0xAB) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i & 1) {
            reg = (reg & ~0xFFFF) | ((reg + 0x1111) & 0xFFFF);
        }
        
        /* Update low 4 bits based on condition */
        unsigned int mask = 0xF;
        reg = (reg & ~mask) | ((reg >> 4) & mask);
        
        flag ^= 1; /* Toggle flag */
    }
    
    COMPILER_BARRIER();
    printf("Test4 reg: 0x%08x\n", reg);
}

/* Test 5: Inline assembly for partial register updates */
void test_asm_partial_update(void) {
    register unsigned int r1 asm("r8") = 0xDEADBEEF;
    register unsigned int r2 asm("r9") = 0xCAFEBABE;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline asm that operates on partial registers */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (result)
            : "r" (r1), "i" (0x0000FFFF), "r" (r2 & 0xFFFF0000)
            : /* no clobber */
        );
        
        /* Another pattern that might generate STRICT_LOW_PART */
        asm volatile (
            "bic %0, %1, %2\n\t"
            "and %3, %4, %5\n\t"
            "orr %0, %0, %3"
            : "=r" (r1), "=r" (result)
            : "r" (r1), "i" (0xFF00FF00), "r" (r2), "i" (0x00FF00FF)
            : /* no clobber */
        );
        
        /* Rotate values */
        unsigned int temp = r1;
        r1 = r2;
        r2 = temp ^ i;
    }
    
    COMPILER_BARRIER();
    printf("Test5 result: 0x%08x, r1: 0x%08x\n", result, r1);
}

/* Test 6: Complex pattern with switch and bitfields */
void test_complex_pattern_with_switch(void) {
    volatile unsigned int state = 0;
    volatile unsigned int output = 0;
    
    struct __attribute__((packed)) {
        unsigned int opcode : 4;
        unsigned int operand : 12;
        unsigned int mode : 2;
    } instruction;
    
    for (int i = 0; i < 200; ++i) {
        /* Create instruction from loop counter */
        instruction.opcode = i & 0xF;
        instruction.operand = (i >> 4) & 0xFFF;
        instruction.mode = (i >> 16) & 0x3;
        
        /* Switch on bitfield-extracted value */
        switch (instruction.opcode) {
            case 0: output = instruction.operand & 0xFF; break;
            case 1: output = (instruction.operand >> 4) & 0xF; break;
            case 2: output = instruction.operand | (instruction.mode << 12); break;
            case 3: output = (instruction.operand << 2) & 0x3FF; break;
            default: output = instruction.operand ^ instruction.opcode; break;
        }
        
        /* Conditional partial update based on mode */
        if (instruction.mode == 0) {
            state = (state & ~0x3FF) | (output & 0x3FF);
        } else if (instruction.mode == 1) {
            state = (state & ~0xFF00) | ((output << 8) & 0xFF00);
        }
        
        /* Memory operation to trigger MEM_P path elsewhere */
        volatile unsigned int *ptr = &state;
        *ptr ^= 0x55555555;
    }
    
    COMPILER_BARRIER();
    printf("Test6 state: 0x%08x, output: %u\n", state, output);
}

/* Main driver */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Determine which test to run based on arguments */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run specific test or all tests */
    switch (test_to_run) {
        case 1:
            test_bitfield_extract_volatile();
            break;
        case 2:
            test_packed_struct_bitfields();
            break;
        case 3:
            test_mixed_bitfield_memory();
            break;
        case 4:
            test_strict_low_part();
            break;
        case 5:
            test_asm_partial_update();
            break;
        case 6:
            test_complex_pattern_with_switch();
            break;
        default:
            /* Run all tests */
            test_bitfield_extract_volatile();
            test_packed_struct_bitfields();
            test_mixed_bitfield_memory();
            test_strict_low_part();
            test_asm_partial_update();
            test_complex_pattern_with_switch();
            break;
    }
    
    return 0;
}
