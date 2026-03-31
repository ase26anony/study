/* test_resource_coverage.c */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to keep computations in registers */
#define KEEP_IN_REGISTER __attribute__((noinline))

/* Test 1: Bitfield operations for ZERO_EXTRACT/STRICT_LOW_PART */
static int test_bitfields(void) {
    volatile struct {
        unsigned int field4:4;    /* Likely ZERO_EXTRACT for 4-bit field */
        unsigned int field8:8;    /* 8-bit field */
        unsigned int field12:12;  /* 12-bit field */
    } bf = {0};
    
    /* Use volatile sources to prevent constant propagation */
    volatile unsigned int source1 = 0xABCD;
    volatile unsigned int source2 = 0x1234;
    
    /* Complex expression that might generate ZERO_EXTRACT in SET_DEST */
    bf.field4 = (source1 & 0xF) + (source2 & 0xF);  /* 4-bit assignment */
    bf.field8 = (source1 >> 4) & 0xFF;              /* 8-bit assignment */
    bf.field12 = (source1 + source2) & 0xFFF;       /* 12-bit assignment */
    
    /* Read back to prevent elimination */
    return bf.field4 + bf.field8 + bf.field12;
}

/* Test 2: SUBREG in SET_DEST through type narrowing */
static int test_subreg(void) {
    /* Volatile destination forces store */
    volatile int16_t dest16;
    volatile int8_t dest8;
    
    /* Register variables encourage SUBREG patterns */
    register int32_t reg32 asm("") = 0x12345678;
    register int32_t reg32_2 asm("") = 0x9ABCDEF0;
    
    /* Narrowing assignments that may create SUBREG */
    dest16 = (int16_t)(reg32 + 0x100);      /* 32-bit to 16-bit */
    dest8 = (int8_t)(reg32_2 & 0xFF);       /* 32-bit to 8-bit */
    
    /* Arithmetic with implicit narrowing */
    int16_t temp16 = reg32;                 /* Implicit truncation */
    dest16 = temp16 + 1;                    /* SUBREG in destination */
    
    return dest16 + dest8;
}

/* Test 3: Complex memory addressing for MEM_P(x) */
static int test_complex_mem(void) {
    int array[256] = {0};
    int *restrict ptr = array;  /* restrict helps keep address computation */
    
    volatile int indices[4] = {7, 13, 42, 97};
    register int values[4] = {100, 200, 300, 400};
    
    /* Complex addressing patterns */
    for (int i = 0; i < 4; i++) {
        /* Multi-dimensional style addressing */
        int idx = indices[i] * 3 + 7;           /* Non-linear computation */
        ptr[idx] = values[i];                   /* Store with complex address */
        
        /* Pointer arithmetic with multiple offsets */
        *(ptr + idx + (i * 2)) = values[i] + 1;
    }
    
    /* Struct with array member */
    struct {
        int data[64];
        int offset;
    } s = {{0}, 16};
    
    s.data[s.offset + 5] = 999;                /* Base + offset + constant */
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += array[i];
    }
    return sum + s.data[s.offset + 5];
}

/* Test 4: Combined patterns */
static int test_combined(void) {
    /* Struct combining bitfield and array */
    volatile struct combined {
        unsigned int flags:8;
        int16_t shorts[32];
        unsigned int status:4;
    } comb = {{0}};
    
    register int32_t r1 = 0x89ABCDEF;
    register int32_t r2 = 0x76543210;
    
    /* Combined assignment: bitfield + array with complex index */
    comb.flags = (r1 & 0xFF) | ((r2 >> 8) & 0xFF);  /* ZERO_EXTRACT potential */
    
    /* Array store with narrowing and complex index */
    int idx = ((int)r1 & 0xF) * 2 + 3;              /* Complex index calculation */
    comb.shorts[idx] = (int16_t)(r1 + r2);          /* SUBREG potential in dest */
    
    /* Another bitfield */
    comb.status = (r1 ^ r2) & 0xF;
    
    return comb.flags + comb.shorts[idx] + comb.status;
}

/* Test 5: Inline assembly for direct RTL influence */
static int test_asm(void) {
    int array[64] = {0};
    int result = 0;
    
    /* Complex memory output with asm */
    for (int i = 0; i < 4; i++) {
        int idx = i * 7 + 3;
        /* Empty asm with memory output constraint */
        asm volatile (
            "# dummy asm with complex memory destination"
            : "=m" (array[idx])   /* Complex addressing in output */
            : 
            : "memory"
        );
        array[idx] = i * 100;     /* Follow up with actual store */
    }
    
    /* Bitfield-like asm (less portable but worth trying) */
    volatile unsigned int packed;
    asm volatile (
        "# bitfield-like store"
        : "=m" (packed)
        :
        : "memory"
    );
    
    /* Compute sum */
    for (int i = 0; i < 64; i++) {
        result += array[i];
    }
    return result + packed;
}

/* Test 6: Builtin operations on sub-word data */
static int test_builtins(void) {
    volatile uint16_t data16 = 0xBEEF;
    volatile uint8_t data8 = 0x42;
    int result = 0;
    
    /* Builtins that may operate on extracted bits */
    result += __builtin_popcount(data16);      /* May extract 16 bits from reg */
    result += __builtin_parity(data8);         /* May extract 8 bits */
    
    /* Store results to bitfields */
    volatile struct {
        unsigned int popcnt:5;
        unsigned int parity:2;
    } bf = {0};
    
    bf.popcnt = __builtin_popcount(data16) & 0x1F;
    bf.parity = __builtin_parity(data8) & 0x3;
    
    return result + bf.popcnt + bf.parity;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing resource.cc coverage patterns...\n");
    
    /* Run all tests */
    checksum += test_bitfields();
    checksum += test_subreg();
    checksum += test_complex_mem();
    checksum += test_combined();
    checksum += test_asm();
    checksum += test_builtins();
    
    printf("Final checksum: %d\n", checksum);
    printf("(Expected non-zero if all tests executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
