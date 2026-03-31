/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to prevent optimizations */
VOLATILE int global_index = 0;
VOLATILE void* global_ptr = NULL;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int data:12;
    unsigned int flags:8;
    unsigned int checksum:8;
} NOINLINE;

/* Another bitfield with different layout */
struct control_reg {
    unsigned int enable:1;
    unsigned int mode:3;
    unsigned int reserved:4;
    unsigned int value:8;
    unsigned int status:16;
} NOINLINE;

NOINLINE static uint32_t test_zero_extract(void) {
    VOLATILE uint32_t result = 0;
    
    /* Initialize bitfield structures */
    struct bitfield_packet packet = {0};
    struct control_reg control = {0};
    
    /* Operations that may generate ZERO_EXTRACT */
    packet.header = 0xA;
    packet.data = 0xABC;
    packet.flags = 0x3F;
    packet.checksum = packet.header ^ (packet.data & 0xFF) ^ packet.flags;
    
    control.enable = 1;
    control.mode = 5;
    control.value = 0xCD;
    control.status = (control.enable << 15) | (control.mode << 12) | control.value;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    uint32_t raw_data = 0xDEADBEEF;
    
    /* Multiple extraction patterns */
    uint32_t low_bits = (raw_data >> 0) & 0xFF;      /* Extract byte 0 */
    uint32_t high_bits = (raw_data >> 24) & 0xFF;    /* Extract byte 3 */
    uint32_t middle = (raw_data >> 8) & 0xFFFF;      /* Extract bytes 1-2 */
    
    /* Bitfield extraction via masking */
    uint32_t mask1 = 0x00FF0000;
    uint32_t mask2 = 0x0000FF00;
    uint32_t extracted1 = (raw_data & mask1) >> 16;
    uint32_t extracted2 = (raw_data & mask2) >> 8;
    
    /* Combine results */
    result = packet.checksum + control.status + low_bits + high_bits + 
             middle + extracted1 + extracted2;
    
    /* Additional bit manipulation */
    uint32_t val = 0x12345678;
    for (int i = 0; i < 4; i++) {
        uint32_t byte = (val >> (i * 8)) & 0xFF;
        result += byte;
    }
    
    return result;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE static uint32_t test_strict_low_part(void) {
    VOLATILE uint32_t result = 0;
    VOLATILE uint32_t int_var = 0xDEADBEEF;
    VOLATILE uint16_t short_var = 0xCAFE;
    VOLATILE uint8_t byte_var = 0x42;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        uint32_t full;
        uint8_t bytes[4];
        uint16_t words[2];
    } data_union;
    
    data_union.full = 0x12345678;
    
    /* Byte-sized stores into wider integers (potential STRICT_LOW_PART) */
    data_union.bytes[0] = 0xAA;      /* Modify low byte */
    data_union.bytes[2] = 0xBB;      /* Modify third byte */
    
    /* Explicit low-part truncation */
    uint32_t temp = int_var;
    uint8_t low_byte = temp & 0xFF;           /* Extract low byte */
    uint16_t low_word = temp & 0xFFFF;        /* Extract low word */
    
    /* Store byte into integer through pointer (force byte store) */
    uint32_t target = 0xFFFFFFFF;
    *(volatile uint8_t*)&target = 0x11;       /* Modify only low byte */
    *(volatile uint8_t*)((char*)&target + 1) = 0x22; /* Modify second byte */
    
    /* Arithmetic that truncates to lower parts */
    uint32_t x = 0xABCD1234;
    uint32_t y = 0x5678;
    uint32_t sum = (x + y) & 0xFFFF;          /* Keep only low 16 bits */
    
    /* Inline assembly with byte register modifier (x86-specific) */
    uint32_t asm_in = 0x88776655;
    uint32_t asm_out;
    
    /* This may generate STRICT_LOW_PART for %b0 (low byte of register) */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* Combine results */
    result = data_union.full + low_byte + low_word + target + sum + asm_out;
    
    /* Additional low-part operations in loop */
    for (int i = 0; i < 4; i++) {
        uint32_t val = 0x100 * i + 0x42;
        uint8_t truncated = val;  /* Implicit truncation to low byte */
        result += truncated;
    }
    
    return result;
}

/* ==================== SUBREG patterns ==================== */

