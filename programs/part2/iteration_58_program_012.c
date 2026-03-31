/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c -o test.o
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    unsigned int ready : 1;
    unsigned int count : 7;
    unsigned int flags : 8;
    unsigned int data  : 16;
} global_status;

/* Volatile bitfield struct to force memory access */
volatile struct {
    unsigned int control : 4;
    unsigned int status  : 4;
} volatile_ctrl;

/* Function to set bitfield via pointer - ensures ZERO_EXTRACT in memory */
void set_bitfield_via_pointer(struct GlobalStatus *ptr, int value) {
    /* Multiple bitfield assignments to increase RTL visibility */
    ptr->count = value & 0x7F;
    /* Compiler barrier to prevent merging */
    asm volatile("" : : : "memory");
    ptr->flags = (value >> 7) & 0xFF;
}

/* Function with complex control flow to prevent optimization */
void conditional_bitfield_set(int condition, struct GlobalStatus *ptr) {
    /* Unpredictable condition based on external input */
    if (condition & 1) {
        ptr->ready = 1;
    } else {
        ptr->ready = 0;
    }
    
    /* Additional operations to create scheduling complexity */
    for (int i = 0; i < (condition & 3); i++) {
        ptr->count = (ptr->count + 1) & 0x7F;
    }
}

/* Function using inline assembly with partial register access */
void partial_register_operations(void) {
    /* STRICT_LOW_PART pattern: partial register assignment */
    register uint8_t byte_reg asm("al");
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile(
        "movb $0x42, %0\n\t"
        "addb $0x10, %0"
        : "=Q" (byte_reg)
        :
        : "cc"
    );
    
    /* Store to memory to ensure MEM reference */
    volatile uint8_t memory_byte;
    asm volatile(
        "movb %1, %0"
        : "=m" (memory_byte)
        : "r" (byte_reg)
        : "memory"
    );
}

/* Function with atomic operations on bitfields */
void atomic_bitfield_ops(struct GlobalStatus *ptr) {
    /* Atomic operations may generate ZERO_EXTRACT with memory */
    __sync_fetch_and_or(&ptr->flags, 0x01);
    __sync_fetch_and_and(&ptr->flags, 0xFE);
}

/* Complex function with mixed operations for scheduling */
void complex_scheduling_test(struct GlobalStatus *ptr, int iterations) {
    volatile int i;
    
    for (i = 0; i < iterations; i++) {
        /* Bitfield assignment - potential ZERO_EXTRACT */
        ptr->data = (ptr->data + i) & 0xFFFF;
        
        /* Volatile bitfield assignment */
        volatile_ctrl.control = i & 0x0F;
        
        /* Inline assembly with memory clobber */
        asm volatile(
            "movl %1, %%eax\n\t"
            "andl $0x7F, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (ptr->count)
            : "r" (i)
            : "eax", "memory"
        );
        
        /* Compiler barrier between operations */
        asm volatile("" : : : "memory");
    }
}

/* Main function that creates the necessary patterns */
int main(int argc, char *argv[]) {
    struct GlobalStatus local_status = {0};
    struct GlobalStatus *heap_status = malloc(sizeof(struct GlobalStatus));
    
    if (!heap_status) return 1;
    
    /* Initialize */
    global_status.ready = 0;
    global_status.count = 0;
    global_status.flags = 0;
    global_status.data = 0;
    
    /* 1. Bitfield assignment to global (memory) */
    global_status.ready = 1;
    
    /* 2. Bitfield via pointer - should generate ZERO_EXTRACT with MEM */
    set_bitfield_via_pointer(&global_status, argc);
    
    /* 3. Conditional bitfield with unpredictable flow */
    conditional_bitfield_set(argc, &global_status);
    
    /* 4. Partial register operations */
    partial_register_operations();
    
    /* 5. Atomic operations on bitfield */
    atomic_bitfield_ops(&global_status);
    
    /* 6. Complex scheduling test with loop */
    complex_scheduling_test(&global_status, argc > 1 ? atoi(argv[1]) : 10);
    
    /* 7. Local struct with address taken */
    set_bitfield_via_pointer(&local_status, argc * 2);
    
    /* 8. Heap allocated struct */
    heap_status->ready = 1;
    heap_status->count = 64;
    set_bitfield_via_pointer(heap_status, argc * 3);
    
    /* Use results to prevent dead code elimination */
    printf("Global: ready=%u count=%u flags=%u data=%u\n",
           global_status.ready, global_status.count, 
           global_status.flags, global_status.data);
    printf("Local: ready=%u count=%u\n",
           local_status.ready, local_status.count);
    printf("Heap: ready=%u count=%u\n",
           heap_status->ready, heap_status->count);
    
    free(heap_status);
    return 0;
}
