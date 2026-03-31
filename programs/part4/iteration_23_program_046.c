/* test-caller-save.c - Complex program to trigger GCC's caller-save instruction chain manipulation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque external functions to force call preservation */
extern void opaque_call_1(void) __attribute__((noinline, noclone));
extern void opaque_call_2(int) __attribute__((noinline, noclone));
extern int opaque_call_3(int, int) __attribute__((noinline, noclone));
extern double opaque_call_4(double) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_data[256];
volatile double global_fp[256];

/* Function pointer with volatile to prevent devirtualization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr;

/* Complex function with many live registers across call */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force register pressure with explicit register variables */
    register int r10_val asm("r10") = mode * 2;
    register int r11_val asm("r11") = mode * 3;
    register int r12_val asm("r12") = mode * 4;
    volatile int stack_save[10];
    
    /* Save volatile values that must survive the call */
    asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r12_val));
    
    /* Complex control flow that may split basic blocks */
    if (mode & 1) {
        /* First call site with many live values */
        opaque_call_1();
        
        /* Use saved values immediately after call */
        stack_save[0] = r10_val + r11_val;
        
        /* Inline asm that clobbers call-clobbered registers */
        asm volatile(
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            : 
            : "r"(r10_val), "r"(r11_val)
            : "eax", "memory"
        );
        
        /* Another call with different register pressure */
        if (mode & 2) {
            opaque_call_2(r12_val);
            
            /* Force reload of values */
            r10_val = stack_save[0] ^ r12_val;
        }
    } else {
        /* Alternative path with goto to create irreducible flow */
        int i = 0;
    loop_start:
        if (i++ < 3) {
            /* Call in loop with break */
            opaque_call_3(i, r10_val);
            if (i == 2) {
                /* Break creates block boundary */
                goto loop_end;
            }
            goto loop_start;
        }
    loop_end:
        
        /* Use all register variables in complex expression */
        r11_val = (r10_val * r12_val) / (r11_val + 1);
    }
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
    
    /* Save result to global to prevent DCE */
    global_data[mode & 255] = r10_val + r11_val + r12_val;
}

/* Function with floating point and mixed calls */
__attribute__((noinline, noclone))
double test2(double input, int iterations) {
    volatile double fp_save[8];
    register double fp1 asm("xmm0") = input;
    register double fp2 asm("xmm1") = input * 2.0;
    register double fp3 asm("xmm2") = input * 3.0;
    
    /* Save FP values */
    fp_save[0] = fp1;
    fp_save[1] = fp2;
    fp_save[2] = fp3;
    
    /* Switch with calls in different cases */
    switch (iterations & 3) {
        case 0:
            opaque_call_1();
            /* Use saved FP values */
            fp1 = fp_save[0] + fp_save[1];
            break;
        case 1:
            opaque_call_4(fp1);
            fp2 = fp_save[1] * fp_save[2];
            /* Fall through */
        case 2:
            opaque_call_2((int)fp1);
            fp3 = fp_save[0] - fp_save[2];
            break;
        default:
            /* Complex default with nested call */
            {
                int temp = (int)fp1;
                opaque_call_3(temp, (int)fp2);
                fp1 = fp_save[0];
            }
            /* goto to create block boundary */
            goto merge_point;
    }
    
    /* Another call after switch */
    opaque_call_1();
    
merge_point:
    /* Use all FP registers in computation */
    double result = fp1 + fp2 + fp3;
    
    /* Inline asm that acts like a call */
    asm volatile(
        "call *%0\n\t"
        : 
        : "r"(volatile_fptr)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    return result;
}

/* Function with __builtin_apply to create unusual register pressure */
__attribute__((noinline, noclone))
void test3(void *arg) {
    /* Force many arguments to be live */
    int a1 = global_counter++;
    int a2 = global_counter++;
    int a3 = global_counter++;
    int a4 = global_counter++;
    int a5 = global_counter++;
    double f1 = 1.0;
    double f2 = 2.0;
    
    /* Save to volatile memory */
    volatile int save_area[10];
    save_area[0] = a1;
    save_area[1] = a2;
    save_area[2] = a3;
    save_area[3] = a4;
    save_area[4] = a5;
    
    /* Call with many arguments */
    opaque_call_3(a1, a2);
    
    /* Use saved values in way that requires original registers */
    a3 = save_area[2] + save_area[3];
    
    /* Another call */
    opaque_call_2(a3);
    
    /* Complex expression requiring temporary */
    int complex = (save_area[0] * save_area[1]) / 
                  (save_area[2] + save_area[3] - save_area[4]);
    
    /* Call via function pointer with volatile */
    if (volatile_fptr) {
        volatile_fptr();
    }
    
    /* Use all values to prevent optimization */
    global_data[complex & 255] = a1 + a2 + a3 + a4 + a5;
}

/* Function with nested calls and irreducible control flow */
__attribute__((noinline, noclone))
int test4(int depth) {
    if (depth <= 0) {
        return 1;
    }
    
    /* Many local variables to increase register pressure */
    int v1 = depth * 1;
    int v2 = depth * 2;
    int v3 = depth * 3;
    int v4 = depth * 4;
    int v5 = depth * 5;
    int v6 = depth * 6;
    int v7 = depth * 7;
    int v8 = depth * 8;
    
    /* Save to volatile array */
    volatile int saved[8];
    saved[0] = v1; saved[1] = v2; saved[2] = v3; saved[3] = v4;
    saved[4] = v5; saved[5] = v6; saved[6] = v7; saved[7] = v8;
    
    /* Nested call */
    int result = test4(depth - 1);
    
    /* Use saved values after return */
    v1 = saved[0] + result;
    v2 = saved[1] - result;
    
    /* Call with computed argument */
    opaque_call_3(v1, v2);
    
    /* More computation with saved values */
    for (int i = 0; i < 4; i++) {
        if (i & 1) {
            opaque_call_1();
        } else {
            opaque_call_2(saved[i]);
        }
        
        /* Break/continue to split blocks */
        if (saved[i] > 100) {
            break;
        }
        
        if (saved[i] < 0) {
            continue;
        }
        
        /* Use another saved value */
        saved[i+4] += saved[i];
    }
    
    /* Final computation using all saved values */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += saved[i];
    }
    
    return sum;
}

