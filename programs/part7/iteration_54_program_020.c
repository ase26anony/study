/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Volatile indices to prevent constant folding */
volatile int vi1 = 7, vi2 = 13, vi3 = 42;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Large local array to force spilling */
static char large_buffer[4096 * 4];

/* Vector type for SIMD operations */
typedef int v4si __attribute__((vector_size(16)));

/* Function prototypes */
struct SmallStruct process_struct(struct SmallStruct s);
struct SmallStruct modify_and_return(struct SmallStruct s, int multiplier);

/* Pattern 1: Complex addressing modes with multi-dimensional arrays */
void complex_addressing(int n) {
    int arr[100][50];
    int arr2[75][60];
    volatile int idx = vi1;
    
    /* Force address reloads with non-constant indices */
    for (int i = 0; i < n; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx % 100][i + vi2] = arr2[(i * vi3) % 75][idx * 2];
        
        /* More complex addressing with pointer arithmetic */
        int *ptr1 = &arr[idx][i];
        int *ptr2 = &arr2[i][idx];
        *ptr1 = *ptr2 + arr[(i + idx) % 100][(i * 3) % 50];
    }
}

/* Pattern 2: Inline assembly with register constraints */
void inline_asm_chain(int iterations) {
    int a = vi1, b = vi2, c = vi3;
    int d, e, f;
    
    for (int i = 0; i < iterations; i++) {
        /* Chain of asm blocks creating dependencies */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]"
            : [out1] "=r" (d)
            : [in1] "r" (a), [in2] "r" (b)
            : "cc"
        );
        
        /* Second asm using output of first as input */
        asm volatile (
            "imul %[in1], %[out1]\n\t"
            "sub $1, %[out1]"
            : [out1] "=r" (e)
            : [in1] "r" (d), "0" (d)
            : "cc"
        );
        
        /* Third asm with memory constraint */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "xor %%eax, %%eax\n\t"
            "add %[in2], %[out1]"
            : [out1] "=r" (f)
            : [in1] "r" (e), [in2] "m" (c)
            : "eax", "cc"
        );
        
        /* Cycle values to create live range splits */
        a = b;
        b = c;
        c = f;
    }
}

/* Pattern 3: Structure passing by value */
struct SmallStruct process_struct(struct SmallStruct s) {
    struct SmallStruct result;
    
    /* Force spilling of structure fields */
    result.a = s.b + vi1;
    result.b = s.c * vi2;
    result.c = s.d - vi3;
    result.d = s.a / (vi1 + 1);
    
    /* Call another function to increase register pressure */
    return modify_and_return(result, 2);
}

struct SmallStruct modify_and_return(struct SmallStruct s, int multiplier) {
    /* Complex operations on structure fields */
    s.a = (s.a * multiplier) + large_buffer[vi1 * 16];
    s.b = (s.b / multiplier) - large_buffer[vi2 * 8];
    s.c = (s.c + multiplier) ^ large_buffer[vi3 * 4];
    s.d = (s.d - multiplier) | large_buffer[(vi1 + vi2) % sizeof(large_buffer)];
    
    return s;
}

/* Pattern 4: Vector operations with builtins */
void vector_operations(void) {
    v4si v1 = {vi1, vi2, vi3, vi1 + vi2};
    v4si v2 = {vi2, vi3, vi1, vi2 + vi3};
    v4si v3, v4;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation requiring complex register allocation */
    v4si v5 = __builtin_shuffle(v3, v4, 
        (v4si){2, 3, 0, 1});
    
    /* Use results to prevent optimization */
    large_buffer[0] = ((char*)&v5)[0];
    large_buffer[1] = ((char*)&v5)[4];
}

/* Pattern 5: Control flow splitting live ranges */
int control_flow_split(int limit) {
    int x = vi1, y = vi2, z = vi3;
    int result = 0;
    
    /* Complex control flow with goto */
    if (x > 10) {
        y = x * 2;
        goto label1;
    } else {
        z = x + 5;
        goto label2;
    }
    
label1:
    for (int i = 0; i < limit; i++) {
        /* Use variables defined before goto */
        result += y + i;
        if (i % 7 == 0) {
            /* Another branch splitting live ranges */
            z = result * 3;
            goto label3;
        }
    }
    goto label4;
    
label2:
    result = z - x;
    for (int i = 0; i < limit; i += 2) {
        result += large_buffer[i] * i;
    }
    goto label4;
    
label3:
    result = result / (z + 1);
    /* Fall through */
    
label4:
    /* Final computation using all variables */
    return result + x + y + z;
}

/* Pattern 6: Mixed operations with volatile and non-addressable variables */
void mixed_volatile_operations(void) {
    volatile int vol1 = vi1;
    volatile int vol2 = vi2;
    int nonvol1, nonvol2;
    
    /* Operations mixing volatile and non-volatile */
    nonvol1 = vol1 * 3;
    
    /* Force address calculation reloads */
    int * volatile volatile_ptr = &large_buffer[vol1 * 64];
    nonvol2 = *volatile_ptr + vol2;
    
    /* Complex expression with multiple memory accesses */
    for (int i = 0; i < 32; i++) {
        large_buffer[(vol1 + i) % sizeof(large_buffer)] = 
            large_buffer[(vol2 * i) % sizeof(large_buffer)] + 
            large_buffer[(i * 3) % sizeof(large_buffer)];
    }
}

/* Main orchestrator */
int main(void) {
    int checksum = 0;
    
    /* Initialize large buffer with pattern */
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        large_buffer[i] = (i * 37) & 0xFF;
    }
    
    /* Execute all patterns */
    complex_addressing(50);
    checksum += vi1 + vi2 + vi3;
    
    inline_asm_chain(10);
    checksum += vi1 * 2;
    
    vector_operations();
    checksum += large_buffer[0] + large_buffer[100];
    
    mixed_volatile_operations();
    checksum += large_buffer[vi1] + large_buffer[vi2];
    
    /* Structure passing chain */
    struct SmallStruct s = {vi1, vi2, vi3, vi1 + vi2 + vi3};
    for (int i = 0; i < 5; i++) {
        s = process_struct(s);
        checksum += s.a + s.b + s.c + s.d;
    }
    
    /* Control flow with split live ranges */
    checksum += control_flow_split(25);
    
    /* Final checksum computation using array */
    for (int i = 0; i < 100; i++) {
        checksum += large_buffer[i * 7 % sizeof(large_buffer)];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
