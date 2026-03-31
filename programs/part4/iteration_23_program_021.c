/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -O1 -fomit-frame-pointer -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double);

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile double g_volatile_double = 3.14159;
volatile long g_volatile_long = 987654321;
volatile void* g_volatile_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[4];

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f;
    double x, y, z;
    void* ptr;
};

/* Force register usage with explicit register variables */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register int reg_var3 asm ("r14");

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1(int a, int b, int c, int d, int e, int f) {
    volatile int local1 = a + b;
    volatile int local2 = c + d;
    volatile int local3 = e + f;
    volatile int local4 = a * b;
    volatile int local5 = c * d;
    volatile int local6 = e * f;
    
    /* Use explicit register variables */
    reg_var1 = local1 + local2;
    reg_var2 = local3 + local4;
    reg_var3 = local5 + local6;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%r10d\n\t"
        "addl %%eax, %%r10d\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "r10", "memory"
    );
    
    /* Function call with many live variables */
    opaque_call_1();
    
    /* Use all volatile locals after call - forces save/restore */
    int sum = local1 + local2 + local3 + local4 + local5 + local6;
    
    /* More inline asm to prevent optimization */
    asm volatile (
        "addl %0, %%r12d\n\t"
        "addl %1, %%r13d\n\t"
        "addl %2, %%r14d\n\t"
        : /* no outputs */
        : "r" (local1), "r" (local2), "r" (local3)
        : "r12", "r13", "r14", "memory"
    );
    
    g_volatile_int += sum + reg_var1 + reg_var2 + reg_var3;
}

/* Test function 2: Complex control flow with calls at block boundaries */
__attribute__((noinline, noclone))
void test2(int n) {
    volatile int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * n;
    }
    
    /* Complex control flow with goto */
    int j = 0;
    volatile int sum = 0;
    
start_loop:
    if (j >= 10) goto end_loop;
    
    /* Function call inside loop with conditional */
    if (j % 3 == 0) {
        opaque_call_2(arr[j]);
        /* Force register pressure around call */
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    }
    
    /* Nested conditional with break */
    if (arr[j] > 20) {
        if (j % 2 == 0) {
            /* Call at potential block boundary */
            int result = opaque_call_3(arr[j], j);
            sum += result;
            if (result > 100) {
                j++;
                continue;
            }
        } else {
            /* Another call path */
            opaque_call_1();
        }
    }
    
    /* Use goto to create irreducible flow */
    if (j == 5) {
        j++;
        goto middle;
    }
    
    j++;
    goto start_loop;

middle:
    /* Insert instruction at block boundary */
    asm volatile ("nop" : : : "memory");
    opaque_call_4(3.14);
    goto start_loop;

end_loop:
    g_volatile_int += sum;
}

/* Test function 3: Switch statement with calls in default case */
__attribute__((noinline, noclone))
void test3(int code) {
    volatile int x = g_volatile_int;
    volatile double y = g_volatile_double;
    volatile long z = g_volatile_long;
    
    /* Use explicit register variables across switch */
    reg_var1 = x;
    reg_var2 = (int)z;
    
    switch (code) {
        case 1:
            x += 10;
            break;
        case 2:
            y *= 2.0;
            break;
        case 3:
            z >>= 1;
            break;
        default:
            /* Function call in default case - creates block boundary */
            opaque_call_2(x);
            
            /* Complex expression requiring temporary registers */
            int temp = (x * 2) + ((int)y * 3) + ((int)z / 4);
            
            /* Inline asm that looks like a call */
            asm volatile (
                "call *%0\n\t"
                : /* no outputs */
                : "r" (func_table[code % 4])
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory"
            );
            
            /* Use all volatiles after asm */
            g_volatile_int = x + temp;
            g_volatile_double = y + temp;
            g_volatile_long = z + temp;
            break;
    }
    
    /* Force register saves across the switch */
    asm volatile (
        "addl %%r12d, %0\n\t"
        "addl %%r13d, %1\n\t"
        : "+m" (g_volatile_int), "+m" (g_volatile_long)
        : /* no inputs */
        : "r12", "r13", "memory"
    );
}

/* Test function 4: Nested function calls with register pressure */
__attribute__((noinline, noclone))
int test4_helper(int a, int b, int c, int d, int e, int f) {
    /* Many parameters to force register pressure */
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile int v3 = e + f;
    
    /* Call within helper */
    opaque_call_3(v1, v2);
    
    /* Complex computation requiring many registers */
    return (v1 * v2) + (v2 * v3) + (v3 * v1) + 
           (a * b) + (c * d) + (e * f);
}

