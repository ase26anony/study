/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
volatile int volatile_input = 12345;
volatile int volatile_index = 2;
volatile short volatile_short = 100;
volatile char volatile_char = 7;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Bitfield extraction using union - should generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int high:24;
        } bits;
    } u;
    
    u.full = volatile_input;
    int result = u.bits.high;  /* Extract high 24 bits */
    
    /* Complex control flow to keep RTL alive */
    if (volatile_input & 1) {
        result ^= 0xFF;
    } else {
        result |= 0xAA;
    }
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    int temp = volatile_input;
    
    /* Modify only low part through pointer - should generate STRICT_LOW_PART */
    struct {
        char low;
        char pad[3];
    } s;
    
    /* Multiple assignments to low part */
    for (int i = 0; i < 3; i++) {
        s.low = (temp & 0xFF) + i;
        temp = s.low * 2;
    }
    
    /* Conditional based on volatile */
    switch (volatile_char & 3) {
        case 0: s.low += 1; break;
        case 1: s.low -= 2; break;
        case 2: s.low *= 3; break;
        default: s.low = 0; break;
    }
    
    use_int(s.low);
    return s.low;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * volatile_input;
    }
    
    /* Complex memory addressing with type punning - should generate SUBREG and MEM */
    int idx = volatile_index;
    
    /* Access as different types */
    short* ptr_short = (short*)((char*)array + idx * sizeof(int));
    *ptr_short = volatile_short;  /* Store short into int array */
    
    /* Another SUBREG pattern */
    long* ptr_long = (long*)&array[4];
    *ptr_long = (long)volatile_input * 256;
    
    /* Pointer arithmetic with different strides */
    char* char_ptr = (char*)array;
    for (int i = 0; i < 8; i++) {
        char_ptr[i * 2] = (char)(volatile_input + i);
    }
    
    /* Compute result from modified array */
    for (int i = 0; i < 8; i++) {
        result += array[i] & 0xFFFF;  /* Extract low part */
    }
    
    use_ptr(array);
    return result;
}

/* Function 4: Combined pattern - ZERO_EXTRACT + STRICT_LOW_PART + MEM */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int v = volatile_input;
    int buffer[8] = {0};
    
    /* ZERO_EXTRACT pattern */
    union {
        uint32_t dword;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
    } converter;
    
    converter.dword = v;
    uint16_t extracted = converter.words.high;  /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART pattern */
    struct {
        uint16_t value;
    } container;
    container.value = extracted;  /* STRICT_LOW_PART */
    
    /* MEM pattern with SUBREG */
    int* ptr = &buffer[volatile_index & 3];
    *((short*)ptr) = container.value;  /* SUBREG + MEM */
    
    /* Complex addressing mode */
    int* base = buffer;
    int offset = volatile_index;
    int* addr = base + offset;
    *addr = v;
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += buffer[i];
    }
    
    use_int(sum);
    return sum;
}

/* Function 5: Nested patterns in loop */
__attribute__((noinline))
static int pattern_nested_loop(void) {
    int data[10];
    int result = 0;
    
    /* Initialize with volatile */
    for (int i = 0; i < 10; i++) {
        data[i] = volatile_input + i;
    }
    
    /* Loop with mixed patterns */
    for (int iter = 0; iter < 4; iter++) {
        /* ZERO_EXTRACT in loop */
        union {
            int val;
            struct {
                unsigned char a, b, c, d;
            } bytes;
        } u;
        u.val = data[iter];
        
        /* STRICT_LOW_PART assignment */
        struct {
            unsigned char low;
        } s;
        s.low = u.bytes.b;  /* Modify only low part */
        
        /* MEM access with SUBREG */
        short* sp = (short*)&data[iter + 1];
        *sp = (short)s.low * 2;
        
        /* Update result */
        result += data[iter] & 0xFF;
    }
    
    use_int(result);
    return result;
}

/* Main function - execute all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all pattern functions */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_nested_loop();
    
    /* Additional volatile-dependent control flow */
    if (volatile_input > 1000) {
        checksum += pattern_zero_extract();
    } else {
        checksum += pattern_strict_low_part();
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
