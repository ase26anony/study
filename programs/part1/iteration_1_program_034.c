/* caller-save-test.c - ISO C99 program to exercise GCC's caller-save register allocation */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force noinline to prevent unwanted optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with different signatures and calling conventions */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return result;
}

NOINLINE float helper2(float a, float b, float c, float d, 
                       float e, float f, float g, float h,
                       float i, float j, float k, float l) {
    /* Many float arguments to exceed register passing limits */
    volatile float sum = a + b + c + d + e + f + g + h + i + j + k + l;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7");
    return sum * 0.5f;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, 
                              int i1, int i2, float f1, double d1) {
    /* Mixed argument types */
    volatile uintptr_t addr = (uintptr_t)p1 + (uintptr_t)p2 - (uintptr_t)p3;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1");
    return (void*)(addr + i1 + i2 + (uintptr_t)(f1 + d1));
}

NOINLINE double helper4(double a, double b, double c, double d,
                        double e, double f, double g, double h) {
    /* Double precision calculations */
    volatile double prod = a * b * c * d;
    volatile double sum = e + f + g + h;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9");
    return prod / sum;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int helper5(int size) {
    volatile char* buffer = alloca(size + 32);
    for (int i = 0; i < size; i++) {
        buffer[i] = (i % 256);
    }
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
    return buffer[size / 2];
}

/* Function with variable arguments to create different call site */
NOINLINE int helper6(int count, ...) {
    volatile int sum = 0;
    /* Force register pressure with many operations */
    for (int i = 0; i < 10; i++) {
        sum += i * count;
    }
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "r8", "r9", "r10");
    return sum;
}

/* Main computation with high register pressure */
NOINLINE static int compute_checksum(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f;
    volatile double da = 1.01, db = 2.02, dc = 3.03, dd = 4.04;
    volatile void* ptr1 = &a, *ptr2 = &b, *ptr3 = &c;
    volatile int result = 0;
    
    /* Create basic block boundaries with control flow */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Integer computations and calls */
            a = helper1(a, b, c, d, e, f, g, h);
            __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
            
            b = a + helper1(b, c, d, e, f, g, h, a);
            c = b * helper1(c, d, e, f, g, h, a, b);
            
            /* Use inline assembly between computations */
            __asm__ volatile ("mov %0, %%rax\n\t"
                             "add %1, %%rax\n\t"
                             : : "r"(a), "r"(b) : "rax", "cc");
            
            d = c - helper1(d, e, f, g, h, a, b, c);
        } else {
            /* Block 2: Floating point computations and calls */
            fa = helper2(fa, fb, fc, fd, fa*2, fb*2, fc*2, fd*2,
                        fa/2, fb/2, fc/2, fd/2);
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
            
            fb = fa + helper2(fb, fc, fd, fa, fb, fc, fd, fa,
                             fb, fc, fd, fa);
            
            /* More register pressure */
            da = helper4(da, db, dc, dd, da+1, db+1, dc+1, dd+1);
            db = helper4(db, dc, dd, da, db+2, dc+2, dd+2, da+2);
        }
        
        /* Block 3: Pointer and mixed computations */
        ptr1 = helper3(ptr1, ptr2, ptr3, a, b, fa, da);
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "xmm0");
        
        ptr2 = helper3(ptr2, ptr3, ptr1, c, d, fb, db);
        ptr3 = helper3(ptr3, ptr1, ptr2, e, f, fc, dc);
        
        /* Function with alloca affecting frame pointer */
        int temp = helper5(64 + iteration * 16);
        __asm__ volatile ("" : : : "rax", "rsp");
        
        /* Variable argument function call */
        result += helper6(iteration, a, b, c, d, e, f, g, h);
        
        /* Complex expression keeping many values live across calls */
        result += (int)(fa + fb + fc + fd) + (int)(da + db + dc + dd);
        result += ((uintptr_t)ptr1 + (uintptr_t)ptr2 + (uintptr_t)ptr3) % 1000;
    }
    
    return result;
}

/* Another function to create more call sites */
NOINLINE static void process_data(int* data, int size) {
    volatile float accum = 0.0f;
    volatile double daccum = 0.0;
    
    for (int i = 0; i < size; i++) {
        if (i % 4 == 0) {
            accum += helper2(accum, data[i], i, i*2, 
                           i*3, i*4, i*5, i*6,
                           accum/2, accum/3, accum/4, accum/5);
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
        } else if (i % 4 == 1) {
            daccum += helper4(daccum, data[i], i, i*0.5,
                            i*0.25, i*0.125, accum, accum*2);
        } else if (i % 4 == 2) {
            data[i] = helper1(data[i], data[i-1], data[i-2], i, 
                            i+1, i+2, i+3, i+4);
            __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8");
        } else {
            void* ptr = helper3(&data[i], &accum, &daccum, 
                               data[i], i, accum, daccum);
            data[i] += (uintptr_t)ptr % 256;
        }
    }
}

int main(void) {
    volatile int checksum = 0;
    
    /* Initial computation with high register pressure */
    checksum = compute_checksum();
    
    /* Additional processing with array operations */
    int data[32];
    for (int i = 0; i < 32; i++) {
        data[i] = i * 3 + checksum % 17;
    }
    
    process_data(data, 32);
    
    /* Final aggregation */
    for (int i = 0; i < 32; i++) {
        checksum += data[i];
        if (i % 8 == 0) {
            /* Insert calls at different points in the loop */
            checksum += helper5(32);
            __asm__ volatile ("" : : : "rax", "rsp", "rcx");
        }
    }
    
    /* One more complex call sequence */
    float fresult = helper2(checksum, checksum*0.5f, checksum*0.25f, 
                           checksum*0.125f, 1.0f, 2.0f, 3.0f, 4.0f,
                           5.0f, 6.0f, 7.0f, 8.0f);
    
    double dresult = helper4(checksum, checksum*0.5, checksum*0.25,
                            checksum*0.125, fresult, fresult*2,
                            fresult*3, fresult*4);
    
    checksum += (int)fresult + (int)dresult;
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
