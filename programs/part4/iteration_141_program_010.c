/* Compile with: g++ -O2 -fno-omit-frame-pointer -fno-inline -fno-strict-aliasing -fPIC caller-save-test.cc -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_REGISTER(type, name, reg) register type name asm(reg)

// External functions that will clobber registers
NOINLINE void external_func1(int* p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

NOINLINE void external_func2(int* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

NOINLINE void external_func3(int* p) {
    *p -= 3;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

// Function pointer type
typedef void (*func_ptr_t)(int*);

// Global volatile to prevent optimizations
volatile int global_seed = 42;

// Main test function with complex register usage
NOINLINE int test_caller_save(int* data, int size) {
    // Use explicit register variables to create pressure on call-clobbered registers
    VOLATILE_REGISTER(long, r10_val, "r10") = global_seed;
    VOLATILE_REGISTER(long, r11_val, "r11") = global_seed * 2;
    VOLATILE_REGISTER(long, r9_val, "r9") = global_seed * 3;
    VOLATILE_REGISTER(long, r8_val, "r8") = global_seed * 4;
    
    // Mix of call-clobbered and call-saved register usage
    register long rbx_val asm("rbx") = global_seed * 5;  // call-saved
    register long r12_val asm("r12") = global_seed * 6;  // call-saved
    register long r13_val asm("r13") = global_seed * 7;  // call-saved
    
    // Array of function pointers for indirect calls
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int func_index = 0;
    
    int result = 0;
    
    // Outer loop - creates multiple basic blocks
    for (int outer = 0; outer < 3; ++outer) {
        // Nested inner loop with function calls
        for (int i = 0; i < size; ++i) {
            // Complex computation using multiple registers
            // that must survive across function calls
            
            // Load data into call-clobbered registers
            VOLATILE_REGISTER(int, temp1, "rax") = data[i];
            VOLATILE_REGISTER(int, temp2, "rcx") = data[(i + 1) % size];
            VOLATILE_REGISTER(int, temp3, "rdx") = data[(i + 2) % size];
            
            // Perform arithmetic in registers
            r10_val = (r10_val * temp1) + r11_val;
            r11_val = (r11_val * temp2) ^ r10_val;
            r9_val = (r9_val + temp3) | r8_val;
            r8_val = (r8_val - temp1) & r9_val;
            
            // Also use call-saved registers
            rbx_val += temp1;
            r12_val ^= temp2;
            r13_val |= temp3;
            
            // Create artificial dependencies with inline assembly
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r9_val), "+r"(r8_val));
            asm volatile("" : "+r"(rbx_val), "+r"(r12_val), "+r"(r13_val));
            
            // Call external function via function pointer
            // This should trigger caller-save insertion
            int* volatile ptr = &data[i];
            funcs[func_index](ptr);
            
            // Switch function pointer for next iteration
            func_index = (func_index + 1) % 3;
            
            // Use the register values after the call
            // This forces them to be live across the call
            temp1 = (int)(r10_val & 0xFFFFFFFF);
            temp2 = (int)(r11_val & 0xFFFFFFFF);
            temp3 = (int)(r9_val & 0xFFFFFFFF);
            
            // More computations mixing all registers
            rbx_val = rbx_val + r10_val - r11_val;
            r12_val = r12_val ^ r9_val ^ r8_val;
            r13_val = r13_val | r10_val | r11_val;
            
            // Store results back, creating memory dependencies
            data[i] = (int)((rbx_val + r12_val + r13_val) & 0xFFFFFFFF);
            
            // Update result
            result += data[i];
            
            // Force register spilling with volatile
            asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r9_val), "r"(r8_val),
                         "r"(rbx_val), "r"(r12_val), "r"(r13_val));
        }
        
        // Additional computation between outer loop iterations
        // to create more basic block boundaries
        if (outer % 2 == 0) {
            // Another indirect call
            func_ptr_t fp = (global_seed & 1) ? external_func1 : external_func2;
            fp(&result);
        }
    }
    
    // Final mixing of all register values
    result += (int)((r10_val + r11_val + r9_val + r8_val + 
                    rbx_val + r12_val + r13_val) & 0xFFFFFFFF);
    
    return result;
}

// Another test function with different pattern
NOINLINE int test_caller_save2(int* data, int size) {
    VOLATILE_REGISTER(long, rax_val, "rax") = global_seed;
    VOLATILE_REGISTER(long, rcx_val, "rcx") = global_seed + 1;
    VOLATILE_REGISTER(long, rdx_val, "rdx") = global_seed + 2;
    
    register long r14_val asm("r14") = global_seed * 11;  // call-saved
    register long r15_val asm("r15") = global_seed * 13;  // call-saved
    
    int sum = 0;
    
    // Different loop structure
    for (int i = 0; i < size; i += 2) {
        // Load and compute
        int val1 = data[i];
        int val2 = data[i + 1];
        
        rax_val = (rax_val * val1) + rcx_val;
        rcx_val = (rcx_val * val2) + rdx_val;
        rdx_val = (rdx_val + val1) * val2;
        
        r14_val += rax_val;
        r15_val += rcx_val;
        
        // Call with live registers
        external_func3(&data[i]);
        
        // Use values after call
        sum += (int)((rax_val + rcx_val + rdx_val) & 0xFF);
        sum += (int)((r14_val + r15_val) & 0xFF);
        
        // Force another call with different pattern
        if (i % 4 == 0) {
            external_func1(&sum);
        }
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i * 3 + 1;
    }
    
    // Run first test
    int result1 = test_caller_save(data, SIZE);
    printf("Result 1: %d\n", result1);
    
    // Re-initialize
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i * 5 + 2;
    }
    
    // Run second test
    int result2 = test_caller_save2(data, SIZE);
    printf("Result 2: %d\n", result2);
    
    // Final computation mixing both results
    int final_result = result1 ^ result2;
    printf("Final result: %d\n", final_result);
    
    free(data);
    return final_result != 0 ? 0 : 1;
}
