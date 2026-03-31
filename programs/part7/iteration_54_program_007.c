/* reload_stress_test.c
 * Designed to trigger various reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fno-inline reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
void complex_addressing(int size) {
    volatile int idx1 = 3, idx2 = 7;
    int arr[100][100];
    static int counter = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j + counter;
        }
    }
    
    /* Complex addressing that can't be folded */
    for (int i = 0; i < size; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_OUTPUT_ADDRESS */
        arr[idx1 + i][idx2 * 2] = arr[idx2][idx1 * i + 1];
        
        /* More complex addressing */
        int* ptr = &arr[i][0];
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        ptr[idx1 * idx2 + i] = ptr[idx2 * i + idx1];
    }
    
    counter++;
}

/* Pattern 2: Structure passing by value */
struct SmallStruct {
    int a, b, c, d;
};

struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    /* Operations that might need reloads */
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.d;
    result.d = s1.d / (s2.c ? s2.c : 1);
    return result;
}

struct SmallStruct chain_struct_ops(struct SmallStruct s) {
    struct SmallStruct temp1, temp2;
    
    /* Chain of structure operations */
    temp1 = process_struct(s, s);
    temp2 = process_struct(temp1, s);
    
    /* RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
    return process_struct(temp2, temp1);
}

/* Pattern 3: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void vector_operations() {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3, vec4;
    
    /* Vector operations that might need decomposition */
    vec3 = vec1 + vec2;
    vec4 = vec1 * vec2;
    
    /* Shuffle operation - complex pattern */
    v4si shuffle_mask = {3, 2, 1, 0};
    vec3 = __builtin_shuffle(vec3, vec4, shuffle_mask);
    
    /* Store to memory with complex addressing */
    volatile int storage[16];
    for (int i = 0; i < 4; i++) {
        storage[i * 4] = vec3[i];
    }
}

/* Pattern 4: Inline assembly with register constraints */
void inline_asm_stress() {
    int a = 1, b = 2, c = 3, d = 4;
    int result1, result2, result3;
    
    /* Chain of asm blocks creating dependencies */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]"
        : [out1] "=r" (result1)
        : [in1] "r" (a), [in2] "r" (b)
        : "cc"
    );
    
    /* Second asm using result from first */
    asm volatile (
        "imul %[in1], %[out1]\n\t"
        "sub %[in2], %[out1]"
        : [out1] "=r" (result2)
        : [in1] "r" (result1), [in2] "r" (c)
        : "cc"
    );
    
    /* Third asm with memory constraint */
    asm volatile (
        "addl $1, %[mem]\n\t"
        "mov %[mem], %[out]"
        : [out] "=r" (result3), [mem] "+m" (d)
        :
        : "cc"
    );
    
    /* Use results to prevent optimization */
    volatile int sink = result1 + result2 + result3;
    (void)sink;
}

/* Pattern 5: Control flow splitting live ranges */
int control_flow_live_ranges(int n) {
    int x = 0, y = 0, z = 0;
    volatile int condition = 1;
    
    /* Complex control flow */
    if (condition) {
        x = n * 2;
        y = n + 1;
        goto middle;
    } else {
        x = n / 2;
        y = n - 1;
    }
    
    /* Unreachable but compiler doesn't know */
    z = x * y;
    
middle:
    /* Use values defined in different blocks */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x += y;
        } else {
            y += x;
        }
        
        /* RELOAD_FOR_OTHER_ADDRESS */
        int* ptr = condition ? &x : &y;
        *ptr += i;
        
        /* Jump creates more control flow complexity */
        if (i == n/2) {
            goto skip;
        }
        
        z += *ptr;
        continue;
        
    skip:
        z -= *ptr;
    }
    
    return x + y + z;
}

/* Pattern 6: Mixed patterns in loops */
void mixed_patterns_in_loop(int iterations) {
    struct SmallStruct ss = {1, 2, 3, 4};
    int arr[50][50];
    volatile int idx = 5;
    
    /* Initialize */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i][j] = i * 50 + j;
        }
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix different patterns */
        complex_addressing(10);
        
        /* Structure operations */
        ss = chain_struct_ops(ss);
        
        /* Array with volatile index */
        int temp = arr[idx][iter % 50];
        arr[iter % 50][idx] = temp + ss.a;
        
        /* Inline asm */
        int asm_result;
        asm volatile (
            "mov %[in], %[out]\n\t"
            "add $1, %[out]"
            : [out] "=r" (asm_result)
            : [in] "r" (temp)
            : "cc"
        );
        
        /* Use result */
        arr[(iter + 1) % 50][idx] = asm_result;
    }
}

/* Main orchestrator */
int main() {
    int checksum = 0;
    volatile int seed = 42;
    
    printf("Starting reload stress test...\n");
    
    /* Pattern 1: Complex addressing */
    complex_addressing(20);
    
    /* Pattern 2: Structure passing */
    struct SmallStruct s1 = {seed, seed + 1, seed + 2, seed + 3};
    struct SmallStruct s2 = chain_struct_ops(s1);
    checksum += s2.a + s2.b + s2.c + s2.d;
    
    /* Pattern 3: Vector operations */
    vector_operations();
    
    /* Pattern 4: Inline assembly */
    inline_asm_stress();
    
    /* Pattern 5: Control flow */
    checksum += control_flow_live_ranges(50);
    
    /* Pattern 6: Mixed patterns */
    mixed_patterns_in_loop(5);
    
    /* Final array checksum to prevent optimization */
    int final_arr[100];
    for (int i = 0; i < 100; i++) {
        final_arr[i] = i + checksum;
        checksum = (checksum * 31 + final_arr[i]) & 0xFFFF;
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
