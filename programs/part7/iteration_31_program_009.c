// main.cpp - Main driver program
#include <iostream>
#include <cstdarg>
#include <cstdint>

// Forward declarations from other modules
extern "C" void ada_module_init();
extern "C" int compute_checksum();

// Global variables with various attributes
__attribute__((section(".data_segment"))) int global_in_section = 42;
thread_local int thread_local_var = 100;
volatile int volatile_global = 200;
const int const_global = 300;

// Structure with mutable member (for DW_AT_mutable)
struct TestStruct {
    int normal;
    mutable int mutable_member;  // Should generate DW_AT_mutable
    volatile int volatile_member;
    const int const_member;
    
    TestStruct() : normal(1), mutable_member(2), volatile_member(3), const_member(4) {}
    
    // Explicit constructor (for DW_AT_explicit in C++ context)
    explicit TestStruct(int x) : normal(x), mutable_member(x+1), volatile_member(x+2), const_member(x+3) {}
};

// Class with explicit methods
class ExplicitClass {
public:
    // Explicit conversion operator
    explicit operator bool() const { return true; }
    
    // Explicit constructor
    explicit ExplicitClass(int) {}
    
    // Template with explicit specialization
    template<typename T>
    explicit ExplicitClass(T) {}
};

// Bit-field structure for string length attributes
struct StringInfo {
    unsigned int length : 16;           // Bit-sized length
    unsigned int byte_length : 24;      // Byte-sized length
    unsigned int : 8;                   // Padding
};

// Array with explicit bounds
struct BoundedArray {
    int data[10];
    int lower_bound = 0;  // Simulate lower bound attribute
    int upper_bound = 9;
};

// Variadic function with prototype (for DW_AT_prototyped)
int variadic_function(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

// Function with noreturn attribute
__attribute__((noreturn)) void never_returns() {
    std::terminate();
}

// Template with ordering (for DW_AT_ordering)
template<typename T>
struct Ordered {
    T value;
    
    bool operator<(const Ordered& other) const {
        return value < other.value;
    }
    
    bool operator>(const Ordered& other) const {
        return value > other.value;
    }
};

// Optional type simulation (for DW_AT_is_optional)
template<typename T>
struct Optional {
    T value;
    bool is_present;
    
    explicit Optional(T val) : value(val), is_present(true) {}
    Optional() : value(), is_present(false) {}
};

// Thread-related structure
struct ThreadData {
    int id;
    long threads_scaled;  // Potential for DW_AT_threads_scaled
};

// Picture string simulation (Ada-like)
struct PictureString {
    char format[50];
    int precision;
    int scale;
};

// Segment attribute using GNU extension
#ifdef __GNUC__
__attribute__((section(".custom_segment")))
#endif
int segment_var = 999;

// Main function that references everything
int main() {
    // Local variables with various attributes
    TestStruct ts1;
    TestStruct ts2(10);  // Explicit constructor call
    
    ExplicitClass ec1(42);
    ExplicitClass ec2 = static_cast<ExplicitClass>(42);
    
    StringInfo si = {100, 200};
    
    BoundedArray ba;
    for (int i = ba.lower_bound; i <= ba.upper_bound; i++) {
        ba.data[i] = i * 2;
    }
    
    // Use variadic function
    int var_sum = variadic_function("test", 1, 2, 3, 4, 5);
    
    // Use optional type
    Optional<int> opt1(42);
    Optional<int> opt2;
    
    // Thread local access
    thread_local_var += 1;
    
    // Volatile access
    volatile_global = volatile_global + 1;
    
    // Segment variable access
    segment_var = segment_var * 2;
    
    // Create thread data
    ThreadData td = {1, 1000};
    
    // Picture string
    PictureString ps = {"ZZZ,ZZZ,ZZ9.99", 2, 2};
    
    // Call Ada module if available
    #ifdef HAS_ADA
    ada_module_init();
    #endif
    
    // Compute checksum using all variables
    int checksum = 0;
    checksum += ts1.mutable_member;
    checksum += ts2.normal;
    checksum += si.length;
    checksum += si.byte_length;
    checksum += ba.data[0];
    checksum += var_sum;
    checksum += opt1.is_present ? opt1.value : 0;
    checksum += thread_local_var;
    checksum += volatile_global;
    checksum += segment_var;
    checksum += td.threads_scaled;
    checksum += global_in_section;
    
    // Ensure variables are used
    if (ec1) {
        checksum += 1;
    }
    
    // Prevent optimization
    asm volatile("" : : "r"(checksum) : "memory");
    
    std::cout << "Checksum: " << checksum << std::endl;
    
    // Call external checksum function
    int external_checksum = compute_checksum();
    std::cout << "External checksum: " << external_checksum << std::endl;
    
    return checksum % 256;
}