__attribute__((noinline, noclone))
void test4(int iterations) {
    volatile int results[10];
    volatile int accum = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Create many live values */
        int a = i * 2;
        int b = i * 3;
        int c = i * 4;
        int d = i * 5;
        int e = i * 6;
        int f = i * 7;
        
        /* Nested call with many live variables */
        results[i] = test4_helper(a, b, c, d, e, f);
        
        /* Function call between uses of results */
        if (i % 2 == 0) {
            opaque_call_1();
        }
        
        /* Use result after potential call */
        accum += results[i];
        
        /* More register pressure */
        asm volatile (
            "movl %0, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "addl %%eax, %1\n\t"
            : "+m" (results[i]), "+m" (accum)
            : /* no inputs */
            : "eax", "memory"
        );
    }
    
    g_volatile_int += accum;
}

/* Test function 5: Mixed types and calling conventions */
__attribute__((noinline, noclone))
void test5(void) {
    /* Mix of types to stress different register classes */
    volatile int vi = 42;
    volatile double vd = 2.71828;
    volatile long vl = 999888777;
    volatile float vf = 1.414;
    volatile short vs = 32767;
    volatile char vc = 'A';
    
    /* Use all in complex expression before call */
    double complex_expr = (double)vi + vd + (double)vl + (double)vf + 
                         (double)vs + (double)vc;
    
    /* Call with double argument */
    double result = opaque_call_4(complex_expr);
    
    /* Use all volatiles after call */
    vi += (int)result;
    vd += result;
    vl += (long)result;
    vf += (float)result;
    vs += (short)result;
    vc += (char)result;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Another call via function pointer */
    func_ptr_t fp = func_table[vi % 4];
    if (fp) {
        /* Inline asm that mimics call setup/teardown */
        asm volatile (
            "pushq %%rbp\n\t"
            "movq %%rsp, %%rbp\n\t"
            "andq $-16, %%rsp\n\t"
            "call *%0\n\t"
            "leave\n\t"
            : /* no outputs */
            : "r" (fp)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "memory"
        );
    }
    
    /* Store all results to globals */
    g_volatile_int = vi;
    g_volatile_double = vd;
    g_volatile_long = vl;
}

/* Main function with mode selection */
int main(int argc, char **argv) {
    /* Initialize function table with opaque functions */
    func_table[0] = (func_ptr_t)opaque_call_1;
    func_table[1] = (func_ptr_t)opaque_call_2;
    func_table[2] = (func_ptr_t)opaque_call_3;
    func_table[3] = (func_ptr_t)opaque_call_4;
    
    /* Parse test mode from argv if provided */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile globals */
    g_volatile_int = 0;
    g_volatile_double = 0.0;
    g_volatile_long = 0;
    g_volatile_ptr = &g_volatile_int;
    
    /* Run all tests, but order depends on mode */
    switch (test_mode) {
        case 0:
            test1(1, 2, 3, 4, 5, 6);
            test2(10);
            test3(99);  /* Use default case */
            test4(8);
            test5();
            break;
        case 1:
            test5();
            test1(7, 8, 9, 10, 11, 12);
            test4(5);
            test3(2);   /* Use case 2 */
            test2(7);
            break;
        case 2:
            test2(15);
            test3(1);   /* Use case 1 */
            test5();
            test1(13, 14, 15, 16, 17, 18);
            test4(7);
            break;
        case 3:
            test4(9);
            test5();
            test2(12);
            test1(19, 20, 21, 22, 23, 24);
            test3(3);   /* Use case 3 */
            break;
        case 4:
            test3(100); /* Use default case */
            test4(6);
            test1(25, 26, 27, 28, 29, 30);
            test5();
            test2(8);
            break;
    }
    
    /* Additional complex loop with nested calls */
    volatile int outer_acc = 0;
    for (int i = 0; i < 3; i++) {
        /* Force register saves across loop iterations */
        reg_var1 = i * 100;
        reg_var2 = i * 200;
        reg_var3 = i * 300;
        
        /* Call with side effects */
        test4_helper(reg_var1, reg_var2, reg_var3, i, i+1, i+2);
        
        /* Use registers after call */
        outer_acc += reg_var1 + reg_var2 + reg_var3;
        
        /* Inline asm that clobbers everything */
        asm volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx\n\t"
            "movl $0, %%esi\n\t"
            "movl $0, %%edi\n\t"
            "movl $0, %%r8d\n\t"
            "movl $0, %%r9d\n\t"
            "movl $0, %%r10d\n\t"
            "movl $0, %%r11d\n\t"
            "movl $0, %%r12d\n\t"
            "movl $0, %%r13d\n\t"
            "movl $0, %%r14d\n\t"
            "movl $0, %%r15d\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory"
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = g_volatile_int + (int)g_volatile_double + 
                   (int)g_volatile_long + outer_acc;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_call_1(void) {
    /* Empty but marked noinline */
    asm volatile ("");
}

void opaque_call_2(int x) {
    g_volatile_int += x;
}

int opaque_call_3(int a, int b) {
    return a + b + g_volatile_int;
}

double opaque_call_4(double x) {
    return x * g_volatile_double;
}
