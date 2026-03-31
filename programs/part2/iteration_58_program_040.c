/* test_resource_coverage.c
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file, specifically lines 282-290 in mark_set_resources.
 * The goal is to create SET RTL patterns with ZERO_EXTRACT or STRICT_LOW_PART
 * destinations that reference memory.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global structures with bitfields to ensure memory storage */
struct bitfield_struct {
    unsigned int regular_field;
    unsigned int bitfield1 : 3;
    unsigned int bitfield2 : 5;
    unsigned int bitfield3 : 8;
    unsigned int bitfield4 : 16;
};

volatile struct bitfield_struct global_bitfield = {0};
struct bitfield_struct *global_ptr = &global_bitfield;

/* Another volatile bitfield structure */
volatile struct {
    unsigned int status : 1;
    unsigned int mode : 2;
    unsigned int flags : 4;
} volatile_status = {0};

/* Function to set bitfields via pointer - ensures memory destination */
void set_bitfields_via_pointer(struct bitfield_struct *s, int value) {
    /* Multiple bitfield assignments to increase pattern visibility */
    s->bitfield1 = value & 0x7;
    COMPILER_BARRIER();
    s->bitfield2 = (value >> 3) & 0x1F;
    COMPILER_BARRIER();
    s->bitfield3 = (value >> 8) & 0xFF;
    COMPILER_BARRIER();
}

/* Function with inline assembly that may generate STRICT_LOW_PART */
void partial_register_operations(void) {
    volatile uint8_t byte_var = 0;
    volatile uint16_t word_var = 0;
    volatile uint32_t dword_var = 0;
    
    /* Inline assembly with byte constraints that might generate STRICT_LOW_PART */
    __asm__ volatile(
        "movb $0x42, %0\n\t"
        : "=Q" (byte_var)  /* "Q" constraint for byte-addressable register */
        :
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* More complex assembly with memory destination */
    uint32_t temp = 0x12345678;
    __asm__ volatile(
        "movl %1, %%eax\n\t"
        "andl $0x7, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (byte_var)   /* Memory destination */
        : "r" (temp)
        : "eax", "memory"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_operations(void) {
    /* Atomic operations on bitfields often generate ZERO_EXTRACT patterns */
    struct {
        unsigned int atomic_field : 4;
    } atomic_struct = {0};
    
    /* Use __sync builtins which may generate complex RTL */
    __sync_fetch_and_or(&global_bitfield.bitfield1, 1);
    COMPILER_BARRIER();
    
    /* Simulate atomic operation on local struct with address taken */
    unsigned int *as_int = (unsigned int*)&atomic_struct;
    __sync_fetch_and_and(as_int, ~0xF);  /* Clear lower 4 bits */
    __sync_fetch_and_or(as_int, 0x5);    /* Set to 5 */
}

/* Complex control flow to ensure scheduling pass analyzes resources */
void complex_control_flow(int iterations, int *input) {
    struct bitfield_struct local_struct;
    struct bitfield_struct *ptr = &local_struct;
    
    /* Unpredictable control flow prevents optimization */
    for (int i = 0; i < iterations; i++) {
        if (input[i] & 1) {
            /* Bitfield assignment with ZERO_EXTRACT destination in memory */
            ptr->bitfield1 = input[i] & 0x7;
            
            /* Inline assembly that reads/writes overlapping resources */
            __asm__ volatile(
                "movl %1, %%eax\n\t"
                "shrl $3, %%eax\n\t"
                "andl $0x1F, %%eax\n\t"
                "movb %%al, %0\n\t"
                : "=m" (ptr->bitfield2)
                : "r" (input[i])
                : "eax", "memory"
            );
        } else {
            /* Different bitfield assignment pattern */
            volatile_status.status = (input[i] >> 1) & 0x1;
            volatile_status.mode = (input[i] >> 2) & 0x3;
        }
        
        /* Compiler barrier prevents merging of operations */
        COMPILER_BARRIER();
        
        /* Additional memory operations to create resource conflicts */
        if (i % 3 == 0) {
            __asm__ volatile(
                "lock orl $0x1, %0\n\t"
                : "+m" (global_bitfield.regular_field)
                :
                : "memory"
            );
        }
    }
}

/* Function that mixes bitfields and inline assembly */
void mixed_operations(void) {
    /* Create a local struct and take its address */
    struct bitfield_struct local;
    struct bitfield_struct *local_ptr = &local;
    
    /* Initialize */
    local.regular_field = 0xDEADBEEF;
    
    /* Series of bitfield operations */
    local_ptr->bitfield1 = 0x3;
    COMPILER_BARRIER();
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    register uint8_t reg_al asm("al");
    __asm__ volatile(
        "movb $0x7F, %%al\n\t"
        "movb %%al, %0\n\t"
        : "=m" (local_ptr->bitfield2)
        : 
        : "al", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* More bitfield operations */
    local_ptr->bitfield3 = 0xAA;
    local_ptr->bitfield4 = 0x1234;
    
    /* Use the values to prevent dead code elimination */
    volatile int sink = local.bitfield1 + local.bitfield2 + local.bitfield3 + local.bitfield4;
    (void)sink;
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    /* Use argc to create unpredictable control flow */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Create input array with varying values */
    int *input_array = (int*)malloc(iterations * sizeof(int));
    for (int i = 0; i < iterations; i++) {
        input_array[i] = rand() & 0xFF;  /* Random 8-bit values */
    }
    
    /* Exercise different patterns */
    printf("Starting resource pattern tests...\n");
    
    /* 1. Bitfield assignments via pointer (ZERO_EXTRACT in memory) */
    set_bitfields_via_pointer(&global_bitfield, 0x55);
    
    /* 2. Partial register operations (potential STRICT_LOW_PART) */
    partial_register_operations();
    
    /* 3. Atomic operations on bitfields */
    atomic_bitfield_operations();
    
    /* 4. Complex control flow with mixed operations */
    complex_control_flow(iterations, input_array);
    
    /* 5. Mixed operations on local struct */
    mixed_operations();
    
    /* Additional volatile bitfield assignments */
    for (int i = 0; i < 5; i++) {
        volatile_status.flags = i & 0xF;
        COMPILER_BARRIER();
    }
    
    /* Use the results to prevent optimization */
    printf("Results: bitfield1=%u, status=%u\n", 
           global_bitfield.bitfield1, 
           volatile_status.status);
    
    free(input_array);
    return 0;
}
