/* test_resource_coverage.c
 * Designed to trigger uncovered lines 282-290 in resource.cc
 * Compile with: gcc -O1 -c test_resource_coverage.c -o test.o
 * Or for scheduling: gcc -O2 -fschedule-insns -c test_resource_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from removing critical patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Global struct with bitfield to ensure memory storage */
struct GlobalStatus {
    volatile unsigned int ready : 1;
    unsigned int count : 7;
    unsigned int flags : 8;
    unsigned int padding : 16;
};

struct GlobalStatus global_status = {0, 0, 0, 0};

/* Struct passed by pointer to force memory access */
struct DeviceReg {
    volatile unsigned int control : 4;
    unsigned int data : 12;
    unsigned int status : 3;
    unsigned int error : 1;
    unsigned int reserved : 12;
};

/* Function that assigns to bitfield via pointer - should generate ZERO_EXTRACT with MEM */
void set_device_control(struct DeviceReg *dev, unsigned int value) {
    /* This assignment to a bitfield through a pointer should create
     * a SET with ZERO_EXTRACT destination that references memory */
    dev->control = value & 0xF;
    
    /* Additional operations to create scheduling complexity */
    dev->data = (value >> 4) & 0xFFF;
    
    COMPILER_BARRIER();
}

/* Function with multiple bitfield operations in a loop */
void update_status_bits(int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* These global bitfield assignments should remain as memory operations
         * due to volatility and external visibility */
        global_status.ready = (i & 1);
        global_status.count = (i & 0x7F);
        global_status.flags = (i & 0xFF);
        
        /* Insert compiler barrier to prevent loop optimization */
        if (i % 3 == 0) {
            COMPILER_BARRIER();
        }
    }
}

/* Function using inline assembly that might generate STRICT_LOW_PART */
void partial_register_ops(void) {
    volatile uint8_t byte_var = 0;
    uint32_t dword_var = 0;
    
    /* Inline assembly that writes to a byte location - may generate STRICT_LOW_PART
     * when the byte is part of a larger memory location */
    __asm__ volatile (
        "movb $0x42, %0\n\t"
        "addb $0x1, %0"
        : "=m" (byte_var)
        : 
        : "memory"
    );
    
    /* Mixed-size operations to create register pressure */
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movb %%al, %0\n\t"
        "movl %%eax, %1"
        : "=m" (byte_var), "=m" (dword_var)
        : 
        : "eax", "memory"
    );
}

/* Function using atomic operations on bitfields */
void atomic_bitfield_ops(struct DeviceReg *dev) {
    /* Atomic operations on bitfields may generate complex RTL patterns */
    __sync_fetch_and_or(&dev->status, 0x1);
    __sync_fetch_and_and(&dev->status, 0x6);
    
    /* Mixed with regular bitfield operations */
    dev->error = 0;
    COMPILER_BARRIER();
    dev->error = 1;
}

/* Complex function with unpredictable control flow */
void complex_control_flow(struct DeviceReg *dev, int argc, char **argv) {
    /* Use argc to create unpredictable but not eliminable conditions */
    int base_value = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Multiple conditional bitfield assignments */
    for (int i = 0; i < 10; i++) {
        if ((base_value + i) & 1) {
            dev->control = (dev->control + 1) & 0xF;
        }
        
        if ((base_value + i) & 2) {
            dev->data = (dev->data ^ 0x555) & 0xFFF;
        }
        
        if ((base_value + i) & 4) {
            dev->status = (dev->status | 0x4) & 0x7;
        }
        
        /* Insert inline assembly that clobbers registers to force
         * resource tracking analysis */
        if (i % 3 == 0) {
            __asm__ volatile (
                "push %%eax\n\t"
                "movl $0xDEADBEEF, %%eax\n\t"
                "pop %%eax"
                : : : "eax", "memory"
            );
        }
    }
}

/* Main function that creates the necessary context */
int main(int argc, char **argv) {
    /* Stack-allocated struct with bitfields */
    struct DeviceReg local_dev = {0, 0, 0, 0, 0};
    
    /* Heap-allocated struct to ensure different memory context */
    struct DeviceReg *heap_dev = malloc(sizeof(struct DeviceReg));
    heap_dev->control = 0;
    heap_dev->data = 0;
    heap_dev->status = 0;
    heap_dev->error = 0;
    heap_dev->reserved = 0;
    
    /* Call functions that should generate the target RTL patterns */
    
    /* 1. Bitfield assignment through pointer - ZERO_EXTRACT with MEM */
    set_device_control(&local_dev, 0xA);
    set_device_control(heap_dev, 0x5);
    
    /* 2. Multiple bitfield operations in loops */
    update_status_bits(5);
    
    /* 3. Partial register operations - potential STRICT_LOW_PART */
    partial_register_ops();
    
    /* 4. Atomic operations on bitfields */
    atomic_bitfield_ops(&local_dev);
    atomic_bitfield_ops(heap_dev);
    
    /* 5. Complex control flow with bitfields */
    complex_control_flow(&local_dev, argc, argv);
    complex_control_flow(heap_dev, argc, argv);
    
    /* Use the results to prevent dead code elimination */
    int result = local_dev.control + local_dev.data + local_dev.status + local_dev.error;
    result += heap_dev->control + heap_dev->data + heap_dev->status + heap_dev->error;
    result += global_status.ready + global_status.count + global_status.flags;
    
    free(heap_dev);
    
    return result & 0xFF; /* Return non-zero to indicate execution */
}
