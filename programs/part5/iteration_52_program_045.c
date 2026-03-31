/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to prevent optimization */
extern int opaque_func(int x);

/* Stress function with complex register pressure patterns */
int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Multi-use temporary value - candidate for rematerialization */
        int base = v1 * v2 + opaque_func(v3) - (v4 % (i + 1));
        
        /* Complex expression with many intermediate values */
        int temp1 = base * v1 + v2 / (v3 + 1);
        int temp2 = (v1 % (v2 + 1)) * base - v3;
        int temp3 = temp1 * temp2 + base / (v4 + 1);
        int temp4 = (temp2 % (temp3 + 1)) - base * v1;
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 5; j++) {
            /* Base address recomputation pattern */
            int* ptr = &array[j];
            result += *(ptr + 0) + *(ptr + 1) + *(ptr + 2);
        }
        
        /* Inline assembly clobbering registers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            : 
            : "r"(temp3), "r"(temp4)
            : "r0", "r1", "r2", "r3", "memory"
        );
        #endif
        
        /* Multi-use of base in different control flow paths */
        if (i % 3 == 0) {
            result += base * 2 + temp1;
        } else if (i % 3 == 1) {
            result += base / 2 + temp2;
        } else {
            result += base + temp3 - temp4;
        }
        
        /* Prevent loop optimization with volatile */
        v1 += opaque_func(i);
        v2 -= v3 % (i + 2);
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_address_computation(int* data, int size) {
    int sum = 0;
    
    for (int i = 0; i < size - 3; i++) {
        /* Base address computation reused with different offsets */
        int* base_ptr = &data[i];
        
        /* Multiple uses of computed base with different offsets */
        sum += base_ptr[0] * base_ptr[1];
        sum -= base_ptr[1] * base_ptr[2];
        sum += base_ptr[2] * base_ptr[3];
        
        /* Complex expression that might need rematerialization */
        int offset = opaque_func(i) % 16;
        int* offset_ptr = base_ptr + offset;
        
        if (offset < size - i) {
            sum += *offset_ptr * 2;
        }
        
        /* More register pressure */
        int temp = data[i] * 3 + data[i + 1] * 5 - data[i + 2] * 7;
        sum += temp / (offset + 1);
        
        #ifdef __OPTIMIZE__
        /* Clobber more registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : "r"(sum), "r"(temp)
            : "r4", "r5", "r6", "r7", "memory"
        );
        #endif
    }
    
    return sum;
}

/* Function with switch statement for multi-use temporaries */
int stress_switch_computation(int mode, int x, int y) {
    int result = 0;
    
    /* Compute value once, use in multiple switch cases */
    int computed = x * y + opaque_func(x) - opaque_func(y);
    computed = computed * 2 - (x % (y + 1));
    
    volatile int v = mode;
    
    switch (v % 4) {
        case 0:
            result = computed * 3 + x;
            break;
        case 1:
            result = computed / 2 + y;
            break;
        case 2:
            result = computed + x * y;
            break;
        case 3:
            result = computed - x + y;
            break;
        default:
            result = computed;
    }
    
    /* Additional complex computation */
    for (int i = 0; i < 4; i++) {
        int temp = computed * i + result;
        result += temp / (i + 1);
        
        #ifdef __OPTIMIZE__
        asm volatile (
            "nop\n\t"
            : 
            : "r"(temp), "r"(computed)
            : "r8", "r9", "r10", "memory"
        );
        #endif
    }
    
    return result;
}

/* Opaque function implementation */
int opaque_func(int x) {
    /* Use system call or volatile to prevent optimization */
    volatile int v = x;
    return v * 1103515245 + 12345;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int array_size = 50;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 10) array_size = 50;
    }
    
    /* Initialize data */
    int* data = (int*)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        data[i] = opaque_func(i);
    }
    
    int total_result = 0;
    
    /* Call stress functions multiple times */
    for (int i = 0; i < iterations; i++) {
        total_result += stress_computation(i, 10);
        total_result += stress_address_computation(data, array_size);
        total_result += stress_switch_computation(i, i * 2, i * 3);
        
        /* Modify data to prevent optimization */
        data[i % array_size] = opaque_func(total_result);
    }
    
    printf("Result: %d\n", total_result);
    
    free(data);
    return total_result != 0 ? 0 : 1;
}
