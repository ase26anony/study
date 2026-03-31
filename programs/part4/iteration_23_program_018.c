/* test-caller-save.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern void opaque_func2(int) __attribute__((noinline, noclone));
extern int opaque_func3(int, int) __attribute__((noinline, noclone));
extern double opaque_func4(double) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile long global_volatile_long = 1234567890L;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
func_ptr_t volatile func_ptrs[4];

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f;
    double x, y, z;
    volatile int v;
};

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) \
    register int var asm(reg) = global_volatile_int

/* Test function 1: Many live variables across call site */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many variables to be live across call */
    volatile int v1 = global_volatile_int;
    volatile int v2 = v1 + 1;
    volatile int v3 = v2 * 2;
    volatile int v4 = v3 - v1;
    volatile double d1 = global_volatile_double;
    volatile double d2 = d1 * 2.0;
    
    /* Use explicit register variables */
    register int r10_val asm("r10") = v1;
    register int r11_val asm("r11") = v2;
    
    /* Array to force stack usage */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = v1 + i;
    }
    
    COMPILER_BARRIER();
    
    /* Function call with many live registers */
    if (mode & 1) {
        opaque_func1();
    } else {
        opaque_func2(v1);
    }
    
    COMPILER_BARRIER();
    
    /* Use all variables after call - forces save/restore */
    v1 = r10_val + r11_val;
    v2 = arr[v1 % 10];
    v3 = (int)(d1 + d2);
    v4 = v1 ^ v2 ^ v3;
    
    /* Complex control flow to split basic blocks */
    switch (v4 & 3) {
        case 0:
            opaque_func1();
            break;
        case 1:
            opaque_func2(v1);
            /* Fall through */
        case 2:
            v1 = opaque_func3(v2, v3);
            break;
        default:
            /* This creates a basic block boundary */
            if (v1 > 0) {
                opaque_func1();
                v1 = 0;
            } else {
                opaque_func2(v1);
                v1 = 1;
            }
            /* goto to create irreducible flow */
            if (v1 == 0) goto label1;
    }
    
    if (mode & 2) {
        /* Another call site */
        opaque_func3(v1, v2);
    }
    
label1:
    /* Use variables again */
    global_volatile_int += v1 + v2 + v3 + v4;
}

/* Test function 2: Nested calls with register pressure */
__attribute__((noinline, noclone))
int test2(int depth) {
    volatile int results[8];
    volatile double doubles[4];
    
    /* Initialize with complex pattern */
    for (int i = 0; i < 8; i++) {
        results[i] = global_volatile_int + i * i;
    }
    for (int i = 0; i < 4; i++) {
        doubles[i] = global_volatile_double * i;
    }
    
    /* Force specific registers with asm clobber */
    int a, b, c, d;
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        : "=r"(a)
        : "r"(results[0]), "r"(results[1])
        : "eax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Nested call scenario */
    if (depth > 0) {
        b = test2(depth - 1);
        
        /* Live variables across inner call */
        c = results[2] + results[3];
        asm volatile ("" : : "r"(c) : "r10", "r11");
        
        /* Another call with live variables */
        d = opaque_func3(a, b);
        
        /* Use all variables - forces spill/restore */
        results[0] = a + b + c + d;
    } else {
        /* Base case with direct call */
        opaque_func1();
        b = results[4];
        c = results[5];
        d = results[6];
    }
    
    /* Loop with break to create block boundaries */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        if (i == depth) {
            opaque_func2(i);  /* Call at block boundary */
            break;
        }
        sum += results[i];
        if (sum > 1000) {
            continue;  /* Creates another block boundary */
        }
    }
    
    /* Complex expression requiring temporaries */
    return (a * b) + (c / (d + 1)) + sum;
}

/* Test function 3: Mixed types and calling conventions */
__attribute__((noinline, noclone))
double test3(int count) {
    /* Mix of types to stress different register classes */
    volatile int ints[6];
    volatile double floats[4];
    volatile long longs[3];
    
    /* Initialize */
    for (int i = 0; i < 6; i++) ints[i] = i * global_volatile_int;
    for (int i = 0; i < 4; i++) floats[i] = global_volatile_double / (i + 1);
    for (int i = 0; i < 3; i++) longs[i] = global_volatile_long >> i;
    
    COMPILER_BARRIER();
    
    /* Function pointer call to create indirect jump */
    func_ptr_t fp = func_ptrs[count % 4];
    if (fp) {
        fp();
    }
    
    /* Use inline asm that looks like a call */
    double result;
    asm volatile (
        "call *%1\n\t"
        "fldl %2\n\t"
        "fstpl %0\n\t"
        : "=m"(result)
        : "r"(fp), "m"(floats[0])
        : "eax", "edx", "ecx", "st", "st(1)", "memory"
    );
    
    /* Switch with calls in multiple cases */
    switch (count & 3) {
        case 0:
            opaque_func1();
            result += floats[0];
            /* Fall through */
        case 1:
            opaque_func2(ints[0]);
            result *= floats[1];
            break;
        case 2:
            result = opaque_func4(floats[2]);
            /* goto to create back edge */
            if (result > 0) goto recalculation;
            break;
        default:
            opaque_func3(ints[1], ints[2]);
            result = floats[3];
    }
    
recalculation:
    /* Complex floating point computation */
    for (int i = 0; i < count % 4; i++) {
        result = result * floats[i] + ints[i];
        if (i == 1) {
            opaque_func4(result);  /* Call in middle of loop */
        }
    }
    
    return result;
}

