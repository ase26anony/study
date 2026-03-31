/* early-remat-test.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early-remat-test early-remat-test.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
const long global_const = 0x123456789ABCDEF0LL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(volatile int *ptr, int val) {
    *ptr = val;
    __asm__ volatile ("" : : "r"(val) : "memory");
}

/* Main test function with high register pressure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int offset = argc > 2 ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent elimination */
    volatile int sink = 0;
    
    /* Outer loop to create many iterations */
    for (int outer = 0; outer < 1000; outer++) {
        /* Create many short-lived intermediate values with different modes */
        
        /* Integer constants (SImode) - expensive to materialize */
        const int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
        const int const2 = 0x80000000;  /* Another large constant */
        const int const3 = 0x12345678;  /* Yet another */
        
        /* Long constants (DImode) */
        const long long_const1 = 0xFEDCBA9876543210LL;
        const long long_const2 = 0x0F0F0F0F0F0F0F0FLL;
        
        /* Pointer constants (Pmode) - symbol addresses */
        int *ptr1 = &global_array[0];
        int *ptr2 = &global_array[64];
        int *ptr3 = &global_array[128];
        
        /* Constants derived from function arguments */
        int derived1 = base + 1;        /* param + 1 */
        int derived2 = base << 2;       /* param << 2 */
        int derived3 = base * 3;        /* param * 3 */
        int derived4 = base - offset;   /* param - offset */
        
        /* Loop-variant values */
        int variant1 = outer * 7;
        int variant2 = outer + base;
        int variant3 = outer ^ offset;
        
        /* Create multiple basic blocks with conditional branches */
        if (outer & 1) {
            /* Branch 1: Use many values */
            int temp1 = const1 + derived1;
            int temp2 = const2 + derived2;
            int temp3 = const3 + derived3;
            
            /* Pointer arithmetic creating more remat candidates */
            int *ptr4 = ptr1 + variant1;
            int *ptr5 = ptr2 + variant2;
            
            /* Mix operations */
            long mixed1 = (long)temp1 * long_const1;
            long mixed2 = (long)temp2 * long_const2;
            
            /* Use values to prevent elimination */
            use_value(&sink, temp1);
            use_value(&sink, temp2);
            use_value(&sink, temp3);
            
            /* More computations */
            int temp4 = variant1 * variant2;
            int temp5 = variant2 * variant3;
            int temp6 = variant3 * variant1;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* More computations after clobber */
            int temp7 = temp4 + temp5;
            int temp8 = temp6 + derived4;
            int temp9 = temp7 * temp8;
            
            /* Store to global to create side effects */
            global_array[outer & 255] = temp9;
            
        } else {
            /* Branch 2: Different computation pattern */
            int temp1 = derived1 - const1;
            int temp2 = derived2 - const2;
            int temp3 = derived3 - const3;
            
            /* Different pointer arithmetic */
            int *ptr4 = ptr3 - variant1;
            int *ptr5 = ptr1 - variant2;
            
            /* Different mix of operations */
            long mixed1 = (long)temp1 / (long_const1 & 0xFFFF);
            long mixed2 = (long)temp2 / (long_const2 & 0xFFFF);
            
            /* Use values */
            use_value(&sink, temp1);
            use_value(&sink, temp2);
            use_value(&sink, temp3);
            
            /* Different computations */
            int temp4 = variant1 + variant2 + variant3;
            int temp5 = variant1 * 3 + variant2 * 5;
            int temp6 = variant3 * 7 + base;
            
            /* Clobber registers */
            clobber_registers();
            
            /* Continue computations */
            int temp7 = temp4 ^ temp5;
            int temp8 = temp6 | derived4;
            int temp9 = temp7 & temp8;
            
            /* Store result */
            global_array[(outer + 128) & 255] = temp9;
        }
        
        /* Third branch outside if-else to create more basic blocks */
        if (outer % 3 == 0) {
            /* More computations with different constants */
            int temp10 = 0x55555555 + outer;  /* Another large constant */
            int temp11 = 0xAAAAAAAA + base;
            int temp12 = 0x33333333 + offset;
            
            /* Pointer computations */
            int *ptr6 = &global_array[temp10 & 255];
            int *ptr7 = &global_array[temp11 & 255];
            
            /* Use them */
            *ptr6 = temp10;
            *ptr7 = temp11;
            
            /* More register pressure */
            long temp13 = (long)temp10 * (long)temp11;
            long temp14 = (long)temp12 * (long)outer;
            long temp15 = temp13 + temp14;
            
            /* Force spill candidates */
            for (int i = 0; i < 4; i++) {
                int spill_temp = temp10 + i * 0x11111111;
                use_value(&sink, spill_temp);
            }
        }
        
        /* Another basic block with switch-like structure */
        switch (outer % 4) {
            case 0: {
                int case0_val = 0x11111111 * outer;
                use_value(&sink, case0_val);
                break;
            }
            case 1: {
                int case1_val = 0x22222222 * base;
                use_value(&sink, case1_val);
                break;
            }
            case 2: {
                int case2_val = 0x33333333 * offset;
                use_value(&sink, case2_val);
                break;
            }
            case 3: {
                int case3_val = 0x44444444 * (outer + base + offset);
                use_value(&sink, case3_val);
                break;
            }
        }
        
        /* Update global volatile to prevent loop elimination */
        global_counter++;
    }
    
    return sink + global_counter;
}