/* Vector types using GCC extensions */
typedef int32_t v4si __attribute__((vector_size(16)));
typedef int16_t v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static uint32_t test_subreg(void) {
    VOLATILE uint32_t result = 0;
    
    /* Vector operations that generate SUBREG accesses */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Extract individual elements (likely SUBREG) */
    int32_t elem0 = vec_int[0];
    int32_t elem2 = vec_int[2];
    int16_t short_elem = vec_short[3];
    float float_elem = vec_float[1];
    
    /* Type punning between different sizes */
    uint64_t big_val = 0x1122334455667788ULL;
    uint32_t low_part = (uint32_t)big_val;           /* Truncate to 32 bits */
    uint16_t smaller = (uint16_t)low_part;           /* Truncate to 16 bits */
    uint8_t smallest = (uint8_t)smaller;             /* Truncate to 8 bits */
    
    /* Cast between float and int (bitcasting) */
    float f = 3.14159f;
    uint32_t f_bits;
    memcpy(&f_bits, &f, sizeof(f_bits));  /* Type punning via memcpy */
    
    /* Direct cast through union (may generate SUBREG) */
    union {
        float f;
        uint32_t i;
    } pun;
    pun.f = 2.71828f;
    uint32_t e_bits = pun.i;
    
    /* Mixed-size arithmetic */
    int16_t s16 = -100;
    int32_t s32 = 1000;
    int64_t s64 = s16 * s32;  /* Promote to different size */
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } mixed_var = {'A', 123, 456, 789LL};
    
    /* Access different sized members */
    mixed_var.c = 'B';
    mixed_var.s = 321;
    mixed_var.i = 654;
    
    /* Combine results */
    result = elem0 + elem2 + short_elem + (uint32_t)float_elem +
             low_part + smaller + smallest + f_bits + e_bits +
             (uint32_t)s64 + mixed_var.i;
    
    /* Additional SUBREG patterns with arrays */
    int32_t array32[4] = {100, 200, 300, 400};
    int16_t array16[8];
    
    /* Copy with size change */
    for (int i = 0; i < 4; i++) {
        array16[i*2] = (int16_t)array32[i];      /* High part */
        array16[i*2+1] = (int16_t)(array32[i] >> 16); /* Low part */
        result += array16[i*2] + array16[i*2+1];
    }
    
    return result;
}

/* ==================== Memory operand patterns ==================== */

NOINLINE static uint32_t test_memory_operand(void) {
    VOLATILE uint32_t result = 0;
    
    /* Complex memory addressing structures */
    struct node {
        int value;
        struct node* next;
        struct node* prev;
    };
    
    /* Allocate and initialize linked list */
    struct node* nodes[4];
    for (int i = 0; i < 4; i++) {
        nodes[i] = (struct node*)malloc(sizeof(struct node));
        nodes[i]->value = i * 100;
        nodes[i]->next = (i < 3) ? nodes[i+1] : NULL;
        nodes[i]->prev = (i > 0) ? nodes[i-1] : NULL;
    }
    
    /* Complex pointer chasing (multi-level dereference) */
    struct node*** ptr_ptr_ptr = (struct node***)malloc(sizeof(struct node**));
    struct node** ptr_ptr = (struct node**)malloc(sizeof(struct node*));
    *ptr_ptr = nodes[0];
    *ptr_ptr_ptr = ptr_ptr;
    
    /* Multi-level dereferencing */
    int val1 = (***ptr_ptr_ptr).value;
    int val2 = (**ptr_ptr)->next->value;
    int val3 = (*nodes[1]->next).value;
    
    /* Array with volatile index (prevents constant propagation) */
    VOLATILE int idx = global_index;
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Complex array indexing */
    int* ptr_array[10];
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &array[i * 10];
    }
    
    /* Volatile memory accesses */
    VOLATILE int* volatile_ptr = &array[idx % 100];
    int volatile_val = *volatile_ptr;
    *volatile_ptr = volatile_val + 1;
    
    /* Structure field access with pointer arithmetic */
    struct data {
        int a;
        int b[5];
        int c;
    } data_item = {0};
    
    struct data* data_ptr = &data_item;
    for (int i = 0; i < 5; i++) {
        data_ptr->b[i] = i * 10;
        result += data_ptr->b[i];
    }
    
    /* Memory access with offset calculation */
    char* byte_ptr = (char*)&data_item;
    for (int i = 0; i < (int)sizeof(struct data); i++) {
        byte_ptr[i] = (char)(i % 256);
        result += byte_ptr[i];
    }
    
    /* Combine results */
    result += val1 + val2 + val3 + volatile_val + data_item.a + data_item.c;
    
    /* Cleanup */
    free(ptr_ptr);
    free(ptr_ptr_ptr);
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    
    return result;
}

/* ==================== Main function ==================== */

int main(void) {
    uint32_t total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    printf("  test_zero_extract completed\n");
    
    total += test_strict_low_part();
    printf("  test_strict_low_part completed\n");
    
    total += test_subreg();
    printf("  test_subreg completed\n");
    
    total += test_memory_operand();
    printf("  test_memory_operand completed\n");
    
    printf("Total checksum: %u (0x%08X)\n", total, total);
    
    return (total > 0) ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) { return 42; }
void* get_ptr(void) { return &global_index; }
