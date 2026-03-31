/* test_resource_marking.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource marking logic, particularly targeting:
 * 1. ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * 2. SUBREG in SET_DEST  
 * 3. MEM_P with complex addressing in SET_DEST
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations alive */
static volatile int sink;

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - volatile forces actual stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int padding : 8;
    } bf = {0};
    
    /* Source variables in registers */
    register unsigned int r1 asm("r12") = 0xABCD;
    register unsigned int r2 asm("r13") = 0x1234;
    register unsigned int r3 asm("r14") = 0x5678;
    
    /* Complex bitfield assignments that may generate ZERO_EXTRACT */
    bf.field4 = (r1 & 0xF) + (r2 & 0x7);  /* Extract and combine bits */
    bf.field8 = __builtin_popcount(r1) & 0xFF;  /* Bit count extraction */
    bf.field12 = ((r1 ^ r2) | r3) & 0xFFF;  /* Bitwise ops with mask */
    
    /* Read back to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile int16_t vs16;
    volatile int8_t vs8;
    
    /* Register sources of different sizes */
    register int32_t r32 asm("r10") = 0x12345678;
    register int32_t r32_2 asm("r11") = 0x9ABCDEF0;
    
    /* Explicit narrowing casts - may generate SUBREG in SET_DEST */
    vs16 = (int16_t)(r32 + r32_2);  /* 32-bit to 16-bit with arithmetic */
    vs8 = (int8_t)((r32 >> 8) & 0xFF);  /* Extract byte */
    
    /* Implicit narrowing through arithmetic */
    int8_t c1 = 100, c2 = 50;
    volatile int8_t result;
    result = c1 + c2;  /* char addition with truncation */
    
    sink = vs16 + vs8 + result;
}

/* Test 3: Complex memory addressing to trigger MEM_P path */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int32_t arr[64][8] = {0};
    int32_t *restrict ptr = &arr[0][0];  /* restrict helps keep addressing */
    
    register int32_t rval asm("r9") = 0xDEADBEEF;
    
    /* Complex address calculations */
    for (int i = 0; i < 8; i++) {
        /* Non-linear index calculation */
        int idx = (i * 7 + 3) & 0x3F;  /* 0-63 */
        int subidx = (i ^ 0x3) & 0x7;  /* 0-7 */
        
        /* Store with complex addressing - may generate MEM with complex XEXP */
        arr[idx][subidx] = rval + i;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx * 8 + subidx) = rval - i;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        int idx = (i * 7 + 3) & 0x3F;
        int subidx = (i ^ 0x3) & 0x7;
        sum += arr[idx][subidx];
    }
    sink = sum;
}

/* Test 4: Combined patterns in struct context */
void test_combined_patterns(void) {
    /* Struct with bitfield and array */
    volatile struct {
        unsigned int flags : 6;
        int16_t data[16];
        unsigned int status : 10;
    } s = {0};
    
    /* Pointer to struct member */
    int16_t *dptr = s.data;
    
    register int32_t rsrc1 asm("r8") = 0x87654321;
    register int32_t rsrc2 asm("r7") = 0xFEDCBA98;
    
    /* Combined: bitfield assignment (ZERO_EXTRACT) */
    s.flags = (__builtin_parity(rsrc1) << 3) | (rsrc2 & 0x7);
    
    /* Combined: sub-word array store with complex index (SUBREG + MEM) */
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 5 + 1) & 0xF;
        
        /* Narrowing store to array element */
        s.data[idx] = (int16_t)((rsrc1 >> (i * 4)) + (rsrc2 & 0xFF));
        
        /* Alternative using pointer arithmetic */
        *(dptr + idx) = (int16_t)(rsrc1 - rsrc2);
    }
    
    /* Another bitfield assignment */
    s.status = (rsrc1 ^ rsrc2) & 0x3FF;
    
    /* Compute checksum */
    int sum = s.flags + s.status;
    for (int i = 0; i < 8; i++) {
        int idx = (i * 5 + 1) & 0xF;
        sum += s.data[idx];
    }
    sink = sum;
}

/* Test 5: Inline assembly to directly influence RTL generation */
void test_inline_asm(void) {
    int32_t array[32] = {0};
    int16_t short_var;
    int32_t int_var = 0x12345678;
    
    /* Memory output with complex addressing */
    asm volatile (
        "# Force complex MEM store\n"
        : "=m" (array[(1 << 3) + 4])  /* Complex address calculation */
        : 
        : "memory"
    );
    
    /* Sub-register store hint */
    asm volatile (
        "# Hint at SUBREG store\n"
        : "=r" (short_var)  /* Output to short - may become SUBREG */
        : "r" (int_var)     /* Input from int register */
    );
    
    sink = array[12] + short_var;
}

int main(void) {
    int total = 0;
    
    printf("Testing resource marking patterns...\n");
    
    /* Run all tests */
    test_bitfield_operations();
    total += sink;
    
    test_subword_operations();
    total += sink;
    
    test_complex_addressing();
    total += sink;
    
    test_combined_patterns();
    total += sink;
    
    test_inline_asm();
    total += sink;
    
    /* Final output to prevent optimization */
    printf("Checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
