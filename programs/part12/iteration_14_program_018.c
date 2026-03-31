/* reload_coverage.c - Trigger multiple reload types in GCC's reload pass */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex addressing and prevent optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Global volatile variables to prevent constant propagation */
volatile int g_volatile_idx1 = 7;
volatile int g_volatile_idx2 = 13;
volatile int g_volatile_idx3 = 19;
volatile int g_volatile_stride = 23;
volatile double g_volatile_scale = 2.71828;
volatile long g_volatile_offset1 = 31;
volatile long g_volatile_offset2 = 37;

/* Function to trigger multiple reload types */
NOINLINE static double trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride, volatile double scale,
    volatile long off1, volatile long off2)
{
    /* Create high register pressure with many live values */
    int a[100];          /* Integer array - pressure on integer regs */
    double b[50];        /* Double array - pressure on FP regs */
    int c[75];           /* More integer pressure */
    double d[40];        /* More FP pressure */
    
    /* Various pointer types for complex addressing */
    char *char_ptr;
    int *int_ptr;
    long *long_ptr;
    double *double_ptr;
    
    /* Intermediate pointers for multi-level indirection */
    void **ptr_to_ptr;
    int **int_ptr_ptr;
    
    /* Results that need to stay alive */
    double fp_result = 0.0;
    int int_result = 0;
    long long_result = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 100; i++) a[i] = rand() % 1000;
    for (int i = 0; i < 50; i++) b[i] = (double)(rand() % 1000) / 10.0;
    for (int i = 0; i < 75; i++) c[i] = rand() % 1000;
    for (int i = 0; i < 40; i++) d[i] = (double)(rand() % 1000) / 10.0;
    
    COMPILER_BARRIER();
    
    /* ============================================
       Complex multi-dimensional array access 
       Likely triggers: RELOAD_FOR_INPUT_ADDRESS
       ============================================ */
    for (int i = 0; i < idx1; i++) {
        for (int j = 0; j < idx2; j++) {
            /* Complex addressing: a[i * volatile_stride + j + idx3] */
            int complex_idx = i * stride + j + idx3;
            if (complex_idx >= 0 && complex_idx < 100) {
                /* Mix integer and FP operations */
                double temp = b[j % 50] * scale;
                a[complex_idx] = (int)(temp + 0.5);
                
                /* More complex: c[(i + j) * 2 + idx3 % 10] */
                int another_idx = (i + j) * 2 + (idx3 % 10);
                if (another_idx >= 0 && another_idx < 75) {
                    c[another_idx] = a[complex_idx] * 2;
                }
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* ============================================
       Multi-level pointer indirection
       Likely triggers: RELOAD_FOR_INPADDR_ADDRESS
                      RELOAD_FOR_OUTPUT_ADDRESS
       ============================================ */
    char_ptr = (char *)a;
    int_ptr = (int *)a;
    long_ptr = (long *)a;
    double_ptr = (double *)b;
    
    /* Chain of pointer arithmetic with volatile offsets */
    for (int i = 0; i < 20; i++) {
        /* Complex address calculation requiring temporary register */
        char *addr1 = char_ptr + off1 + i * sizeof(int) * 3;
        int *addr2 = (int *)(addr1 + off2);
        
        if ((void *)addr2 < (void *)&a[100]) {
            /* Store with complex address */
            *addr2 = i * 100 + idx1;
            
            /* Load with different complex address */
            int_ptr = (int *)(char_ptr + off2 + i * sizeof(int) * 2);
            if ((void *)int_ptr < (void *)&a[100]) {
                int_result += *int_ptr;
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* ============================================
       Inline assembly with register constraints
       Likely triggers: RELOAD_FOR_OPERAND_ADDRESS
                      RELOAD_FOR_OUTADDR_ADDRESS
       ============================================ */
    int asm_input = idx1 * idx2;
    int asm_output;
    long asm_addr_input;
    
    /* Assembly that takes memory address as input */
    asm_addr_input = (long)&a[idx1 % 50];
    
    __asm__ volatile (
        /* Input: memory address in register */
        /* Output: computed value */
        "movq %[addr], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "imull %[in], %%ebx\n\t"
        "movl %%ebx, %[out]\n\t"
        : [out] "=r" (asm_output)      /* Output in register */
        : [addr] "r" (asm_addr_input), /* Address in register */
          [in] "r" (asm_input)         /* Input in register */
        : "rax", "rbx", "memory", "cc"
    );
    
    /* Another assembly with different constraints */
    double asm_fp_input = b[idx2 % 50];
    double asm_fp_output;
    
    __asm__ volatile (
        /* Force address reload for FP operand */
        "movsd %[in], %%xmm0\n\t"
        "mulsd %[scale], %%xmm0\n\t"
        "movsd %%xmm0, %[out]\n\t"
        : [out] "=m" (asm_fp_output)   /* Output in memory */
        : [in] "m" (asm_fp_input),     /* Input in memory */
          [scale] "m" (scale)          /* Scale in memory */
        : "xmm0", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ============================================
       Mixed register class operations
       Likely triggers: RELOAD_OTHER
                      RELOAD_FOR_OTHER_ADDRESS
       ============================================ */
    /* Integer to FP conversion and back */
    for (int i = 0; i < 30; i++) {
        /* Use all local arrays to keep them live */
        int int_val = a[i] + c[i % 75];
        double fp_val = b[i % 50] + d[i % 40];
        
        /* Convert and mix types */
        fp_result += (double)int_val * fp_val;
        int_result += (int)(fp_val * 100.0);
        
        /* Complex addressing with type mixing */
        double *fp_addr = &b[(int_val + i) % 50];
        int *int_addr = &c[((int)(*fp_addr) + i) % 75];
        
        *int_addr = (int)(*fp_addr * scale);
        *fp_addr = (double)(*int_addr) / scale;
    }
    
    COMPILER_BARRIER();
    
    /* ============================================
       Pointer-to-pointer indirection
       Likely triggers: RELOAD_FOR_OPADDR_ADDR
       ============================================ */
    /* Create pointer chains */
    int *temp_ptrs[10];
    for (int i = 0; i < 10; i++) {
        temp_ptrs[i] = &a[i * 10 + idx1 % 10];
    }
    
    int_ptr_ptr = &temp_ptrs[idx2 % 10];
    ptr_to_ptr = (void **)int_ptr_ptr;
    
    /* Multi-level dereference with volatile offsets */
    for (int i = 0; i < 5; i++) {
        int **pptr = (int **)((char *)ptr_to_ptr + off1 + i * sizeof(void *));
        if (pptr >= &temp_ptrs[0] && pptr <= &temp_ptrs[9]) {
            int *deref_ptr = *pptr;
            if (deref_ptr >= &a[0] && deref_ptr <= &a[99]) {
                *deref_ptr = *deref_ptr + idx3 + (int)off2;
                
                /* Another level */
                int *next_ptr = *(int **)((char *)pptr + off2 % 16);
                if (next_ptr && next_ptr >= &a[0] && next_ptr <= &a[99]) {
                    long_result += *next_ptr;
                }
            }
        }
    }
    
    COMPILER_BARRIER();
    
    /* Final computation using all results */
    fp_result += (double)int_result / 1000.0;
    fp_result += (double)long_result / 10000.0;
    fp_result += asm_fp_output;
    fp_result += (double)asm_output / 100.0;
    
    /* Use volatile parameter to prevent dead code elimination */
    return fp_result * (1.0 + (double)(idx1 + idx2 + idx3) / 1000.0);
}

int main(void)
{
    srand(time(NULL));
    
    /* Initialize with random values */
    g_volatile_idx1 = rand() % 50 + 1;
    g_volatile_idx2 = rand() % 50 + 1;
    g_volatile_idx3 = rand() % 50 + 1;
    g_volatile_stride = rand() % 20 + 5;
    g_volatile_scale = (double)(rand() % 100) / 10.0 + 1.0;
    g_volatile_offset1 = rand() % 100;
    g_volatile_offset2 = rand() % 100;
    
    /* Call the function that triggers reloads */
    double result = trigger_reloads(
        g_volatile_idx1,
        g_volatile_idx2,
        g_volatile_idx3,
        g_volatile_stride,
        g_volatile_scale,
        g_volatile_offset1,
        g_volatile_offset2
    );
    
    printf("Result: %f\n", result);
    printf("Indices: %d, %d, %d\n", 
           g_volatile_idx1, g_volatile_idx2, g_volatile_idx3);
    
    return 0;
}
