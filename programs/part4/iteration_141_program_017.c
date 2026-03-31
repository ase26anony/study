/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

/* Function with mixed register usage and calls */
int __attribute__((noinline)) 
compute_with_saves(int *arr, int n, void (*callback)(int*)) {
    /* Force values into specific registers */
    register long r10 asm("r10") = arr[0];
    register long r11 asm("r11") = arr[1];
    register long r12 asm("r12") = arr[2];  /* Call-saved reg */
    register long r13 asm("r13") = arr[3];  /* Call-saved reg */
    register long r14 asm("r14") = arr[4];  /* Call-saved reg */
    
    volatile int keep_alive = 0;
    int sum = 0;
    
    /* Nested loops with calls - creates complex live ranges */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 3; j++) {
            /* Mix of operations in call-clobbered registers */
            r10 = r10 * 3 + arr[i];
            r11 = r11 / 2 + arr[n - i - 1];
            
            /* Use inline assembly to prevent optimization */
            asm volatile("" : "+r"(r10), "+r"(r11) : : "cc", "memory");
            
            /* Function call - forces save/restore of call-clobbered regs */
            callback(&arr[i]);
            
            /* More operations keeping r10, r11 live across calls */
            r10 = r10 ^ arr[j];
            r11 = r11 | 0xFF;
            
            /* Use call-saved registers across the call */
            r12 = r12 + r10;
            r13 = r13 - r11;
            r14 = r14 ^ (r10 & r11);
            
            /* Volatile access to prevent dead code elimination */
            keep_alive = r10 + r11;
        }
        
        /* Another call with different pattern */
        if (i % 2 == 0) {
            external_func2(&arr[i]);
        } else {
            external_func3(&arr[i]);
        }
        
        /* Complex expression mixing all registers */
        sum += (r10 * r11) + (r12 - r13) ^ r14;
        
        /* Force spill/reload around this point */
        asm volatile("" : : "r"(r10), "r"(r11), "r"(r12), 
                     "r"(r13), "r"(r14) : "memory");
    }
    
    /* Final computation using all registers */
    int result = (r10 + r11) * (r12 - r13) / (r14 ? r14 : 1);
    return sum + result + keep_alive;
}

/* Function with indirect calls */
int __attribute__((noinline))
indirect_calls_test(int *arr, int n, int selector) {
    /* Array of function pointers */
    void (*funcs[3])(int*) = {
        external_func1,
        external_func2,
        external_func3
    };
    
    /* Multiple register variables */
    register int a asm("rax") = arr[0];
    register int b asm("rbx") = arr[1];  /* Call-saved */
    register int c asm("rcx") = arr[2];
    register int d asm("rdx") = arr[3];
    
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Vary which function is called */
        void (*fp)(int*) = funcs[(i + selector) % 3];
        
        /* Compute values in call-clobbered registers */
        a = a * 7 + i;
        c = c * 3 - i;
        d = d ^ (a * c);
        
        /* Inline assembly to create artificial dependencies */
        asm volatile("# dependency" : "+r"(a), "+r"(c), "+r"(d));
        
        /* Indirect call - compiler must be conservative */
        fp(&arr[i]);
        
        /* Use values after call */
        b = b + a;      /* Call-saved reg updated */
        total += b * c - d;
        
        /* Another call */
        if (i % 4 == 0) {
            external_func1(&total);
        }
        
        /* More operations */
        a = a ^ total;
        c = c + b;
        d = d * 2;
        
        /* Force all registers to be live here */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    }
    
    return total + a + b + c + d;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    int data[SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37 + 123) % 1000;
    }
    
    printf("Starting caller-save test...\n");
    
    /* Test 1: Direct function pointer call */
    int result1 = compute_with_saves(data, 50, external_func1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: Indirect calls with varying targets */
    int result2 = indirect_calls_test(data, 40, 1);
    printf("Result 2: %d\n", result2);
    
    /* Test 3: More complex pattern */
    int sum = 0;
    for (int i = 0; i < 30; i++) {
        /* Alternate between different call patterns */
        void (*fp)(int*) = (i % 3 == 0) ? external_func1 : 
                          (i % 3 == 1) ? external_func2 : external_func3;
        
        /* Create register pressure */
        register int x asm("r8") = data[i];
        register int y asm("r9") = data[i + 1];
        register int z asm("r10") = data[i + 2];
        
        for (int j = 0; j < 5; j++) {
            x = x * 2 + j;
            y = y / 3 - j;
            z = z ^ (x * y);
            
            /* Call with live values in call-clobbered regs */
            fp(&data[i]);
            
            /* Use values after call */
            sum += x + y + z;
            
            /* Prevent optimization */
            asm volatile("" : "+r"(x), "+r"(y), "+r"(z));
        }
    }
    
    printf("Result 3: %d\n", sum);
    
    /* Verify final state */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check ^= data[i];
    }
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
