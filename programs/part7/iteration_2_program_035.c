/* test_resource.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int g_volatile_seed = 12345;
static volatile int g_volatile_index = 0;
static volatile short g_volatile_short = 0xABCD;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
int test_zero_extract(void) {
    volatile int input = g_volatile_seed;
    int result = 0;
    
    /* Method 1: Using bitfield union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = input;
    result = u.bits.mid;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual extraction with volatile */
    volatile uint32_t mask = 0x0FF00000;
    volatile uint32_t shift = 20;
    result += (input & mask) >> shift;  /* Another ZERO_EXTRACT candidate */
    
    /* Method 3: Nested extractions */
    for (int i = 0; i < 3; i++) {
        volatile int offset = i * 4;
        result += (input >> offset) & 0xF;  /* Multiple ZERO_EXTRACT */
    }
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
int test_strict_low_part(void) {
    volatile int input = g_volatile_seed + 1;
    int result = 0;
    
    /* Method 1: Structure with small members */
    struct {
        signed char low_byte;
        short low_word;
        int full;
    } s;
    
    s.full = input;
    s.low_byte = input & 0xFF;  /* STRICT_LOW_PART candidate */
    s.low_word = input & 0xFFFF; /* Another candidate */
    
    /* Method 2: Pointer to low part */
    int temp = input;
    char *low_ptr = (char*)&temp;
    *low_ptr = input & 0xFF;  /* Modifies only low byte */
    
    /* Method 3: Union type punning */
    union {
        int32_t i;
        int16_t s[2];
    } u;
    u.i = input;
    u.s[0] = (input & 0xFFFF) ^ 0x55AA;  /* STRICT_LOW_PART */
    
    result = s.low_byte + *low_ptr + u.s[0];
    use_int(result);
    return result;
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
int test_subreg(void) {
    volatile int base = g_volatile_seed + 2;
    int result = 0;
    
    /* Method 1: Array with different type access */
    int32_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = base + i * 100;
    }
    
    volatile int idx = g_volatile_index & 7;
    
    /* Access through different sized pointers */
    int16_t *short_ptr = (int16_t*)&array[idx];
    int8_t *byte_ptr = (int8_t*)&array[idx];
    
    /* Multiple SUBREG accesses */
    result += short_ptr[0];  /* SUBREG from 32-bit to 16-bit */
    result += short_ptr[1];  /* Another SUBREG */
    result += byte_ptr[2];   /* SUBREG from 32-bit to 8-bit */
    
    /* Method 2: Register-sized operations */
    long long big_val = (long long)base * 1000;
    int32_t *parts = (int32_t*)&big_val;
    result += parts[0];  /* Low part - SUBREG candidate */
    result += parts[1];  /* High part - SUBREG candidate */
    
    use_int(result);
    return result;
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
int test_mem_complex(void) {
    volatile int input = g_volatile_seed + 3;
    int result = 0;
    
    /* Complex memory addressing */
    int buffer[32];
    volatile int offset = g_volatile_index;
    
    /* Initialize buffer */
    for (int i = 0; i < 32; i++) {
        buffer[i] = input + i * 17;
    }
    
    /* Multiple addressing modes */
    int *ptr1 = &buffer[offset & 31];
    int *ptr2 = buffer + ((offset * 3) & 31);
    int *ptr3 = (int*)((char*)buffer + (offset * 4));
    
    /* Access through computed addresses */
    result += *ptr1;
    result += ptr2[1];
    result += ptr3[2];
    
    /* Nested memory access */
    int **ptr_to_ptr = &ptr1;
    result += **ptr_to_ptr;
    
    use_int(result);
    return result;
}

/* Function 5: Combined patterns */
__attribute__((noinline))
int test_combined(void) {
    volatile int input = g_volatile_seed + 4;
    int result = 0;
    
    /* Structure with bitfields and arrays */
    struct {
        union {
            uint32_t flags;
            struct {
                uint32_t mode: 3;
                uint32_t count: 10;
                uint32_t reserved: 19;
            } bits;
        } u;
        short data[4];
        int values[2];
    } ctx;
    
    /* Initialize */
    ctx.u.flags = input;
    for (int i = 0; i < 4; i++) {
        ctx.data[i] = (input + i * 100) & 0xFFFF;
    }
    
    /* ZERO_EXTRACT from bitfield */
    result = ctx.u.bits.count;  /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART through pointer */
    short *low_ptr = &ctx.data[0];
    *low_ptr = input & 0x7FFF;  /* STRICT_LOW_PART */
    
    /* SUBREG access */
    int32_t *as_int = (int32_t*)ctx.data;
    result += as_int[0];  /* SUBREG access */
    
    /* Complex MEM access */
    volatile int idx = (input >> 3) & 1;
    ctx.values[idx] = result;
    result += ctx.values[!idx];
    
    use_int(result);
    return result;
}

/* Function 6: Control flow variations */
__attribute__((noinline))
int test_control_flow(void) {
    volatile int input = g_volatile_seed + 5;
    int result = 0;
    
    /* Switch with different operations */
    switch (input & 0x3) {
        case 0: {
            /* ZERO_EXTRACT in case 0 */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 5;
                    uint32_t b: 11;
                    uint32_t c: 16;
                } f;
            } u = { .val = input };
            result = u.f.b;  /* ZERO_EXTRACT */
            break;
        }
        case 1: {
            /* STRICT_LOW_PART in case 1 */
            int temp = input;
            struct {
                char low;
                char high[3];
            } s;
            s.low = temp & 0x7F;  /* STRICT_LOW_PART */
            result = s.low;
            break;
        }
        case 2: {
            /* SUBREG in case 2 */
            long long ll = input * 100LL;
            int *parts = (int*)&ll;
            result = parts[0] + parts[1];  /* SUBREG accesses */
            break;
        }
        default: {
            /* Complex MEM in default */
            int arr[8] = {0};
            volatile int idx = input & 7;
            int *ptr = (int*)((char*)arr + idx * sizeof(int));
            *ptr = input;  /* MEM with addressing */
            result = *ptr;
            break;
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < 4; i++) {
        volatile int iter = input + i;
        
        if (iter & 1) {
            /* Bitfield extraction in loop */
            result += (iter >> (i * 2)) & 0x3;  /* ZERO_EXTRACT */
        } else {
            /* Sub-register access in loop */
            short *sp = (short*)&iter;
            result += sp[0];  /* SUBREG */
        }
    }
    
    use_int(result);
    return result;
}

/* Main function */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all test functions */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_mem_complex();
    checksum += test_combined();
    checksum += test_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