/* Function with vector types and mixed calls */
#ifdef __SSE2__
#include <emmintrin.h>
__attribute__((noinline, noclone))
void test5(void) {
    /* Vector variables */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    volatile __m128i v_save[4];
    
    /* Save vectors */
    v_save[0] = v1;
    v_save[1] = v2;
    
    /* Call that might clobber vector regs */
    opaque_call_1();
    
    /* Restore and use */
    v1 = _mm_add_epi32(v_save[0], v_save[1]);
    
    /* Another call */
    opaque_call_2(42);
    
    /* More vector ops */
    v2 = _mm_mullo_epi16(v1, _mm_set1_epi16(2));
    
    /* Save to global */
    _mm_store_si128((__m128i*)&global_data[0], v2);
}
#endif

/* Main driver with runtime-dependent control flow */
int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_fptr = (func_ptr_t)opaque_call_1;
    
    /* Parse mode from argv if provided */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_data[i] = i;
        global_fp[i] = i * 0.5;
    }
    
    /* Run all tests in sequence, but with mode-dependent order */
    int test_order[] = {1, 2, 3, 4, 5};
    
    /* Potentially reorder based on input */
    if (mode & 1) {
        test_order[0] = 3;
        test_order[3] = 1;
    }
    
    /* Execute tests */
    for (int i = 0; i < 5; i++) {
        switch (test_order[i]) {
            case 1:
                test1(mode + i);
                break;
            case 2:
                global_fp[i] = test2(global_fp[i], mode + i);
                break;
            case 3:
                test3(&global_counter);
                break;
            case 4:
                global_data[i] = test4(3 + (mode & 3));
                break;
            case 5:
#ifdef __SSE2__
                test5();
#endif
                break;
        }
        
        /* Call between tests to force saves/restores */
        if (i & 1) {
            opaque_call_2(i);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_data[i];
        checksum += (unsigned long long)global_fp[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy definitions to satisfy linker (normally would be in separate file) */
void opaque_call_1(void) {
    asm volatile("" : : : "memory");
}

void opaque_call_2(int x) {
    global_counter += x;
    asm volatile("" : : : "memory");
}

int opaque_call_3(int a, int b) {
    asm volatile("" : : : "memory");
    return a + b;
}

double opaque_call_4(double x) {
    asm volatile("" : : : "memory");
    return x * 2.0;
}
