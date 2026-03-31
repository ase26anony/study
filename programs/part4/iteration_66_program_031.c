/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_index1 = 10;
volatile int g_index2 = 20;
volatile int g_offset = 5;

/* Complex nested data structures */
struct Inner {
    int member_array[100];
    volatile int* volatile_ptr;
};

struct Outer {
    struct Inner inner_struct[50];
    int outer_array[200];
    volatile long dynamic_offset;
};

/* Global test data */
struct Outer g_nested_array[10];
int g_global_array[1000];
volatile int* g_base_ptr = g_global_array;

/* Function to force address computation before call */
void __attribute__((noinline)) 
use_complex_address(volatile int* addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_input_address(void) {
    volatile int result = 0;
    int i, j;
    
    /* Complex addressing that requires input address reload */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 5; j++) {
            /* Multiple register values in address computation */
            int idx = (i * g_index1 + j * g_index2) >> 1;
            
            /* Inline asm with input memory operand at complex address */
            asm volatile(
                "addl %%ecx, %%eax\n\t"
                : "=a"(result)
                : "a"(result), 
                  "m"(g_nested_array[i].inner_struct[j].member_array[idx]), /* Input address reload */
                  "c"(g_nested_array[i].inner_struct[j].member_array[idx])
                : "memory"
            );
        }
    }
    
    return result;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline,optimize("O2")))
test_output_address(int iterations) {
    volatile int temp;
    int i;
    
    /* Force output to memory at computed addresses */
    for (i = 0; i < iterations; i++) {
        /* Complex output address computation */
        int out_idx = (i << 2) + (int)g_nested_array[0].dynamic_offset;
        
        /* Inline asm with output memory operand */
        asm volatile(
            "movl %%eax, %0\n\t"
            : "=m"(g_global_array[out_idx])  /* Output address reload */
            : "a"(i * 100)
            : "memory"
        );
        
        /* Mix with input to create register pressure */
        asm volatile(
            "imull %%ecx, %%eax\n\t"
            : "+a"(temp)
            : "c"(g_global_array[out_idx]),  /* Input from same address */
              "m"(g_global_array[out_idx])   /* Input memory constraint */
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void __attribute__((noinline,optimize("O2")))
test_operand_address(void) {
    int i;
    
    for (i = 0; i < 20; i++) {
        /* Complex address expression passed to function */
        /* This forces operand address computation before call */
        use_complex_address(
            &g_nested_array[i % 5].inner_struct[i % 3]
                .member_array[(i * g_index1 + g_index2) % 50]
        );
        
        /* Another complex address computation */
        volatile int* addr = &g_global_array[
            (i << 3) + 
            (int)g_nested_array[1].dynamic_offset + 
            g_offset
        ];
        
        /* Use in inline asm with multiple constraints */
        asm volatile(
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m"(*addr)                    /* Memory output */
            : "r"(addr),                     /* Address in register */
              "m"(*addr)                     /* Memory input */
            : "eax", "memory"
        );
    }
}

/* Test RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_mixed_input_address(void) {
    volatile int sum = 0;
    int i, j, k;
    
    /* Triple nested addressing */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 3; k++) {
                /* Extremely complex address computation */
                int idx = (i * 100 + j * 25 + k * 8) 
                         + (int)g_nested_array[i].dynamic_offset
                         + g_offset;
                
                /* Multiple memory inputs with different addressing */
                asm volatile(
                    "movl %1, %%eax\n\t"
                    "addl %2, %%eax\n\t"
                    "addl %%eax, %0\n\t"
                    : "+m"(sum)
                    : "m"(g_nested_array[i].inner_struct[j].member_array[idx]),
                      "m"(g_nested_array[k].outer_array[idx % 200]),
                      "m"(g_global_array[(idx + g_index1) % 1000])
                    : "eax", "memory"
                );
            }
        }
    }
    
    return sum;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void __attribute__((noinline,optimize("O2")))
test_mixed_output_address(int seed) {
    int i;
    volatile int base = seed;
    
    /* Chain output addresses */
    for (i = 0; i < 15; i++) {
        /* Output address depends on previous computation */
        int out_addr1 = (base + i * 7) % 500;
        int out_addr2 = (out_addr1 * 3 + g_index2) % 500;
        
        /* Multiple outputs with complex addresses */
        asm volatile(
            "movl %%eax, %0\n\t"
            "leal (%%eax, %%ecx, 2), %%edx\n\t"
            "movl %%edx, %1\n\t"
            : "=m"(g_global_array[out_addr1]),   /* Output address reload */
              "=m"(g_global_array[out_addr2])    /* Another output address */
            : "a"(base + i * 100),
              "c"(g_index1)
            : "edx", "memory"
        );
        
        /* Update base using memory output */
        asm volatile(
            "addl %1, %0\n\t"
            : "+m"(base)
            : "m"(g_global_array[out_addr1])
            : "memory"
        );
    }
}

/* Test RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
int __attribute__((noinline,optimize("O2")))
test_other_reloads(void) {
    volatile int total = 0;
    int i;
    
    /* Unusual addressing patterns */
    for (i = 0; i < 25; i++) {
        /* Address computation with bit operations */
        int complex_idx = ((i & 0xF) << 4) | ((i >> 4) & 0xF);
        complex_idx = (complex_idx * g_index1) ^ g_index2;
        
        /* Mixed register/memory operations */
        asm volatile(
            "movl %1, %%ebx\n\t"
            "movl %2, %%ecx\n\t"
            "xorl %%ebx, %%ecx\n\t"
            "addl %%ecx, %0\n\t"
            "movl %%ecx, %3\n\t"
            : "+m"(total),
              "+m"(g_global_array[complex_idx % 500])
            : "m"(g_nested_array[i % 3].outer_array[complex_idx % 200]),
              "m"(g_global_array[(complex_idx + 100) % 500])
            : "ebx", "ecx", "memory"
        );
    }
    
    return total;
}

/* Main driver */
int main(void) {
    int i, checksum = 0;
    
    /* Initialize test data */
    for (i = 0; i < 10; i++) {
        g_nested_array[i].dynamic_offset = i * 7;
        for (int j = 0; j < 50; j++) {
            for (int k = 0; k < 100; k++) {
                g_nested_array[i].inner_struct[j].member_array[k] = i + j + k;
            }
        }
        for (int j = 0; j < 200; j++) {
            g_nested_array[i].outer_array[j] = i * j;
        }
    }
    
    for (i = 0; i < 1000; i++) {
        g_global_array[i] = i;
    }
    
    /* Run tests to trigger different reload types */
    checksum += test_input_address();
    
    test_output_address(12);
    checksum += g_global_array[50];
    
    test_operand_address();
    checksum += g_global_array[100];
    
    checksum += test_mixed_input_address();
    
    test_mixed_output_address(42);
    checksum += g_global_array[200];
    
    checksum += test_other_reloads();
    
    /* Final computation to prevent dead code elimination */
    for (i = 0; i < 50; i++) {
        checksum += g_nested_array[i % 3].inner_struct[i % 4]
                   .member_array[(i * g_index1) % 100];
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
