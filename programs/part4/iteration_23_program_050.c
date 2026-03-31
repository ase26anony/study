/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent optimization */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void* opaque_func4(void*);

/* Volatile globals to maintain live ranges across calls */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile void* global_volatile_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_table[4];

/* Complex structure to force register pressure */
struct LargeRegPressure {
    long a, b, c, d, e, f, g, h;
    double x, y, z;
    void* p1, *p2;
};

/* Test function 1: Many integer arguments with register constraints */
__attribute__((noinline, noclone))
int test1(int mode) {
    /* Explicit register variables that conflict with call-clobbered regs */
    register int r10_val asm("r10") = global_volatile_int;
    register int r11_val asm("r11") = mode * 2;
    register int r12_val asm("r12") = r10_val + r11_val;
    
    /* Volatile array to force spills */
    volatile int save_area[8];
    for (int i = 0; i < 8; i++) {
        save_area[i] = r12_val + i;
    }
    
    /* Inline asm that clobbers registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        : 
        : "r" (r10_val), "r" (r11_val)
        : "eax", "memory"
    );
    
    /* Function call that clobbers registers */
    int result = opaque_func2(r12_val);
    
    /* Complex use of saved values after call */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += save_area[i] * result;
    }
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    
    /* Use register variables again */
    r12_val = sum + r10_val - r11_val;
    
    /* Force register reload */
    asm volatile ("movl %0, %%r12d" : : "r" (r12_val) : "r12");
    
    return r12_val;
}

/* Test function 2: Floating point with mixed calls */
__attribute__((noinline, noclone))
double test2(double input) {
    volatile double saves[6];
    register double xmm0_val asm("xmm0") = input;
    register double xmm1_val asm("xmm1") = global_volatile_double;
    
    /* Save FP values */
    for (int i = 0; i < 6; i++) {
        saves[i] = xmm0_val + i * xmm1_val;
    }
    
    /* Call that might clobber FP registers */
    double temp = opaque_func3(xmm0_val);
    
    /* Complex FP computation requiring all saved values */
    double total = 0.0;
    for (int i = 0; i < 6; i++) {
        total += saves[i] * temp;
    }
    
    /* Irreducible control flow with calls */
    if (total > 100.0) {
        opaque_func1();
        goto label1;
    } else {
        if (total < 0) {
            opaque_func4(&global_volatile_ptr);
        }
    }
    
    /* Create basic block boundary */
    int count = 0;
    while (count < 3) {
        total += opaque_func2(count);
        if (total > 50.0) {
            break;  /* Causes block splitting */
        }
        count++;
        continue;
    label1:
        total -= 10.0;
        break;
    }
    
    return total;
}

/* Test function 3: Vector-like operations with many live values */
__attribute__((noinline, noclone))
void* test3(void* base, int size) {
    /* Many pointer variables live across calls */
    char* p1 = (char*)base;
    char* p2 = p1 + size/2;
    char* p3 = p2 + size/4;
    volatile char* volatile_p = p3;
    
    /* Save pointers in volatile locations */
    volatile void* saved_ptrs[4];
    saved_ptrs[0] = p1;
    saved_ptrs[1] = p2;
    saved_ptrs[2] = p3;
    saved_ptrs[3] = global_volatile_ptr;
    
    /* Switch with calls in different cases */
    switch (size % 4) {
        case 0:
            opaque_func1();
            p1 = (char*)saved_ptrs[0];
            break;
        case 1:
            opaque_func2(size);
            p2 = (char*)saved_ptrs[1];
            break;
        case 2:
            opaque_func3(size);
            p3 = (char*)saved_ptrs[2];
            break;
        default:
            /* This creates a call at block end */
            opaque_func4(saved_ptrs[3]);
            /* Following instruction might need insertion after call */
            p1 = (char*)saved_ptrs[0];
            p2 = (char*)saved_ptrs[1];
            p3 = (char*)saved_ptrs[2];
            break;
    }
    
    /* Use all pointers after switch */
    *p1 = *p2 + *p3;
    
    /* Inline asm that looks like a call */
    asm volatile (
        "call *%0\n\t"
        : 
        : "r" (func_table[size % 4])
        : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    
    return p1;
}

/* Test function 4: Nested calls with register pressure */
__attribute__((noinline, noclone))
long test4(long a, long b, long c, long d, long e, long f) {
    /* Many arguments -> register pressure */
    struct LargeRegPressure s;
    s.a = a; s.b = b; s.c = c; s.d = d; s.e = e; s.f = f;
    s.g = a + b; s.h = c + d;
    s.x = (double)a; s.y = (double)b; s.z = (double)c;
    s.p1 = &s; s.p2 = &global_volatile_int;
    
    /* Volatile struct to force memory ops */
    volatile struct LargeRegPressure vs = s;
    
    /* First call */
    long result1 = opaque_func2((int)a);
    
    /* Use struct after call */
    long sum = vs.a + vs.b + vs.c + vs.d;
    
    /* Nested call scenario */
    if (sum > result1) {
        /* Inner call with live values */
        double temp = opaque_func3(vs.x);
        sum += (long)(temp * 100.0);
        
        /* Instruction that might need insertion after inner call */
        vs.g = sum;
    } else {
        /* Different path */
        opaque_func4(vs.p1);
        vs.h = sum * 2;
    }
    
    /* Loop with break/continue creating block boundaries */
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            opaque_func1();
            continue;  /* Splits basic block */
        }
        if (i == 4) {
            break;     /* Another split */
        }
        sum += opaque_func2(i);
    }
    
    return sum + vs.g + vs.h;
}

/* Helper with nested call for outer/inner save sequence */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x) {
    volatile int saves[4];
    saves[0] = x;
    saves[1] = x * 2;
    
    /* Outer call */
    int mid = opaque_func2(x);
    
    /* Inner call with intervening computation */
    saves[2] = mid + saves[0];
    double dbl = opaque_func3((double)saves[2]);
    saves[3] = (int)dbl;
    
    /* Use all saves */
    return saves[0] + saves[1] + saves[2] + saves[3] + mid;
}

int main(int argc, char** argv) {
    /* Initialize function pointers */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Run all test functions in different orders based on mode */
    int int_result = 0;
    double double_result = 0.0;
    void* ptr_result = NULL;
    long long_result = 0;
    
    switch (mode) {
        case 0:
            int_result = test1(mode);
            double_result = test2(int_result);
            ptr_result = test3(&int_result, sizeof(int));
            long_result = test4(1, 2, 3, 4, 5, 6);
            break;
        case 1:
            double_result = test2(10.5);
            ptr_result = test3(&double_result, sizeof(double));
            int_result = test1((int)double_result);
            long_result = test4(6, 5, 4, 3, 2, 1);
            break;
        case 2:
            ptr_result = test3(&global_volatile_int, sizeof(global_volatile_int));
            long_result = test4(100, 200, 300, 400, 500, 600);
            int_result = test1((long)ptr_result & 0xFF);
            double_result = test2(int_result);
            break;
        default:
            long_result = test4(999, 888, 777, 666, 555, 444);
            int_result = helper_with_nested_call(long_result & 0xFF);
            double_result = test2(int_result);
            ptr_result = test3(&long_result, sizeof(long));
            break;
    }
    
    /* Force use of all results to prevent DCE */
    volatile int checksum = 0;
    checksum += int_result;
    checksum += (int)double_result;
    checksum += (long)ptr_result;
    checksum += (int)long_result;
    checksum += global_volatile_int;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    printf("Result checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
