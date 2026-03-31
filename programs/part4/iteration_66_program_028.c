/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 100;
volatile int g_index2 = 200;
volatile int g_base_offset = 300;

/* Complex data structures to force address computations */
struct Inner {
    int data[16];
    int extra;
};

struct Outer {
    struct Inner arrays[8];
    int padding[4];
};

struct Nested {
    struct Outer levels[4];
    int metadata[8];
};

/* Global test data */
struct Nested g_nested_array[10];
int g_simple_array[1024];
int* g_dynamic_ptr;

/* Helper to force address computation before call */
__attribute__((noinline))
void use_complex_address(int* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
__attribute__((noinline))
void test_input_address(void) {
    int i, j, k;
    int sum = 0;
    
    /* Force register pressure with many live variables */
    register int r1 asm("r12") = g_index1;
    register int r2 asm("r13") = g_index2;
    register int r3 asm("r14") = g_base_offset;
    
    /* Complex addressing that requires input address reloads */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 2; k++) {
                /* Multiple index computations in address */
                int idx = (r1 + i) * 2 + (r2 + j) * 3 + (r3 + k);
                
                /* Inline asm with memory input constraint and complex address */
                asm volatile(
                    "addl %%ecx, %%eax\n\t"
                    : "+a"(sum)
                    : "m"(g_simple_array[(idx << 2) + (i * j * k)]), 
                      "c"(i + j + k)
                    : "memory"
                );
            }
        }
    }
    
    /* Another pattern with structure member access */
    struct Outer* outer_ptr = &g_nested_array[2].levels[1];
    int offset = r1 + r2;
    
    asm volatile(
        "movl (%%rbx, %%rcx, 4), %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        : "+a"(sum)
        : "b"(outer_ptr), 
          "c"(offset),
          "m"(outer_ptr->arrays[offset].data[offset % 8])
        : "rdx", "memory"
    );
    
    printf("Input address test sum: %d\n", sum);
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
__attribute__((noinline))
void test_output_address(void) {
    int i;
    volatile int* volatile_ptr = g_simple_array;
    
    /* Force output to memory with computed address */
    for (i = 0; i < 8; i++) {
        int offset = (g_index1 * i + g_index2) % 256;
        
        /* Inline asm with memory output constraint */
        asm volatile(
            "movl %%eax, (%%rbx, %%rcx, 4)\n\t"
            : "=m"(volatile_ptr[offset])
            : "a"(i * 100), "b"(volatile_ptr), "c"(offset)
            : "memory"
        );
    }
    
    /* Complex output address with structure */
    struct Inner* inner_ptr = &g_nested_array[3].levels[2].arrays[1];
    int idx = g_index1 + g_index2;
    
    asm volatile(
        "imull $77, %%eax, %%eax\n\t"
        "movl %%eax, (%%rbx, %%rcx, 8)\n\t"
        : "=m"(inner_ptr->data[idx % 16])
        : "a"(idx), "b"(inner_ptr), "c"(idx % 16)
        : "memory"
    );
    
    printf("Output address test completed\n");
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
__attribute__((noinline))
void test_operand_address(void) {
    int i;
    
    /* Force operand address reloads through function calls */
    for (i = 0; i < 5; i++) {
        /* Complex address expression as function argument */
        use_complex_address(
            &g_nested_array[i].levels[i % 4].arrays[i % 2].data[
                (g_index1 * i + g_index2) % 16
            ]
        );
    }
    
    /* Mixed input/output with operand addresses */
    int* ptr_array[10];
    for (i = 0; i < 10; i++) {
        ptr_array[i] = &g_simple_array[i * 64];
    }
    
    register int* rptr asm("r15") = ptr_array[3];
    int offset = g_index1;
    
    /* This should trigger operand address reloads */
    asm volatile(
        "leaq (%%r15, %%rax, 4), %%rbx\n\t"
        "movl (%%rbx), %%ecx\n\t"
        "addl $1, %%ecx\n\t"
        "movl %%ecx, (%%rbx)\n\t"
        : "+m"(*rptr)
        : "a"(offset), "r"(rptr)
        : "rbx", "rcx", "memory"
    );
    
    printf("Operand address test completed\n");
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
__attribute__((noinline))
void test_inpaddr_outaddr(void) {
    int i, j;
    int buffer[64];
    
    /* Create addressing chains */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            int* src = &g_simple_array[(g_index1 + i) * 8 + j];
            int* dst = &buffer[(g_index2 + j) * 4 + i];
            
            /* Complex memory-to-memory with address computations */
            asm volatile(
                "movl (%%rbx), %%eax\n\t"
                "addl $42, %%eax\n\t"
                "movl %%eax, (%%rcx)\n\t"
                : "=m"(*dst)
                : "m"(*src), "b"(src), "c"(dst)
                : "rax", "memory"
            );
        }
    }
    
    /* Nested addressing with pointer arithmetic */
    struct Outer* outer = &g_nested_array[5].levels[2];
    int* base1 = outer->arrays[0].data;
    int* base2 = outer->arrays[3].data;
    
    for (i = 0; i < 8; i++) {
        int idx1 = (g_index1 * i) % 16;
        int idx2 = (g_index2 * i) % 16;
        
        asm volatile(
            "movl (%%rbx, %%rax, 4), %%edx\n\t"
            "movl %%edx, (%%rcx, %%rsi, 4)\n\t"
            : "=m"(base2[idx2])
            : "m"(base1[idx1]), "b"(base1), "c"(base2),
              "a"(idx1), "S"(idx2)
            : "rdx", "memory"
        );
    }
    
    printf("Inpaddr/Outaddr test completed\n");
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
__attribute__((noinline))
void test_other_address(void) {
    int i;
    volatile int result = 0;
    
    /* Mixed operations that don't fit clean categories */
    for (i = 0; i < 10; i++) {
        /* Multiple memory references in different addressing modes */
        asm volatile(
            "movl (%%rbx), %%eax\n\t"
            "addl (%%rcx, %%rdx, 4), %%eax\n\t"
            "movl %%eax, (%%rsi)\n\t"
            "addl $1, (%%rdi)\n\t"
            : 
            : "b"(&g_index1), 
              "c"(g_simple_array), 
              "d"(i),
              "S"(&g_simple_array[i * 32]),
              "D"(&g_nested_array[i % 5].levels[0].arrays[0].data[0])
            : "rax", "memory"
        );
    }
    
    /* Unusual addressing pattern */
    int (*func_ptr)(void) = (int (*)(void))&test_input_address;
    uintptr_t code_addr = (uintptr_t)func_ptr;
    
    asm volatile(
        "leaq 0x100(%%rax), %%rbx\n\t"
        "movq %%rbx, %0\n\t"
        : "=m"(code_addr)
        : "a"(code_addr)
        : "rbx", "memory"
    );
    
    printf("Other address test completed\n");
}

/* Mixed test combining multiple patterns */
__attribute__((noinline))
int test_mixed_reloads(void) {
    int i, total = 0;
    
    /* Initialize data */
    for (i = 0; i < 1024; i++) {
        g_simple_array[i] = i;
    }
    
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                for (int l = 0; l < 16; l++) {
                    g_nested_array[i].levels[j].arrays[k].data[l] = 
                        i * 1000 + j * 100 + k * 10 + l;
                }
            }
        }
    }
    
    /* Run all tests */
    test_input_address();
    test_output_address();
    test_operand_address();
    test_inpaddr_outaddr();
    test_other_address();
    
    /* Final computation using all addressing modes */
    for (i = 0; i < 100; i++) {
        int idx1 = (g_index1 + i) % 10;
        int idx2 = (g_index2 + i) % 4;
        int idx3 = (i * 7) % 8;
        int idx4 = (i * 11) % 16;
        
        total += g_nested_array[idx1].levels[idx2].arrays[idx3].data[idx4];
        
        /* Force more reloads with inline asm */
        asm volatile(
            "addl (%%rbx, %%rcx, 4), %%eax\n\t"
            : "+a"(total)
            : "b"(g_simple_array), 
              "c"((i * g_index1) % 1024),
              "m"(g_simple_array[(i * g_index1) % 1024])
            : "memory"
        );
    }
    
    return total;
}

int main(void) {
    int result;
    
    printf("Starting reload coverage test...\n");
    
    /* Allocate dynamic memory to increase register pressure */
    g_dynamic_ptr = (int*)malloc(4096 * sizeof(int));
    if (!g_dynamic_ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Run the comprehensive test */
    result = test_mixed_reloads();
    
    printf("Final checksum: %d\n", result);
    printf("Test completed successfully\n");
    
    free(g_dynamic_ptr);
    return 0;
}
