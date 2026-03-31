/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * by generating SET RTL patterns with ZERO_EXTRACT/STRICT_LOW_PART
 * destinations that reference memory.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    unsigned int ready: 1;
    unsigned int count: 4;
    unsigned int data: 8;
    unsigned int padding: 19;
};

volatile struct GlobalStatus g_status = {0, 0, 0, 0};

/* Function that takes pointer to bitfield struct - ensures memory destination */
void set_bitfield_in_memory(struct GlobalStatus *status, int condition) {
    /* External condition prevents dead code elimination */
    if (condition) {
        /* Multiple bitfield assignments to increase visibility */
        status->ready = 1;           /* ZERO_EXTRACT with MEM destination */
        COMPILER_BARRIER();
        status->count = condition & 0xF;  /* Another bitfield assignment */
        COMPILER_BARRIER();
    }
}

/* Function with complex control flow and inline assembly */
void manipulate_with_assembly(struct GlobalStatus *status, int val) {
    /* Inline assembly that reads/writes memory and clobbers registers */
    /* This creates scheduling dependencies */
    asm volatile (
        "movl %[val], %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %[data]\n\t"
        : [data] "=m" (status->data)  /* Memory output constraint */
        : [val] "r" (val)
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Another asm with different constraints */
    asm volatile (
        "lock orl $0x1, %[ready]\n\t"  /* Atomic operation on bitfield */
        : [ready] "+m" (status->ready)
        :
        : "memory"
    );
}

/* Function using STRICT_LOW_PART patterns via byte operations */
void byte_operations(void) {
    volatile uint32_t word = 0x12345678;
    volatile uint8_t *byte_ptr = (volatile uint8_t*)&word;
    
    /* Multiple byte writes that may generate STRICT_LOW_PART */
    byte_ptr[0] = 0xAA;  /* Writing low byte of 32-bit memory location */
    COMPILER_BARRIER();
    byte_ptr[1] = 0xBB;  /* Another byte write */
    
    /* Inline assembly with "=Q" constraint (byte-addressable register) */
    register uint8_t reg_byte asm("al");
    asm volatile (
        "movb $0xCC, %0\n\t"
        : "=Q" (reg_byte)
        :
        : "memory"
    );
    
    /* Store the byte register to memory */
    byte_ptr[2] = reg_byte;
}

/* Function with loop containing bitfield operations */
void loop_with_bitfields(int iterations) {
    struct LocalStruct {
        unsigned int flags: 3;
        unsigned int value: 5;
        unsigned int state: 2;
    } local;
    
    /* Take address to force memory storage */
    struct LocalStruct *ptr = &local;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple bitfield assignments in loop */
        ptr->flags = i & 0x7;
        ptr->value = (i * 3) & 0x1F;
        ptr->state = (i >> 1) & 0x3;
        
        /* Compiler barrier prevents loop optimization */
        COMPILER_BARRIER();
        
        /* External function call prevents optimization */
        if (i % 10 == 0) {
            printf("Iteration %d\n", i);
        }
    }
}

/* Atomic operations on bitfields */
void atomic_bitfield_ops(void) {
    struct AtomicStruct {
        unsigned int lock: 1;
        unsigned int counter: 7;
        unsigned int data: 24;
    } atomic_var = {0, 0, 0};
    
    /* Atomic operations often generate ZERO_EXTRACT with MEM */
    __sync_fetch_and_or(&atomic_var.lock, 1);
    COMPILER_BARRIER();
    __sync_fetch_and_add((int*)&atomic_var.counter, 1);
}

/* Main function with unpredictable control flow */
int main(int argc, char *argv[]) {
    /* Use argc for unpredictable conditions */
    int condition = argc > 1 ? atoi(argv[1]) : 5;
    
    printf("Starting resource pattern test...\n");
    
    /* Test 1: Bitfield in memory via pointer */
    set_bitfield_in_memory(&g_status, condition);
    
    /* Test 2: Assembly with memory constraints */
    manipulate_with_assembly(&g_status, condition * 2);
    
    /* Test 3: Byte operations for STRICT_LOW_PART */
    byte_operations();
    
    /* Test 4: Loop with bitfields */
    loop_with_bitfields(condition < 100 ? condition : 100);
    
    /* Test 5: Atomic operations */
    atomic_bitfield_ops();
    
    /* Mix operations to create complex scheduling graph */
    for (int i = 0; i < 3; i++) {
        g_status.data = (g_status.data + i) & 0xFF;
        g_status.count = (g_status.count + 1) & 0xF;
        COMPILER_BARRIER();
        
        /* Inline asm with both input and output dependencies */
        asm volatile (
            "movl %[in], %%ecx\n\t"
            "addl %%ecx, %[out]\n\t"
            : [out] "+m" (g_status.data)
            : [in] "r" (i)
            : "ecx", "memory"
        );
    }
    
    printf("Test completed. Status: ready=%d, count=%d, data=%d\n",
           g_status.ready, g_status.count, g_status.data);
    
    return 0;
}
