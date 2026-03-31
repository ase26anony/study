#include <stddef.h>
#include <string.h>

// Helper to create compile-time constant indices
template<int N>
constexpr int constant_index() { return N; }

// Volatile wrapper to inhibit early optimization
volatile int volatile_seed = 0;

// Memory target operations (MEM_P path)
void memory_target_operations(char* dest, const char* src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Case 1: count <= 2 (should take MEM_P && count <= 2 path)
    if (count <= 2) {
        // Small constant-sized copy
        for (int i = lo; i <= hi; ++i) {
            dest[i] = src[i];
        }
    }
    // Case 2: count > 2 but small total size
    else if (count == 3) {
        // Use builtin memcpy with constant size
        __builtin_memcpy(dest + lo, src + lo, 3 * sizeof(char));
    }
}

// Non-memory target operations (!MEM_P path)
int non_memory_target_operations(const int* arr, int lo, int hi) {
    int result = 0;
    // This creates a non-MEM_P target (register computation)
    for (int i = lo; i <= hi; ++i) {
        result += arr[i];
    }
    return result;
}

int main(int argc, char* argv[]) {
    // Arrays of different types to test different TYPE_SIZE values
    char char_array[100] = {0};
    int int_array[100] = {0};
    long long ll_array[100] = {0};
    
    // Initialize with some values
    for (int i = 0; i < 100; ++i) {
        char_array[i] = i % 26 + 'a';
        int_array[i] = i * 2;
        ll_array[i] = i * 1000LL;
    }
    
    // Use volatile pointers to prevent early folding
    volatile char* volatile_char = char_array;
    volatile int* volatile_int = int_array;
    volatile long long* volatile_ll = ll_array;
    
    int result = 0;
    
    // Use argc to create different control flow paths
    switch (argc % 4) {
        case 0: {
            // Path 1: MEM_P with count = 1 (single element)
            int lo = constant_index<5>();
            int hi = constant_index<5>();
            char temp[100];
            memory_target_operations(temp, char_array, lo, hi);
            result += temp[5];
            break;
        }
        
        case 1: {
            // Path 2: MEM_P with count = 2 (two elements)
            int lo = constant_index<10>();
            int hi = constant_index<11>();
            char temp[100];
            memory_target_operations(temp, char_array, lo, hi);
            result += temp[10] + temp[11];
            break;
        }
        
        case 2: {
            // Path 3: MEM_P with count = 3 (small type, small total size)
            int lo = constant_index<20>();
            int hi = constant_index<22>();
            char temp[100];
            memory_target_operations(temp, char_array, lo, hi);
            result += temp[20] + temp[21] + temp[22];
            break;
        }
        
        case 3: {
            // Path 4: !MEM_P path (register computation)
            int lo = constant_index<30>();
            int hi = constant_index<35>();
            result += non_memory_target_operations(int_array, lo, hi);
            
            // Also test with larger type where TYPE_SIZE * count might be larger
            long long ll_result = 0;
            for (int i = 40; i <= 45; ++i) {
                ll_result += ll_array[i];
            }
            result += (int)(ll_result % 1000);
            break;
        }
    }
    
    // Additional test with volatile indices to ensure tree nodes are created
    volatile int vlo = 50;
    volatile int vhi = 55;
    
    // This should still be analyzed as constant bounds due to the values
    if (volatile_seed == 0) {  // Always true, but compiler doesn't know
        int actual_lo = 50;
        int actual_hi = 55;
        
        // Mixed operation: part memory, part register
        int sum = 0;
        for (int i = actual_lo; i <= actual_hi; ++i) {
            // Memory load followed by register operation
            sum += int_array[i];
            // Memory store
            char_array[i % 10] = (char)(sum % 26);
        }
        result += sum;
    }
    
    // Force use of __builtin_constant_p to verify constant propagation
    if (__builtin_constant_p(constant_index<60>())) {
        // Access with compile-time constant bounds
        int val = int_array[constant_index<60>()];
        result += val;
    }
    
    // Array slice copy with constant bounds
    char src_slice[10] = "abcdefghi";
    char dst_slice[10] = {0};
    
    // This should create constant bounds (2-5 inclusive, count=4)
    int slice_lo = 2;
    int slice_hi = 5;
    for (int i = slice_lo; i <= slice_hi; ++i) {
        dst_slice[i - slice_lo] = src_slice[i];
    }
    result += dst_slice[0];
    
    return result;
}
