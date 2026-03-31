/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will be called */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : "memory");
}

/* Function with inline assembly to clobber specific registers */
void __attribute__((noinline)) clobber_many_regs(int *p) {
    /* Clobber multiple call-clobbered registers */
    asm volatile("" : : "r"(*p) : 
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
}

/* Volatile variables to prevent optimization */
volatile int trigger = 0;

/* Main test function with complex register usage */
int __attribute__((noinline)) test_function(int *data, int size) {
    /* Use explicit register variables for call-clobbered registers */
    register long r10 asm("r10") = data[0];
    register long r11 asm("r11") = data[1];
    register long r8 asm("r8") = data[2];
    register long r9 asm("r9") = data[3];
    register long rcx asm("rcx") = data[4];
    register long rdx asm("rdx") = data[5];
    
    /* Also use call-saved registers */
    register long r12 asm("r12") = data[6];
    register long r13 asm("r13") = data[7];
    register long r14 asm("r14") = data[8];
    register long r15 asm("r15") = data[9];
    
    /* Function pointer array */
    void (*funcs[3])(int *) = {external_func1, external_func2, external_func3};
    
    /* Complex nested loop with function calls */
    int result = 0;
    for (int outer = 0; outer < 3; ++outer) {
        /* Force values to stay in registers across calls */
        for (int i = 0; i < size; ++i) {
            /* Mix of arithmetic operations keeping values live */
            r10 = (r10 * 1103515245 + 12345) & 0x7fffffff;
            r11 = (r11 * 1103515245 + 12345) & 0x7fffffff;
            r8 = (r8 * 1103515245 + 12345) & 0x7fffffff;
            r9 = (r9 * 1103515245 + 12345) & 0x7fffffff;
            
            /* Use call-saved registers too */
            r12 = r12 ^ data[i];
            r13 = r13 + data[i];
            r14 = r14 - data[i];
            r15 = r15 * (data[i] | 1);
            
            /* Inline assembly to create artificial dependencies */
            asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r8), "+r"(r9) : :);
            asm volatile("" : "+r"(r12), "+r"(r13), "+r"(r14), "+r"(r15) : :);
            
            /* Call external function - values must be saved/restored */
            if (trigger & 1) {
                clobber_many_regs(&data[i]);
            } else {
                /* Indirect call via function pointer */
                int idx = (r10 + i) % 3;
                funcs[idx](&data[i]);
            }
            
            /* More operations after call - values must be restored */
            rcx = r10 + r11;
            rdx = r8 * r9;
            
            /* Complex expression using all registers */
            int temp = (r10 + r11 + r8 + r9 + rcx + rdx) & 0xff;
            temp = (temp ^ r12 ^ r13 ^ r14 ^ r15) & 0xff;
            
            result += temp;
            data[i] = temp;
            
            /* Volatile memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Switch between direct and indirect calls */
        trigger ^= 1;
    }
    
    /* Final computation using all register values */
    result += r10 + r11 + r8 + r9 + rcx + rdx;
    result += r12 + r13 + r14 + r15;
    
    return result;
}

/* Another test with different pattern */
int __attribute__((noinline)) test_function2(int *data, int size) {
    int sum = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Load multiple values into registers */
        register int a asm("r10") = data[i];
        register int b asm("r11") = data[i + 1];
        register int c asm("r8") = data[i + 2];
        register int d asm("r9") = data[i + 3];
        
        /* Do computation */
        a = a * 3 + 7;
        b = b * 5 - 11;
        c = c * 13 + 17;
        d = d * 19 - 23;
        
        /* Force them to be live across call */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) : :);
        
        /* Call that clobbers registers */
        if (i % 8 == 0) {
            clobber_many_regs(&data[i]);
        } else {
            external_func1(&data[i]);
        }
        
        /* Use values after call */
        sum += a + b + c + d;
        
        /* Store back */
        data[i] = a;
        data[i + 1] = b;
        data[i + 2] = c;
        data[i + 3] = d;
    }
    
    return sum;
}

int main() {
    /* Initialize data array */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i * 1103515245 + 12345;
    }
    
    /* Run first test */
    int result1 = test_function(data, 256);
    printf("Result 1: %d\n", result1);
    
    /* Re-initialize */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * 6364136223846793005ULL + 1442695040888963407ULL) & 0x7fffffff;
    }
    
    /* Run second test */
    int result2 = test_function2(data, 256);
    printf("Result 2: %d\n", result2);
    
    /* Final check */
    int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += data[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    return (result1 + result2 + final_sum) != 0 ? 0 : 1;
}
