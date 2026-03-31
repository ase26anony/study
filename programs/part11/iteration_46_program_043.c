/* test_resource_marking.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT and STRICT_LOW_PART in SET_DEST
 * - SUBREG in SET_DEST  
 * - MEM_P(x) with complex addressing
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations */
static volatile int sink;

/* Test 1: Bitfield operations to generate ZERO_EXTRACT/STRICT_LOW_PART */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field4 : 4;    /* 4-bit field */
        unsigned int field8 : 8;    /* 8-bit field */
        unsigned int field12 : 12;  /* 12-bit field */
        unsigned int dummy : 8;     /* Padding */
    } bf = {0};
    
    /* Source values with bitwise operations */
    unsigned int a = 0xABCD1234;
    unsigned int b = 0xDEADBEEF;
    unsigned int c = 0xCAFEBABE;
    
    /* Complex assignments to bitfields - may generate ZERO_EXTRACT */
    bf.field4 = (a & 0xF) + (b & 0xF);           /* 4-bit extract and store */
    bf.field8 = ((a >> 8) & 0xFF) ^ ((b >> 16) & 0xFF); /* 8-bit extract */
    bf.field12 = (c & 0xFFF) | ((a >> 4) & 0xFFF);      /* 12-bit extract */
    
    /* Use bitfield values to prevent elimination */
    sink = bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subreg_operations(void) {
    /* Volatile sub-word destinations */
    volatile int8_t v8;
    volatile int16_t v16;
    volatile int32_t v32 = 0x12345678;
    
    /* Register variables - encourage SUBREG patterns */
    register int32_t r32 asm("r12") = 0x9ABCDEF0;
    register int64_t r64 asm("r13") = 0x1122334455667788ULL;
    
    /* Narrowing assignments - may generate SUBREG in SET_DEST */
    v8 = (int8_t)(v32 + r32);          /* 32-bit to 8-bit with SUBREG */
    v16 = (int16_t)(r64 >> 16);        /* 64-bit to 16-bit with SUBREG */
    
    /* Arithmetic with implicit narrowing */
    char c1 = 100, c2 = 50;
    volatile char vc;
    vc = c1 + c2;                      /* May generate SUBREG for overflow truncation */
    
    /* Use values to prevent elimination */
    sink = v8 + v16 + vc;
}

/* Test 3: Complex memory addressing to trigger MEM_P(x) path */
void test_complex_addressing(void) {
    /* Multi-dimensional array with non-trivial indexing */
    int array[64][8] __attribute__((aligned(64)));
    int * restrict ptr = &array[0][0];
    
    /* Complex index calculations */
    for (int i = 0; i < 16; i++) {
        /* Non-linear index: i*3 + 7 encourages address computation */
        int idx = i * 3 + 7;
        
        /* Store with complex addressing - address stays live */
        array[idx % 64][(i * 5) % 8] = i * 100 + idx;
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx + (i * 2)) = i * 200;
    }
    
    /* Struct with array member */
    struct {
        int header;
        int data[32];
        int footer;
    } s __attribute__((aligned(64)));
    
    /* Access through pointer with offset */
    int *data_ptr = s.data;
    for (int i = 0; i < 16; i++) {
        /* Complex addressing: base + scaled index + constant */
        data_ptr[i * 2 + 4] = i * 300;
    }
    
    /* Use memory values */
    sink = array[7][3] + s.data[12];
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int config : 6;
        unsigned int status : 10;
        int16_t samples[16];
        unsigned int tail : 4;
    } device __attribute__((aligned(32)));
    
    /* Register source */
    register int32_t source asm("r14") = 0x87654321;
    
    /* Combined assignment: bitfield + sub-word array element */
    device.config = (source >> 8) & 0x3F;          /* Potential ZERO_EXTRACT */
    
    /* Complex index calculation */
    int idx = (source & 0xF) * 3 + 5;
    device.samples[idx % 16] = (int16_t)source;    /* Potential SUBREG + complex MEM */
    
    /* Another bitfield assignment */
    device.status = ((source >> 16) & 0x3FF) | 0x100;
    
    /* Use values */
    sink = device.config + device.samples[8] + device.status;
}

/* Test 5: Inline assembly for direct RTL influence */
void test_asm_patterns(void) {
    int buffer[32] __attribute__((aligned(64)));
    int index = 7;
    
    /* Complex addressing in asm output */
    asm volatile (
        "# Force complex MEM address\n\t"
        : "=m" (buffer[index * 2 + 3])  /* Complex addressing */
        : 
        : "memory"
    );
    
    /* Bitfield manipulation via asm */
    unsigned int value = 0x12345678;
    unsigned int mask = 0x00000FFF;
    unsigned int result;
    
    asm volatile (
        "and %[res], %[val], %[msk]\n\t"
        : [res] "=r" (result)
        : [val] "r" (value), [msk] "r" (mask)
    );
    
    sink = buffer[17] + result;
}

/* Test 6: Builtin operations on sub-word data */
void test_builtin_operations(void) {
    volatile uint16_t data16 = 0xBEEF;
    volatile uint8_t data8 = 0xAB;
    
    /* Builtins that may involve bit extraction */
    int popcnt = __builtin_popcount(data16);      /* May examine bits */
    int parity = __builtin_parity(data8);         /* May examine bits */
    
    /* Combine with bitfield store */
    struct {
        volatile unsigned int count : 5;
        volatile unsigned int check : 3;
    } bf;
    
    bf.count = popcnt & 0x1F;          /* Potential ZERO_EXTRACT */
    bf.check = parity & 0x7;           /* Potential ZERO_EXTRACT */
    
    sink = bf.count + bf.check;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += sink;
    
    test_subreg_operations();
    checksum += sink;
    
    test_complex_addressing();
    checksum += sink;
    
    test_combined_patterns();
    checksum += sink;
    
    test_asm_patterns();
    checksum += sink;
    
    test_builtin_operations();
    checksum += sink;
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
