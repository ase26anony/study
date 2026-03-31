/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int modifier = argc > 2 ? atoi(argv[2]) : 7;
    
    /* Volatile sink to prevent optimization */
    volatile int result_sink = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 1000; outer++) {
        /* Many local variables with different constant expressions */
        /* These become rematerialization candidates */
        
        /* Small integer constants (require multiple instructions) */
        int c1 = 0x7FFFFFFF;  /* Large constant */
        int c2 = -0x80000000; /* Large negative constant */
        long c3 = 0x12345678; /* Different size constant */
        
        /* Constants derived from function arguments */
        int derived1 = base + 1;          /* param + 1 */
        int derived2 = base << 2;         /* param << 2 */
        int derived3 = base * modifier;   /* param * param */
        long derived4 = (long)base * 100; /* Different type */
        
        /* Symbol addresses that can be recomputed */
        int *addr1 = &global_array[0];
        int *addr2 = &global_array[modifier];
        int *addr3 = &global_array[base & 0xFF];
        
        /* More complex expressions */
        int expr1 = (base * 3) / 2 + 1;
        int expr2 = (modifier << 3) | 0xF;
        long expr3 = global_const + outer;
        
        /* Use in conditional branches to create multiple basic blocks */
        if (outer & 1) {
            /* Branch 1 - use some values */
            int temp1 = c1 + derived1;
            int temp2 = expr1 * 2;
            long temp3 = expr3 >> 4;
            
            /* Pointer arithmetic */
            int *ptr1 = addr1 + (temp1 & 0xF);
            int *ptr2 = addr2 + (temp2 & 0x7);
            
            /* Use values to prevent elimination */
            result_sink += *ptr1 + temp2;
            clobber_registers();
            
            /* More computations */
            int chain1 = temp1 + c2;
            int chain2 = chain1 * derived2;
            long chain3 = (long)chain2 + temp3;
            
            /* Store to global to create side effects */
            global_array[outer & 0xFF] = chain1;
            
        } else {
            /* Branch 2 - use different values */
            int temp4 = c2 - derived3;
            int temp5 = expr2 / 3;
            long temp6 = derived4 + global_const;
            
            /* Different pointer arithmetic */
            int *ptr3 = addr3 + (temp4 & 0xF);
            int *ptr4 = &global_array[temp5 & 0xFF];
            
            /* Use values */
            result_sink += *ptr3 - temp5;
            clobber_registers();
            
            /* Different computation chain */
            int chain4 = temp4 * expr1;
            int chain5 = chain4 >> derived2;
            long chain6 = temp6 - chain5;
            
            /* Store to global */
            global_array[(outer + 1) & 0xFF] = chain4;
        }
        
        /* Third basic block (always executed) */
        {
            /* Mix all data types and operations */
            int mix1 = c1 + c2;
            long mix2 = (long)derived1 * derived4;
            int *mix_ptr = addr1 + (mix1 & 0xF);
            
            /* Complex expression with multiple uses */
            int complex1 = (mix1 * 3) + (derived2 / 4);
            int complex2 = complex1 ^ expr2;
            long complex3 = mix2 + complex2;
            
            /* Force register pressure with many live values */
            int live1 = complex1;
            int live2 = complex2;
            long live3 = complex3;
            int *live4 = mix_ptr;
            int live5 = derived3;
            int live6 = expr1;
            long live7 = expr3;
            int live8 = c1;
            int live9 = c2;
            long live10 = c3;
            
            /* Use all live values in a way that prevents optimization */
            result_sink += live1 + live2 + (int)live3;
            result_sink += (int)((long)live4 - (long)addr1);
            result_sink += live5 + live6 + (int)live7;
            result_sink += live8 + live9 + (int)live10;
            
            /* Another clobber to force spills */
            clobber_registers();
            
            /* More computations to extend live ranges */
            int extended1 = live1 * live2;
            long extended2 = live3 + live7;
            int extended3 = live5 - live6;
            
            /* Use extended values */
            result_sink += extended1 + extended3;
            result_sink += (int)extended2;
            
            /* Final store */
            global_counter += result_sink & 1;
        }
        
        /* Additional nested condition to create more BBs */
        if (outer % 3 == 0) {
            int nested1 = derived1 * 7;
            int nested2 = nested1 + 0xABCD;
            long nested3 = (long)nested2 * 0x100;
            
            result_sink += nested1;
            clobber_registers();
            
            /* Pointer with different mode */
            long *long_ptr = (long*)&global_array[outer & 0x7F];
            *long_ptr = nested3;
        }
    }
    
    printf("Result: %d\n", result_sink);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
