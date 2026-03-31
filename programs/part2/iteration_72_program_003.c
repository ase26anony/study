#include <stdio.h>
#include <stdlib.h>

/* Helper function 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        // This generates memory access with zero offset: arr[0]
        sum += ptr[0];  // Line should trigger mem_insn setup with offset 0
        ptr++;          // Post-access pointer increment
    }
    return sum;
}

/* Helper function 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *dptr, int n) {
    double sum = 0.0;
    double *ptr = dptr + n;  // Start at end
    
    for (int i = 0; i < n; i++) {
        --ptr;               // Pre-decrement before access
        // *ptr is equivalent to ptr[0] with zero offset
        sum += *ptr;         // Should trigger mem_insn setup
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int value;
    char padding[60];
    long extra;
};

long func_struct_first_member(struct Data *data, int n) {
    long sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        // Access first member - offset 0 in structure
        sum += ptr->value;  // Should trigger mem_insn setup
        ptr++;              // Pointer arithmetic
    }
    return sum;
}

/* Helper function 4: Different data types and sizes */
void func_mixed_types(char *cptr, short *sptr, int *iptr, long *lptr, int n) {
    volatile int result = 0;  // Volatile to prevent elimination
    
    for (int i = 0; i < n; i++) {
        // Different memory modes: QImode, HImode, SImode, DImode
        result += cptr[0];  // char - QImode
        result += sptr[0];  // short - HImode
        result += iptr[0];  // int - SImode
        result += lptr[0];  // long - DImode
        
        cptr++;
        sptr++;
        iptr++;
        lptr++;
    }
}

/* Helper function 5: While loop with pointer arithmetic in body */
float func_while_loop(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    int count = n;
    
    while (count-- > 0) {
        sum += ptr[0];  // Zero offset access
        ptr++;          // Arithmetic in loop body
    }
    return sum;
}

/* Helper function 6: Do-while loop ensuring at least one execution */
short func_do_while(short *arr, int n) {
    short sum = 0;
    short *ptr = arr;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  // Zero offset
        ptr++;
        n--;
    } while (n > 0);
    
    return sum;
}

/* Helper function 7: Memory access and arithmetic separated by trivial code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  // Memory access with zero offset
        
        // Small independent calculation between access and increment
        int dummy = i * 2;  // Trivial code that shouldn't prevent optimization
        
        sum += temp;
        ptr++;              // Pointer increment after independent code
    }
    return sum;
}

/* Helper function 8: Nested access patterns */
void func_nested_patterns(int *arr1, int *arr2, int n) {
    volatile int result = 0;
    
    for (int i = 0; i < n; i++) {
        // Multiple zero-offset accesses in same basic block
        result += arr1[0];
        result += arr2[0];
        
        // Both pointers incremented
        arr1++;
        arr2++;
    }
}

/* Helper function 9: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset
        
        // Conditional that compiler can determine is always true
        if (i < n) {    // Always true in loop
            ptr++;      // Pointer increment in conditional block
        }
    }
    return sum;
}

/* Helper function 10: Floating point with different precisions */
double func_float_ops(float *fptr, double *dptr, int n) {
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        sum += fptr[0];  // SFmode access
        sum += dptr[0];  // DFmode access
        
        fptr++;
        dptr++;
    }
    return sum;
}

int main() {
    // Initialize arrays with different types
    int int_arr[100];
    double double_arr[100];
    char char_arr[100];
    short short_arr[100];
    long long_arr[100];
    float float_arr[100];
    struct Data struct_arr[50];
    
    // Initialize data
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        char_arr[i] = i % 128;
        short_arr[i] = i * 2;
        long_arr[i] = i * 100L;
        float_arr[i] = i * 0.75f;
    }
    
    for (int i = 0; i < 50; i++) {
        struct_arr[i].value = i * 3;
        struct_arr[i].extra = i * 200L;
    }
    
    volatile int total = 0;  // Volatile to prevent dead code elimination
    
    // Call all functions multiple times with different patterns
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(int_arr, 50);
        total += (int)func_ptr_deref_predec(double_arr, 50);
        total += (int)func_struct_first_member(struct_arr, 25);
        
        func_mixed_types(char_arr, short_arr, int_arr, long_arr, 25);
        
        total += (int)func_while_loop(float_arr, 50);
        total += func_do_while(short_arr, 50);
        total += func_separated_ops(int_arr, 50);
        
        func_nested_patterns(int_arr, int_arr + 50, 25);
        total += func_always_taken(int_arr, 50);
        total += (int)func_float_ops(float_arr, double_arr, 25);
    }
    
    // Print result to ensure code isn't eliminated
    printf("Result checksum: %d\n", total);
    
    return 0;
}
