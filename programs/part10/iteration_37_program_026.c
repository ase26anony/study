#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#include <optional>
#include <cstddef>

// For DW_AT_explicit and DW_AT_mutable
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    
    // For DW_AT_mutable
    mutable int mutable_counter;
    
    int getValue() const { 
        mutable_counter++;  // Modifying mutable in const method
        return value; 
    }
    
private:
    int value;
};

// For DW_AT_ordering - enum with non-sequential values
enum class OrderedEnum : int {
    First = 10,
    Second = 5,
    Third = 20,
    Fourth = 15
};

// For DW_AT_small - packed struct with bit-fields
struct __attribute__((packed)) SmallStruct {
    unsigned char tiny : 3;
    unsigned char small : 5;
    unsigned short medium : 9;
    int large;
};

// For DW_AT_picture_string - simulate COBOL-like picture string
struct PictureString {
    char data[32];
    // Simulating PICTURE "999V99"
    typedef char picture_type[6];
};

// For DW_AT_string_length family
struct StringWithLength {
    size_t length;
    size_t capacity;
    char data[];  // Flexible array member
};

// For DW_AT_prototyped - old-style K&R function
#ifdef __cplusplus
extern "C" {
#endif
int old_style_prototype();  // No parameter list
#ifdef __cplusplus
}
#endif

// For DW_AT_threads_scaled
extern thread_local int thread_local_array[100];

// For DW_AT_lower_bound - simulate non-zero lower bound
struct ArrayWithLowerBound {
    int* data;
    int lower_bound;
    int upper_bound;
    
    int& operator[](int index) {
        return data[index - lower_bound];
    }
};

// Complex template for varied contexts
template<typename T>
class ComplexTemplate {
public:
    ComplexTemplate(T val) : value(val), optional_val(val) {}
    
    // For DW_AT_is_optional
    std::optional<T> optional_val;
    
    // Method with complex control flow for location info
    T process() {
        // Create complex scoping for DW_AT_location
        {
            register int temp __asm__("eax") = static_cast<int>(value);
            volatile int* ptr = &temp;
            // Force variable to have interesting location
            asm volatile("" : "+r"(temp));
            value = static_cast<T>(temp);
        }
        return value;
    }
    
private:
    T value;
};

// Function using segment attribute
#ifdef __x86_64__
#define SEGMENT_ATTR __attribute__((section(".my_segment")))
#else
#define SEGMENT_ATTR
#endif

SEGMENT_ATTR int segment_var = 42;

// Function declarations
void use_string_with_length(StringWithLength* str);
void init_array_with_lower_bound(ArrayWithLowerBound& arr, int lower, int upper);
void manipulate_complex_location();

#endif // DWARF_TEST_H
