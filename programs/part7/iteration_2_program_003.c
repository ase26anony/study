/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile inputs to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile short g_volatile_short = 100;
volatile char g_volatile_char = 'A';
volatile int g_volatile_index = 3;

/* ===== Pattern 1: ZERO_EXTRACT ===== */
__attribute__((noinline))
int pattern_zero_extract(void) {
    /* Union with bitfields to generate ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low_bits : 8;
            uint32_t middle_bits : 12;
            uint32_t high_bits : 12;
        } parts;
    } data;
    
    volatile uint32_t input = g_volatile_int;
    data.full = input;
    
    /* These operations should generate ZERO_EXTRACT RTL */
    uint32_t extracted1 = data.parts.middle_bits;
    uint32_t extracted2 = data.parts.high_bits;
    
    /* Use results to keep them alive */
    use_int(extracted1);
    use_int(extracted2);
    
    return extracted1 + extracted2;
}

/* ===== Pattern 2: STRICT_LOW_PART ===== */
__attribute__((noinline))
int pattern_strict_low_part(void) {
    /* Structure with small integer to generate STRICT_LOW_PART */
    struct {
        char low_byte;
        char padding[3];
        int full_word;
    } container;
    
    volatile int temp = g_volatile_int;
    
    /* This assignment should generate STRICT_LOW_PART RTL */
    container.low_byte = (char)(temp & 0xFF);
    
    /* Another pattern using pointer to low part */
    int* ptr = &container.full_word;
    *(char*)ptr = g_volatile_char;  /* Modify only low byte */
    
    use_char(container.low_byte);
    use_int(container.full_word);
    
    return container.low_byte + container.full_word;
}

/* ===== Pattern 3: SUBREG and MEM_P ===== */
__attribute__((noinline))
int pattern_subreg_mem(void) {
    int array[16];
    volatile int idx = g_volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 10;
    }
    
    /* Complex memory addressing with type punning */
    /* This should generate SUBREG and MEM RTL patterns */
    
    /* Pattern 3a: Access through different pointer types */
    int* int_ptr = &array[idx];
    short* short_ptr = (short*)int_ptr;
    char* char_ptr = (char*)int_ptr;
    
    /* Pattern 3b: SUBREG through pointer arithmetic */
    int value1 = *short_ptr;  /* Load short, extend to int */
    *char_ptr = g_volatile_char;  /* Store char */
    
    /* Pattern 3c: More complex addressing */
    int offset = idx * sizeof(int) / 2;
    short* ptr2 = (short*)((char*)array + offset);
    *ptr2 = g_volatile_short;
    
    use_short(*short_ptr);
    use_int(value1);
    use_ptr(ptr2);
    
    return array[idx] + *short_ptr + value1;
}

/* ===== Combined Pattern ===== */
__attribute__((noinline))
int pattern_combined(void) {
    /* Combine ZERO_EXTRACT, STRICT_LOW_PART, and MEM/SUBREG */
    
    union {
        uint32_t value;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
    } data;
    
    volatile uint32_t input = g_volatile_int;
    data.value = input;
    
    /* ZERO_EXTRACT pattern */
    uint16_t extracted = data.words.high;
    
    /* STRICT_LOW_PART pattern */
    struct {
        uint32_t full;
        uint8_t low_byte;
    } container;
    container.full = extracted;
    container.low_byte = (uint8_t)(extracted & 0xFF);  /* Modify low part only */
    
    /* MEM/SUBREG pattern with array */
    uint8_t buffer[8];
    volatile int idx = g_volatile_index & 0x3;  /* Limit to 0-3 */
    
    /* Store through different pointer types */
    *(uint16_t*)(buffer + idx) = container.full;
    *(uint8_t*)(buffer + idx + 2) = container.low_byte;
    
    /* Load through different type */
    uint32_t result = *(uint16_t*)(buffer + idx);
    
    use_int(result);
    return result;
}

/* ===== Pattern with Control Flow ===== */
__attribute__((noinline))
int pattern_with_control_flow(void) {
    volatile int condition = g_volatile_int;
    int result = 0;
    
    /* Switch statement to create different basic blocks */
    switch (condition & 0x3) {
        case 0: {
            /* ZERO_EXTRACT in one path */
            union {
                uint32_t val;
                struct { uint16_t a:4; uint16_t b:12; } bits;
            } u;
            u.val = condition;
            result = u.bits.b;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART in another path */
            int temp = condition;
            struct { char c; } s;
            s.c = temp & 0x7F;  /* Only modify low 7 bits */
            result = s.c;
            break;
        }
        case 2: {
            /* MEM/SUBREG in third path */
            int arr[4] = {1, 2, 3, 4};
            short* sp = (short*)arr;
            result = sp[g_volatile_index & 0x3];
            break;
        }
        default: {
            /* Combined pattern in default */
            union { int i; short s[2]; } u;
            u.i = condition;
            u.s[1] = g_volatile_short;  /* Modify high short */
            result = u.i;
            break;
        }
    }
    
    /* Loop with MEM access */
    int sum = 0;
    int loop_array[8];
    for (int i = 0; i < 8; i++) {
        loop_array[i] = i + result;
        /* Access through byte pointer */
        char* cp = (char*)&loop_array[i];
        sum += cp[0];
    }
    
    use_int(sum);
    return result + sum;
}

/* ===== Main Function ===== */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Call all pattern functions */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_with_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}

/* ===== External Function Definitions (weak) ===== */
/* These would normally be in a separate file, but included here for completeness */
#ifdef INCLUDE_WEAK_DEFS
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_char(char x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
#endif
