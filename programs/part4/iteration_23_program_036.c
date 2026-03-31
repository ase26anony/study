/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction chain manipulation code at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;
volatile void* global_pointer = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Complex structure to force register pressure */
struct RegPressure {
    long a, b, c, d, e, f, g, h;
    double x, y, z;
    void* p;
};

/* Force many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Use explicit register variables to create conflicts */
    register int r10_val asm("r10") = mode * 2;
    register int r11_val asm("r11") = mode * 3;
    register int r12_val asm("r12") = mode * 4;
    register int r13_val asm("r13") = mode * 5;
    
    volatile int stack_slot1 = r10_val;
    volatile int stack_slot2 = r11_val;
    volatile int stack_slot3 = r12_val;
    volatile int stack_slot4 = r13_val;
    
    /* Create irreducible control flow with goto */
    if (mode & 1) goto label1;
    if (mode & 2) goto label2;
    
    /* Force register saves before call */
    asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r12_val), "r"(r13_val) : "memory");
    
    /* Call that clobbers registers */
    asm volatile(
        "movl $0, %%eax\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        "movl $0, %%esi\n\t"
        "movl $0, %%edi\n\t"
        "movl $0, %%r8d\n\t"
        "movl $0, %%r9d\n\t"
        "movl $0, %%r10d\n\t"
        "movl $0, %%r11d\n\t"
        : : : "eax", "ecx", "edx", "esi", "edi", "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Use saved values after call - forces reload to insert restore instructions */
    int sum = stack_slot1 + stack_slot2 + stack_slot3 + stack_slot4;
    
    /* Complex expression requiring temporary registers */
    sum = (sum * r10_val) / (r11_val + 1) + (r12_val << 2) - (r13_val >> 1);
    
    global_counter += sum;
    
    /* More control flow complexity */
    if (sum > 100) {
        opaque_func1();
        goto label1;
    }
    
label2:
    /* Nested call scenario */
    {
        int temp = opaque_func2(sum);
        asm volatile("" : "+r"(temp) : : "memory");
        global_counter += temp;
    }
    return;
    
label1:
    /* Different path with another call */
    double d = opaque_func3(sum);
    asm volatile("" : "+t"(d) : : "memory");
    global_accumulator += d;
}

/* Test with floating point and mixed types */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double save1, save2, save3, save4;
    
    /* Use many FP registers */
    register double fp1 asm("xmm0") = base;
    register double fp2 asm("xmm1") = base * 2.0;
    register double fp3 asm("xmm2") = base * 3.0;
    register double fp4 asm("xmm4") = base * 4.0;
    register double fp5 asm("xmm5") = base * 5.0;
    
    /* Save to volatile stack slots */
    save1 = fp1;
    save2 = fp2;
    save3 = fp3;
    save4 = fp4;
    
    /* Force all FP registers live */
    asm volatile("" : : "x"(fp1), "x"(fp2), "x"(fp3), "x"(fp4), "x"(fp5) : "memory");
    
    /* Call that clobbers FP registers */
    asm volatile(
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
    );
    
    /* Switch statement creating complex CFG */
    int choice = (int)base % 4;
    switch (choice) {
        case 0:
            /* Use saved values - forces restore insertion */
            fp1 = save1 + save2;
            break;
        case 1:
            fp1 = save2 * save3;
            break;
        case 2:
            fp1 = save3 - save4;
            break;
        default:
            /* Call in default case */
            opaque_func1();
            fp1 = save1 + save2 + save3 + save4;
            break;
    }
    
    /* Loop with break that creates block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i > (int)fp1) {
            /* Function call at block boundary */
            opaque_func2(i);
            break;
        }
        fp1 += 0.5;
    }
    
    global_accumulator += fp1 + fp5;
}

/* Test with vector types and __builtin_apply */
__attribute__((noinline, noclone))
void test3(void* ptr) {
    /* Use __builtin_apply to create unusual call sequences */
    typedef void* (*apply_func_t)(void*, void*, void*, void*);
    
    /* Many arguments to force register pressure */
    long arg1 = 0x12345678;
    long arg2 = 0x87654321;
    long arg3 = 0xABCDEF01;
    long arg4 = 0xFEDCBA98;
    
    volatile long saved_args[8];
    saved_args[0] = arg1;
    saved_args[1] = arg2;
    saved_args[2] = arg3;
    saved_args[3] = arg4;
    
    /* Force arguments to be in specific registers */
    register long rdi_val asm("rdi") = arg1;
    register long rsi_val asm("rsi") = arg2;
    register long rdx_val asm("rdx") = arg3;
    register long rcx_val asm("rcx") = arg4;
    register long r8_val asm("r8") = (long)ptr;
    register long r9_val asm("r9") = global_counter;
    
    asm volatile("" : : "r"(rdi_val), "r"(rsi_val), "r"(rdx_val), "r"(rcx_val),
                       "r"(r8_val), "r"(r9_val) : "memory");
    
    /* Create a situation where BB_END might need updating */
    void* result = NULL;
    
    /* Complex control flow with labels */
    if ((long)ptr & 1) {
        goto complex_path;
    }
    
    /* Direct call */
    result = opaque_func4(ptr);
    
    /* Use saved values after call */
    arg1 = saved_args[0] + (long)result;
    goto merge_point;
    
complex_path:
    /* Indirect call via function pointer */
    if (func_table[0]) {
        func_table[0]();
    }
    
    /* More register pressure */
    asm volatile(
        "movq $0, %%rax\n\t"
        "movq $0, %%rbx\n\t"
        : : : "rax", "rbx", "memory"
    );
    
    arg1 = saved_args[1] * saved_args[2];
    
merge_point:
    /* Instruction that might be at BB_END */
    global_pointer = (void*)(arg1 + arg2 + arg3 + arg4);
    
    /* Nested loop with function calls */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                opaque_func2(i + j);
                continue;
            }
            arg1 += i * j;
        }
        /* Break creates block boundary */
        if (i == 2) break;
    }
}

