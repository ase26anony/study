#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* Opaque function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_packed {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int status:4;
    unsigned int data:20;
} NOINLINE;

struct bitfield_packed test_zero_extract(void) {
    struct bitfield_packed bf;
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf.flag = (raw >> 0) & 0x7;
    bf.value = (raw >> 3) & 0x1F;
    bf.status = (raw >> 8) & 0xF;
    bf.data = (raw >> 12) & 0xFFFFF;
    
    /* Bitfield extraction with masking */
    unsigned int extracted = 0;
    extracted |= (bf.flag & 0x7) << 0;
    extracted |= (bf.value & 0x1F) << 3;
    extracted |= (bf.status & 0xF) << 8;
    extracted |= (bf.data & 0xFFFFF) << 12;
    
    /* Complex bitfield manipulation */
    struct bitfield_packed bf2;
    bf2.flag = bf.value ^ bf.status;
    bf2.value = (bf.data >> 5) & 0x1F;
    bf2.status = (bf.flag << 1) | (bf.value & 1);
    bf2.data = (extracted * 3) & 0xFFFFF;
    
    return bf2;
}

/* ========== STRICT_LOW_PART patterns ========== */
union byte_overlay {
    uint32_t full;
    uint8_t bytes[4];
    struct {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    };
} NOINLINE;

int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    union byte_overlay overlay;
    overlay.full = wide_reg;
    
    /* Byte-sized stores into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&wide_reg = 0xFF;  /* Store byte into low part */
    
    /* Union-based byte access */
    overlay.bytes[1] = overlay.bytes[0] ^ 0x55;
    overlay.b2 = overlay.b1 + overlay.b3;
    
    /* Truncation preserving high bits */
    uint32_t temp = wide_reg;
    uint8_t low_byte = temp & 0xFF;  /* Explicit truncation */
    temp = (temp & ~0xFF) | (low_byte ^ 0xAA);
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        overlay.bytes[i] = (overlay.bytes[i] * (i + 1)) & 0xFF;
    }
    
    /* Inline assembly forcing low-part register access */
    uint32_t result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "r" (overlay.full)
        : "%eax"
    );
    
    return result + overlay.full + temp;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

int test_subreg(void) {
    /* Vector operations with element extraction */
    v4si vec = {1, 2, 3, 4};
    v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Type punning between different sizes */
    uint32_t int_val = 0x87654321;
    uint16_t short_val = *(uint16_t*)&int_val;  /* SUBREG access */
    uint8_t char_val = *(uint8_t*)&int_val;
    
    /* Vector element extraction */
    int elem0 = vec[0];
    int elem2 = vec[2];
    short elem1_short = short_vec[1];
    short elem5_short = short_vec[5];
    
    /* Mixed-type operations */
    float float_val = 3.14159f;
    uint32_t int_from_float;
    memcpy(&int_from_float, &float_val, sizeof(float_val));
    
    /* Complex subregister access pattern */
    struct {
        v4si vec_part;
        uint32_t scalar_part;
    } mixed;
    
    mixed.vec_part = vec;
    mixed.scalar_part = int_val;
    
    /* Extract and recombine */
    uint32_t combined = (short_val << 16) | char_val;
    combined += elem0 + elem2 + elem1_short + elem5_short;
    combined ^= int_from_float;
    
    return combined + mixed.scalar_part;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested_data {
    int values[8];
    struct nested_data* next;
    volatile int counter;
};

int test_memory_operand(void) {
    /* Complex pointer chasing */
    struct nested_data data[4];
    VOLATILE_VAR struct nested_data* current = &data[0];
    
    /* Initialize linked structure */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 8; j++) {
            data[i].values[j] = i * 100 + j;
        }
        data[i].next = &data[i + 1];
        data[i].counter = 0;
    }
    data[3].next = NULL;
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    int** double_ptr = (int**)malloc(sizeof(int*));
    int* single_ptr = (int*)malloc(16 * sizeof(int));
    
    *triple_ptr = double_ptr;
    *double_ptr = single_ptr;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index;
    int sum = 0;
    
    /* Pointer chasing with volatile accesses */
    while (current != NULL) {
        /* Multiple memory operations in one expression */
        sum += current->values[idx % 8];
        sum += current->counter++;
        
        /* Complex addressing mode */
        sum += *(*(triple_ptr))[idx % 4];
        
        current = current->next;
        idx = (idx * 13 + 7) & 0xF;
    }
    
    /* Structure field access with computed offset */
    for (int i = 0; i < 4; i++) {
        int offset = (i * 2) % 8;
        sum += data[i].values[offset] - data[i].values[offset + 1];
    }
    
    /* Cleanup */
    free(single_ptr);
    free(double_ptr);
    free(triple_ptr);
    
    return sum;
}

/* ========== Main test driver ========== */
int main(void) {
    int total = 0;
    
    printf("Testing GCC RTL pattern generation for resource tracking...\n");
    
    /* Test ZERO_EXTRACT patterns */
    struct bitfield_packed bf_result = test_zero_extract();
    total += bf_result.flag + bf_result.value + bf_result.status + bf_result.data;
    
    /* Test STRICT_LOW_PART patterns */
    total += test_strict_low_part();
    
    /* Test SUBREG patterns */
    total += test_subreg();
    
    /* Test complex memory operand patterns */
    total += test_memory_operand();
    
    printf("Final checksum: %d\n", total);
    printf("If this compiles and runs, the RTL patterns should be generated.\n");
    
    /* Additional verification to prevent dead code elimination */
    VOLATILE_VAR int sink = total;
    asm volatile ("" : : "r"(sink));
    
    return 0;
}

/* Dummy external functions */
int get_index(void) { return global_index; }
void* get_ptr(void) { return global_ptr; }