/* Test function 4: __builtin_apply usage */
__attribute__((noinline, noclone))
void test4(void* arg) {
    /* Use __builtin_apply to create unusual call sequences */
    volatile int buffer[10];
    for (int i = 0; i < 10; i++) {
        buffer[i] = i * (int)(long)arg;
    }
    
    /* Simulate variable arguments */
    int (*func)(int, ...) = (int (*)(int, ...))opaque_func3;
    
    /* Force register pressure before builtin */
    register int r1 asm("ebx") = buffer[0];
    register int r2 asm("esi") = buffer[1];
    register int r3 asm("edi") = buffer[2];
    
    COMPILER_BARRIER();
    
    /* This creates complex prologue/epilogue */
    asm volatile (
        "push %0\n\t"
        "push %1\n\t"
        "push %2\n\t"
        "call *%3\n\t"
        "add $12, %%esp\n\t"
        : 
        : "r"(r1), "r"(r2), "r"(r3), "r"(func)
        : "eax", "ecx", "edx", "memory"
    );
    
    /* Unreachable code pattern to force edge cases */
    if (global_volatile_int == 0) {
        goto unusual_path;
    }
    
    /* Normal path */
    buffer[3] = r1 + r2 + r3;
    return;
    
unusual_path:
    /* This creates another basic block */
    opaque_func1();
    asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
}

/* Helper with nested call for outer/inner save sequences */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x) {
    volatile int saved[5];
    for (int i = 0; i < 5; i++) saved[i] = x + i;
    
    /* Outer call context */
    int a = saved[0];
    int b = saved[1];
    
    COMPILER_BARRIER();
    
    /* Inner call */
    int c = opaque_func3(a, b);
    
    /* Live across inner call */
    int d = saved[2];
    int e = saved[3];
    
    /* Use inline asm to force specific register usage */
    int result;
    asm volatile (
        "imul %2, %1\n\t"
        "add %3, %1\n\t"
        "add %4, %1\n\t"
        "mov %1, %0\n\t"
        : "=r"(result)
        : "r"(c), "r"(d), "r"(e), "r"(saved[4])
        : "cc"
    );
    
    /* Conditional call at block end */
    if (result > 100) {
        opaque_func2(result);
        /* This should trigger BB_END update */
    }
    
    return result;
}

/* Main function with mode selection */
int main(int argc, char** argv) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize function pointers */
    func_ptrs[0] = (func_ptr_t)opaque_func1;
    func_ptrs[1] = (func_ptr_t)opaque_func2;
    func_ptrs[2] = (func_ptr_t)opaque_func3;
    func_ptrs[3] = (func_ptr_t)opaque_func4;
    
    /* Run all test functions in sequence */
    int result1, result2;
    double result3;
    
    COMPILER_BARRIER();
    
    /* Test 1 - forces many caller-save registers */
    test1(mode);
    
    /* Test 2 - nested calls */
    result1 = test2(2);
    
    /* Test 3 - mixed types */
    result3 = test3(mode + 1);
    
    /* Test 4 - builtin-like patterns */
    test4((void*)(long)result1);
    
    /* Helper with nested structure */
    result2 = helper_with_nested_call(result1);
    
    /* Compute checksum to prevent elimination */
    int checksum = global_volatile_int + (int)global_volatile_double + 
                   (int)(global_volatile_long & 0xFFFFFFFF) + 
                   result1 + result2 + (int)result3;
    
    printf("Checksum: %d\n", checksum);
    
    /* Loop with calls at boundaries */
    for (int i = 0; i < 3; i++) {
        if (i == mode) {
            opaque_func1();  /* Call at potential BB_END */
            break;
        }
        checksum += i;
        if (checksum & 1) {
            opaque_func2(i);  /* Another call site */
            continue;
        }
    }
    
    return checksum & 255;
}

/* Dummy definitions to satisfy linker */
void opaque_func1(void) {
    global_volatile_int ^= 0x5555;
}

void opaque_func2(int x) {
    global_volatile_int += x;
}

int opaque_func3(int a, int b) {
    return a - b + global_volatile_int;
}

double opaque_func4(double x) {
    return x * global_volatile_double;
}