/* Function with mixed calling conventions and varargs-like behavior */
__attribute__((noinline, noclone))
void test4(int count, ...) {
    /* Simulate va_arg usage */
    volatile long reg_save_area[10];
    
    /* Use many registers */
    register long rax_save asm("rax") = count * 10;
    register long rbx_save asm("rbx") = count * 20;
    register long rcx_save asm("rcx") = count * 30;
    
    reg_save_area[0] = rax_save;
    reg_save_area[1] = rbx_save;
    reg_save_area[2] = rcx_save;
    
    /* Force them to be live */
    asm volatile("" : : "r"(rax_save), "r"(rbx_save), "r"(rcx_save) : "memory");
    
    /* Inline asm that looks like a call */
    asm volatile(
        "pushq %%rbp\n\t"
        "movq %%rsp, %%rbp\n\t"
        "subq $32, %%rsp\n\t"
        "call *%0\n\t"
        "addq $32, %%rsp\n\t"
        "popq %%rbp\n\t"
        : : "r"(opaque_func1) : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    /* Use saved values - will need restores */
    long total = reg_save_area[0] + reg_save_area[1] + reg_save_area[2];
    
    /* Unpredictable control flow */
    switch (total % 5) {
        case 0: goto case0;
        case 1: goto case1;
        case 2: goto case2;
        case 3: goto case3;
        default: goto default_case;
    }
    
case0:
    opaque_func2(total);
    goto end;
case1:
    total += opaque_func2(total * 2);
    goto end;
case2:
    /* Nested call in middle of block */
    {
        int temp = opaque_func2(total);
        total += temp;
        opaque_func1();
    }
    goto end;
case3:
    for (int i = 0; i < total % 10; i++) {
        if (i == 5) {
            opaque_func1();
            break;
        }
        total += i;
    }
    goto end;
default_case:
    total = opaque_func2(total) * 3;
    /* fall through */
end:
    global_counter += total;
}

/* Helper with nested calls to create save/restore chains */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    if (depth <= 0) {
        return opaque_func2(val);
    }
    
    volatile int save = val;
    
    /* Recursive call */
    int result = nested_helper(depth - 1, val * 2);
    
    /* Use saved value after recursive call */
    result += save;
    
    /* Another call */
    result += opaque_func2(result);
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize function table with opaque functions */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to create runtime-dependent control flow */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Force different compilation paths */
    switch (mode) {
        case 0:
            test1(10);
            test2(3.14159);
            test3(&global_counter);
            test4(4, 1, 2, 3, 4);
            break;
        case 1:
            test2(2.71828);
            test1(20);
            test4(3, 5, 6, 7);
            test3(&global_accumulator);
            break;
        case 2:
            test3(NULL);
            test4(5, 10, 20, 30, 40, 50);
            test1(30);
            test2(1.41421);
            break;
        case 3:
            /* Test nested calls */
            for (int i = 0; i < 3; i++) {
                int result = nested_helper(i, mode * 10);
                global_counter += result;
                opaque_func1();
            }
            test1(40);
            test2(0.57721);
            break;
        default:
            /* Execute all tests */
            test1(50);
            test2(1.61803);
            test3(&global_pointer);
            test4(6, 100, 200, 300, 400, 500, 600);
            for (int i = 0; i < 2; i++) {
                nested_helper(2, i * 100);
            }
            break;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = (long)global_counter + 
                   (long)global_accumulator + 
                   (long)global_pointer;
    
    /* Use checksum in opaque way */
    asm volatile("" : : "r"(checksum) : "memory");
    
    printf("Result: %ld\n", checksum);
    
    return (int)(checksum % 256);
}

/* Dummy definitions to satisfy linker (these would normally be in another file) */
void opaque_func1(void) {
    asm volatile("nop" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile("" : "+t"(x) : : "memory");
    return x * 1.1;
}

void* opaque_func4(void* x) {
    asm volatile("" : "+r"(x) : : "memory");
    return (void*)((long)x + 1);
}
