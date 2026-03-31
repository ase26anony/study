/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func3(unsigned long *p) {
    *p ^= 0xAAAAAAAA;
    asm volatile("" : : : "memory");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Force register usage with explicit register variables */
#ifdef __x86_64__
#define REG1 "r10"
#define REG2 "r11"
#define REG3 "r12"  /* Call-saved register */
#define REG4 "r13"  /* Call-saved register */
#define REG5 "r14"
#define REG6 "r15"
#elif defined(__i386__)
#define REG1 "esi"
#define REG2 "edi"
#define REG3 "ebx"  /* Call-saved register */
#define REG4 "ebp"  /* Call-saved register */
#define REG5 "ecx"
#define REG6 "edx"
#else
/* Generic fallback - will still work but may not hit specific registers */
#define REG1 ""
#define REG2 ""
#define REG3 ""
#define REG4 ""
#define REG5 ""
#define REG6 ""
#endif

/* Main test function with complex register usage across calls */
int __attribute__((noinline)) test_function(int *data, int size, func_ptr_t fp) {
    volatile int counter = 0;  /* Prevent optimization */
    int result = 0;
    
    /* Explicit register variables to force specific register allocation */
    register long reg_val1 asm(REG1) = 0;
    register long reg_val2 asm(REG2) = 0;
    register long reg_val3 asm(REG3) = 0;  /* Call-saved */
    register long reg_val4 asm(REG4) = 0;  /* Call-saved */
    register long reg_val5 asm(REG5) = 0;
    register long reg_val6 asm(REG6) = 0;
    
    /* Initialize with data */
    reg_val1 = data[0];
    reg_val2 = data[1];
    reg_val3 = data[2];
    reg_val4 = data[3];
    reg_val5 = data[4];
    reg_val6 = data[5];
    
    /* Nested loops with function calls - creates complex live ranges */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            /* Mix of operations keeping values live in registers */
            reg_val1 = reg_val1 * reg_val2 + reg_val3;
            reg_val2 = reg_val2 ^ reg_val4 | reg_val5;
            reg_val3 = reg_val3 + reg_val6 - reg_val1;
            
            /* Artificial dependency to prevent reordering */
            asm volatile("" : "+r"(reg_val1), "+r"(reg_val2), "+r"(reg_val3));
            
            /* Call external function - forces caller-save for call-clobbered regs */
            if (j == 0) {
                external_func1(&data[i]);
            } else if (j == 1) {
                external_func2(&reg_val5);  /* Pass call-clobbered register value */
            } else {
                /* Function pointer call - inhibits optimizations */
                fp(&reg_val6);
            }
            
            /* More operations after call - values must be restored */
            reg_val4 = reg_val4 + reg_val1 * 2;
            reg_val5 = reg_val5 - reg_val2 / 3;
            reg_val6 = reg_val6 ^ reg_val3;
            
            /* Another artificial dependency */
            asm volatile("" : "+r"(reg_val4), "+r"(reg_val5), "+r"(reg_val6));
            
            counter++;  /* Volatile access */
        }
        
        /* Store results back, creating pressure to keep values in registers */
        data[i] = (reg_val1 + reg_val2 + reg_val3 + 
                   reg_val4 + reg_val5 + reg_val6) % 1000;
        
        /* Rotate register values to create different live ranges */
        long temp = reg_val1;
        reg_val1 = reg_val2;
        reg_val2 = reg_val3;
        reg_val3 = reg_val4;
        reg_val4 = reg_val5;
        reg_val5 = reg_val6;
        reg_val6 = temp;
    }
    
    /* Final computation using all register values */
    result = (reg_val1 ^ reg_val2) + (reg_val3 & reg_val4) - 
             (reg_val5 | reg_val6) + counter;
    
    return result;
}

/* Another test with indirect calls and mixed register usage */
int __attribute__((noinline)) test_indirect_calls(int *data, int size) {
    int sum = 0;
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Multiple register variables with mixed lifetimes */
    register int r1 asm(REG1) = 1;
    register int r2 asm(REG2) = 2;
    register int r3 asm(REG3) = 3;  /* Call-saved */
    register int r4 asm(REG4) = 4;  /* Call-saved */
    
    for (int i = 0; i < size; i++) {
        /* Complex expression spanning multiple calls */
        for (int f = 0; f < 3; f++) {
            /* Values must survive across indirect call */
            r1 = r1 * data[i] + r2;
            r2 = r2 + data[(i + 1) % size] - r3;
            
            /* Inline asm to prevent optimization */
            asm volatile("" : "+r"(r1), "+r"(r2));
            
            /* Indirect call - compiler doesn't know which registers are clobbered */
            funcs[f](&data[i]);
            
            /* More computation after call */
            r3 = r3 ^ r1;
            r4 = r4 | r2;
            
            /* Force dependency chain */
            asm volatile("" : "+r"(r3), "+r"(r4));
        }
        
        sum += r1 + r2 + r3 + r4;
        
        /* Swap values to create different register pressure */
        int tmp = r1;
        r1 = r2;
        r2 = r3;
        r3 = r4;
        r4 = tmp;
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37 + 123) % 1000;
    }
    
    /* Run first test */
    int result1 = test_function(data, SIZE, (func_ptr_t)external_func3);
    printf("Test 1 result: %d\n", result1);
    
    /* Re-initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 51 + 79) % 1000;
    }
    
    /* Run second test with indirect calls */
    int result2 = test_indirect_calls(data, SIZE);
    printf("Test 2 result: %d\n", result2);
    
    /* Verify some data was modified */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
