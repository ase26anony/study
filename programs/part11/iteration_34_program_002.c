#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer - should generate ZERO_EXTRACT */
volatile unsigned int test1_bitfield_extract(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns to increase chances */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & ((1U << 8) - 1);
        result += extracted;
        
        /* Extract bits 16-23 */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Variable width extraction */
        int width = (i % 8) + 1;
        extracted = (source >> (i % 24)) & ((1U << width) - 1);
        result |= extracted;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 2: Packed struct with bitfields - classic ZERO_EXTRACT source */
volatile unsigned int test2_packed_struct(void) {
    struct __attribute__((packed)) {
        unsigned int a : 5;
        unsigned int b : 11;
        unsigned int c : 7;
        unsigned int d : 9;
    } s;
    
    volatile unsigned int *ptr = (volatile unsigned int*)&s;
    *ptr = 0x12345678;
    
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        result += s.b;
        result ^= s.c;
        
        /* Write to bitfields */
        s.a = (s.b + i) & 0x1F;  /* 5-bit mask */
        s.d = s.c ^ 0xFF;
        
        /* Complex bitfield expression */
        s.b = (s.a << 3) | (s.d & 0x7);
        
        COMPILER_BARRIER();
    }
    
    return result + s.a + s.b + s.c + s.d;
}

/* Test 3: Mixed bitfield operations with memory - targets both ZERO_EXTRACT and MEM_P */
volatile unsigned int test3_mixed_bitfield_memory(void) {
    /* Array with bitfield structs */
    struct __attribute__((packed)) {
        unsigned short low : 4;
        unsigned short mid : 8;
        unsigned short high : 4;
    } arr[16];
    
    volatile unsigned int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        arr[i].low = i & 0xF;
        arr[i].mid = (i * 7) & 0xFF;
        arr[i].high = (i >> 2) & 0xF;
    }
    
    /* Process array - should generate MEM references and ZERO_EXTRACT */
    for (int iter = 0; iter < 20; ++iter) {
        for (int i = 0; i < 16; ++i) {
            /* Bitfield reads (ZERO_EXTRACT) */
            unsigned int val = arr[i].mid;
            
            /* Conditional update */
            if (val & 1) {
                arr[i].low = (arr[i].high + 3) & 0xF;
            } else {
                arr[i].high = (arr[i].low ^ arr[i].mid) & 0xF;
            }
            
            result += arr[i].low + arr[i].mid + arr[i].high;
        }
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
volatile unsigned int test4_strict_low_part(void) {
    volatile unsigned int data = 0x87654321;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional merge operation - may generate STRICT_LOW_PART */
        if (data & 0x100) {
            /* Update only low byte */
            data = (data & ~0xFF) | ((data + i) & 0xFF);
        } else {
            /* Update only low 16 bits */
            data = (data & ~0xFFFF) | ((data ^ i) & 0xFFFF);
        }
        
        /* Another pattern for partial update */
        unsigned char low_byte = (data >> 8) & 0xFF;
        data = (data & ~0xFF00) | (low_byte << 8);
        
        result += data;
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 5: Inline assembly attempting to generate STRICT_LOW_PART */
volatile unsigned int test5_asm_partial_store(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 50; ++i) {
        unsigned int temp = value;
        
        /* Inline assembly that operates on partial register */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (temp)
            : "r" (temp), "i" (0x0000FFFF), "r" (i & 0xFFFF)
            : /* no clobber */
        );
        
        /* Another asm pattern */
        unsigned int low_part;
        asm volatile (
            "ubfx %0, %1, #0, #16"
            : "=r" (low_part)
            : "r" (temp)
            : /* no clobber */
        );
        
        value = temp ^ low_part;
        result += value;
        
        COMPILER_BARRIER();
    }
    
    return result;
}

/* Test 6: Complex pattern mixing bitfields and partial stores */
volatile unsigned int test6_complex_mix(void) {
    /* Register variables to encourage register allocation pressure */
    register unsigned int r1 asm ("r8") = 0x11111111;
    register unsigned int r2 asm ("r9") = 0x22222222;
    volatile unsigned int *mem = (volatile unsigned int*)malloc(64 * sizeof(unsigned int));
    
    if (!mem) return 0;
    
    /* Initialize memory */
    for (int i = 0; i < 64; i++) {
        mem[i] = i * 0x01010101;
    }
    
    volatile unsigned int result = 0;
    
    /* Complex loop with multiple patterns */
    for (int i = 0; i < 30; ++i) {
        /* Bitfield-like extraction from memory */
        unsigned int val = mem[i % 64];
        unsigned int field1 = (val >> 8) & 0xF;   /* 4 bits */
        unsigned int field2 = (val >> 16) & 0xFF; /* 8 bits */
        
        /* Partial store back */
        mem[i % 64] = (mem[i % 64] & ~0xFF00) | ((field1 + field2) << 8);
        
        /* Register operations with partial updates */
        r1 = (r1 & ~0xFFFF) | ((r1 + r2) & 0xFFFF);
        r2 = (r2 & ~0xFF0000) | ((r1 ^ i) << 16);
        
        /* Switch based on bitfield value */
        switch (field1 & 0x7) {
            case 0: result += r1; break;
            case 1: result += r2; break;
            case 2: result += val; break;
            case 3: result += field2; break;
            case 4: result ^= r1; break;
            case 5: result ^= r2; break;
            default: result |= 1; break;
        }
        
        COMPILER_BARRIER();
    }
    
    free((void*)mem);
    return result + r1 + r2;
}

/* Main driver */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line or all by default */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1")) {
        final_result += test1_bitfield_extract();
    }
    
    if (run_all || strstr(argv[1], "2")) {
        final_result += test2_packed_struct();
    }
    
    if (run_all || strstr(argv[1], "3")) {
        final_result += test3_mixed_bitfield_memory();
    }
    
    if (run_all || strstr(argv[1], "4")) {
        final_result += test4_strict_low_part();
    }
    
    if (run_all || strstr(argv[1], "5")) {
        final_result += test5_asm_partial_store();
    }
    
    if (run_all || strstr(argv[1], "6")) {
        final_result += test6_complex_mix();
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
