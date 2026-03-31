/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

/* Force register pressure and complex addressing modes */
#define NUM_VARS 30
#define ARRAY_SIZE 100

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) { return x; }

/* Complex structure for challenging addressing */
struct nested {
    int a[5][5];
    long b[3];
    volatile int c;
};

struct container {
    struct nested inner[4];
    volatile long offset;
};

/* Test function with many parameters to force register pressure */
__attribute__((noinline, noipa))
long test_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    long b1, long b2, long b3, long b4, long b5,
    volatile int* mem_ptr, struct container* cont,
    int idx1, int idx2, int idx3
) {
    /* Local variables to increase register pressure */
    register int r12_var asm ("r12") = a1;
    register int r13_var asm ("r13") = a2;
    int locals[NUM_VARS];
    volatile int vol_var = 0;
    
    /* Initialize locals with complex dependencies */
    for (int i = 0; i < NUM_VARS; i++) {
        locals[i] = barrier(i + a1 + a2 + a3);
    }
    
    /* Complex arithmetic with many live variables */
    locals[0] = a1 * a2 + a3 - a4;
    locals[1] = a5 ^ a6 | a7 & a8;
    locals[2] = (a9 << 3) | (a10 >> 2);
    locals[3] = b1 + b2 - b3;
    locals[4] = b4 * b5 / (b1 ? b1 : 1);
    
    /* Force spills with many operations */
    for (int i = 5; i < NUM_VARS - 5; i++) {
        locals[i] = locals[i-1] + locals[i-2] - locals[i-3] * locals[i-4];
        locals[i] = barrier(locals[i]);
    }
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[r12], %[tmp1]\n\t"
        "mov %[r13], %[tmp2]\n\t"
        "add %[tmp1], %[tmp2]\n\t"
        : [tmp1] "=r" (locals[10]), [tmp2] "=r" (locals[11])
        : [r12] "r" (r12_var), [r13] "r" (r13_var)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* Complex addressing modes - SIB addressing on x86 */
    int* array = (int*)mem_ptr;
    int complex_addr = 0;
    
    /* Force SIB addressing with all components */
    for (int i = 0; i < 10; i++) {
        /* array[base + index*scale] where all may need reloads */
        int base = idx1;
        int index = idx2 + i;
        int scale = 4;
        
        /* Volatile to prevent optimization */
        vol_var = array[base + index * scale / 4];
        complex_addr += vol_var;
        
        /* Another complex addressing mode */
        complex_addr += cont->inner[i % 4].a[index % 5][base % 5];
    }
    
    /* Mixed register classes - integer to float and back */
    {
        union pun {
            int i;
            float f;
        } u;
        
        u.i = a1 + a2 + a3;
        /* Force move between integer and float registers */
        float temp_f = u.f * 1.5f;
        u.f = temp_f;
        locals[12] = u.i;
    }
    
    /* Atomic operations with complex addressing */
    int atomic_temp = 0;
    __atomic_store(&cont->offset, &b1, __ATOMIC_RELAXED);
    __atomic_load(&cont->offset, &atomic_temp, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure nesting */
    long result = atomic_temp;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            /* cont->inner[i].b[j] with volatile index */
            volatile int vi = i * idx3;
            volatile int vj = j * idx1;
            result += cont->inner[vi % 4].b[vj % 3];
        }
    }
    
    /* Final computation using all variables */
    for (int i = 0; i < NUM_VARS; i++) {
        result += locals[i];
    }
    
    result += complex_addr;
    result += r12_var * r13_var;
    
    return result;
}

/* Secondary reload stressor */
__attribute__((noinline, noipa))
int secondary_reload_test(volatile int* mem, int index) {
    /* This should force secondary reloads on many architectures */
    register int reg_var asm ("ebx") = 0;
    int result;
    
    /* Complex memory address that may need secondary reload */
    asm volatile (
        "movl (%[mem], %[index], 4), %[reg]\n\t"
        "addl $42, %[reg]\n\t"
        "movl %[reg], %[result]\n\t"
        : [reg] "=&r" (reg_var), [result] "=r" (result)
        : [mem] "r" (mem), [index] "r" (index)
        : "memory"
    );
    
    /* Use the register variable in another complex operation */
    asm volatile (
        "imull %[reg], %[result]\n\t"
        : [result] "+r" (result)
        : [reg] "r" (reg_var)
        : "cc"
    );
    
    return result;
}

int main() {
    /* Large set of scalar variables */
    int vars[NUM_VARS];
    long long_vars[10];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < NUM_VARS; i++) {
        vars[i] = barrier(i * 3 + 1);
    }
    for (int i = 0; i < 10; i++) {
        long_vars[i] = barrier(i * 5 + 2);
    }
    
    /* Complex structure */
    struct container cont = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                cont.inner[i].a[j][k] = i * 25 + j * 5 + k;
            }
        }
        for (int j = 0; j < 3; j++) {
            cont.inner[i].b[j] = i * 10 + j;
        }
    }
    
    /* Memory array for complex addressing */
    int mem_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        mem_array[i] = barrier(i * 7 + 3);
    }
    
    /* Call test function with many arguments */
    long result = test_function(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        long_vars[0], long_vars[1], long_vars[2], long_vars[3], long_vars[4],
        mem_array, &cont,
        vars[10] % 50, vars[11] % 50, vars[12] % 4
    );
    
    /* Secondary reload test */
    int sec_result = secondary_reload_test(mem_array, vars[13] % 50);
    result += sec_result;
    
    /* Use result to prevent dead code elimination */
    barrier((int)result);
    
    /* Print checksum */
    printf("Result: %ld\n", result);
    
    return (int)(result % 256);
}
