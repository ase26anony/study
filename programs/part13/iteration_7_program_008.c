/* early-remat-test.c */
#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
int global_array[256] = {0};
static const long static_const = 0x7FFF0000FFFFL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_regs(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) use_value(int x) {
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Main function with high register pressure loop */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int base_offset = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < loop_count; outer++) {
        /* Create many intermediate values with different modes/types */
        
        /* Integer constants (SImode) - remat candidates */
        int const1 = 0x12345678;          /* Large constant requiring multiple insns */
        int const2 = -987654321;          /* Another large constant */
        int const3 = 0x7FFFFFFF;          /* Max positive 32-bit */
        int const4 = 0x80000000;          /* Min negative 32-bit */
        
        /* Long constants (DImode) */
        long const5 = 0x123456789ABCDEF0L;
        long const6 = static_const + outer;  /* Mix with loop variant */
        
        /* Pointer arithmetic (Pmode) - good remat candidates */
        int *ptr1 = &global_array[base_offset + outer];
        int *ptr2 = &global_array[base_offset * 2 - outer];
        int *ptr3 = (int*)((uintptr_t)&global_array[0] + (outer << 2));
        
        /* Derived values from function arguments */
        int derived1 = base_offset + 1;      /* param + 1 */
        int derived2 = base_offset << 2;     /* param << 2 */
        int derived3 = base_offset * 3;      /* param * 3 */
        int derived4 = (base_offset & 0xFF) | 0x100;
        
        /* Loop-variant values */
        int var1 = outer * 7;
        int var2 = outer + global_base;
        int var3 = outer ^ 0x55AA55AA;
        long var4 = (long)outer * 1000000L;
        
        /* Complex expressions mixing constants and variables */
        int expr1 = const1 + var1;
        int expr2 = const2 - var2;
        long expr3 = const5 + var4;
        int expr4 = (derived1 * derived2) >> 1;
        int expr5 = (const3 & const4) | derived3;
        
        /* Multiple basic blocks to increase register pressure */
        if (outer & 1) {
            /* Branch 1 - use some values */
            expr1 = use_value(expr1);
            expr3 = (long)use_value((int)expr3);
            
            /* More intermediate values in this branch */
            int branch1_val1 = ptr1[0] + const1;
            int branch1_val2 = ptr2[1] - const2;
            long branch1_val3 = (long)ptr3[2] * expr3;
            
            sink += branch1_val1 + branch1_val2 + (int)branch1_val3;
            
            /* Nested condition */
            if (outer & 2) {
                int nested_val = derived4 * 3;
                ptr1[0] = nested_val;
                clobber_regs();
            }
        } else {
            /* Branch 2 - use different values */
            expr2 = use_value(expr2);
            expr4 = use_value(expr4);
            
            /* Different computations in this branch */
            int branch2_val1 = (int)const6 + expr2;
            long branch2_val2 = var4 / (expr4 + 1);
            int branch2_val3 = derived1 ^ derived2 ^ derived3;
            
            sink += branch2_val1 + (int)branch2_val2 + branch2_val3;
            
            /* Another nested condition */
            if (outer & 4) {
                long nested_val = expr3 >> 2;
                global_array[outer & 0xFF] = (int)nested_val;
                clobber_regs();
            }
        }
        
        /* Third branch for more complexity */
        if (outer % 3 == 0) {
            /* Mix pointer and integer operations */
            int *ptr4 = ptr1 + (derived1 >> 2);
            int temp1 = *ptr4 + expr5;
            int temp2 = temp1 * const1;
            long temp3 = (long)temp2 * var4;
            
            /* Chain of computations */
            for (int i = 0; i < 3; i++) {
                temp1 = use_value(temp1 + i);
                temp3 = temp3 / (temp2 + 1);
            }
            
            sink += temp1 + (int)temp3;
        }
        
        /* Use all pointer values to keep them live */
        if (ptr1 != ptr2) {
            ptr1[0] = expr1;
            ptr2[1] = expr2;
        }
        
        /* Force register clobbering periodically */
        if (outer % 10 == 0) {
            clobber_regs();
        }
        
        /* Use volatile to prevent dead code elimination */
        sink += const1 + const2 + (int)const5 + (int)const6 + 
                derived1 + derived2 + derived3 + derived4 +
                var1 + var2 + (int)var4 + expr1 + expr2 + 
                (int)expr3 + expr4 + expr5;
    }
    
    return sink;
}
