#include <stdio.h>
#include <stdint.h>

/* Test 1: Bitfield operations to generate ZERO_EXTRACT */
void test_bitfield_operations(void) {
    /* Volatile bitfield struct - encourages ZERO_EXTRACT for stores */
    volatile struct {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 12;
        unsigned int field4 : 3;
    } bf;
    
    /* Initialize some variables for complex expressions */
    unsigned int a = 0x12345678;
    unsigned int b = 0x9ABCDEF0;
    unsigned int c = 0x13579BDF;
    
    /* Multiple assignments to trigger ZERO_EXTRACT in SET_DEST */
    bf.field1 = (a & 0xF) + (b & 0x7);          /* Should generate ZERO_EXTRACT */
    bf.field2 = ((a >> 8) & 0xFF) ^ ((b >> 4) & 0xFF); /* Complex expression */
    bf.field3 = __builtin_popcount(a) + __builtin_parity(b); /* Builtin usage */
    bf.field4 = (c & 0x7) | ((a >> 16) & 0x1);  /* Mixed bitwise operations */
    
    /* Read back to prevent elimination */
    volatile unsigned int read_back = bf.field1 + bf.field2 + bf.field3 + bf.field4;
    (void)read_back;
}

/* Test 2: Sub-word type operations to generate SUBREG */
void test_subword_operations(void) {
    /* Volatile sub-word destinations */
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2;
    
    /* Register variables to encourage register operations */
    register int r1 = 0x12345678;
    register int r2 = 0x9ABCDEF0;
    register int r3 = 0x13579BDF;
    
    /* Explicit casts that may generate SUBREG in SET_DEST */
    vs1 = (short)r1;                    /* Simple narrowing cast */
    vs2 = (short)(r1 + r2);             /* Arithmetic then narrowing */
    vs3 = (short)((r1 & 0xFFFF) * (r2 & 0xFFFF)); /* Complex expression */
    
    /* Implicit narrowing through arithmetic */
    char c1 = 100;
    char c2 = 50;
    vc1 = c1 + c2;                      /* May overflow, truncates to char */
    vc2 = (c1 * c2) / 2;                /* More complex narrowing */
    
    /* Read back to prevent elimination */
    volatile int sum = vs1 + vs2 + vs3 + vc1 + vc2;
    (void)sum;
}

/* Test 3: Complex memory addressing to trigger MEM_P with address computation */
void test_complex_memory_addressing(void) {
    /* Local arrays with restrict to avoid aliasing assumptions */
    int arr1[256];
    int arr2[128];
    short arr3[512];
    
    /* Struct with array member */
    struct {
        int data[64];
        short extra[32];
    } s1, *ptr = &s1;
    
    /* Initialize some values */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 3;
    }
    
    /* Complex addressing patterns */
    for (int i = 0; i < 64; i++) {
        /* Multi-dimensional style addressing */
        int idx = i * 3 + 7;
        arr1[idx] = i * 5;                     /* Non-linear index */
        
        /* Pointer arithmetic with multiple offsets */
        *(arr2 + (i & 0x3F) + 16) = i * 7;
        
        /* Struct member access through pointer with index */
        ptr->data[i] = i * 11;
        ptr->extra[i & 0x1F] = (short)(i * 13); /* Combined with narrowing */
    }
    
    /* More complex address computation */
    int base = 32;
    for (int i = 0; i < 32; i++) {
        /* arr3[base + i*stride + offset] pattern */
        int stride = 7;
        int offset = 3;
        arr3[base + i * stride + offset] = (short)(i * 17);
    }
    
    /* Compute checksum to prevent elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += arr1[i * 3 + 7] + arr2[(i & 0x3F) + 16] + 
                   ptr->data[i] + ptr->extra[i & 0x1F];
    }
    (void)checksum;
}

/* Test 4: Combined patterns in single assignments */
void test_combined_patterns(void) {
    /* Struct combining bitfield and array */
    volatile struct {
        unsigned int flags : 8;
        unsigned int status : 4;
        short values[32];
        char bytes[64];
    } combined;
    
    /* Variables for complex expressions */
    register int rval = 0x89ABCDEF;
    int array_idx = 0;
    
    /* Combined assignment 1: Bitfield with complex expression */
    combined.flags = ((rval >> 8) & 0xFF) | ((rval >> 16) & 0x0F);
    
    /* Combined assignment 2: Array element with narrowing cast */
    combined.values[array_idx * 2 + 5] = (short)(rval + 0x1234);
    
    /* Combined assignment 3: Byte array with implicit narrowing */
    combined.bytes[array_idx * 3 + 7] = (rval & 0xFF) + 64;
    
    /* Inline assembly to directly influence RTL generation */
    int dummy = 42;
    asm volatile (
        "# Force complex memory operand\n"
        : "=m" (combined.values[array_idx * 4 + 3])  /* Complex addressing */
        : "r" (dummy)
    );
    
    /* Read back to prevent elimination */
    volatile int total = combined.flags + combined.status + 
                        combined.values[0] + combined.bytes[0];
    (void)total;
}

/* Test 5: Additional patterns using inline assembly for direct RTL control */
void test_asm_patterns(void) {
    int data[64];
    short sdata[128];
    
    /* Inline assembly with memory output and complex addressing */
    for (int i = 0; i < 16; i++) {
        int idx = i * 5 + 2;
        
        /* Memory output with complex addressing */
        asm volatile (
            "# Memory store with complex address\n"
            : "=m" (data[idx])      /* Should create MEM with complex address */
            : 
            : "memory"
        );
        
        /* Another with sub-word type */
        asm volatile (
            "# Short store with complex address\n"
            : "=m" (sdata[idx * 2 + 1])  /* Complex addressing for short */
            :
            : "memory"
        );
    }
    
    /* Compute checksum */
    volatile int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data[i];
    }
    (void)sum;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Execute all tests */
    test_bitfield_operations();
    checksum += 1;
    
    test_subword_operations();
    checksum += 2;
    
    test_complex_memory_addressing();
    checksum += 3;
    
    test_combined_patterns();
    checksum += 4;
    
    test_asm_patterns();
    checksum += 5;
    
    /* Final checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return checksum == 15 ? 0 : 1;
}
