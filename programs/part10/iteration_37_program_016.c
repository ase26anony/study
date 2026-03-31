#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#include <optional>
#include <cstddef>

// For DW_AT_explicit and DW_AT_mutable
class ExplicitMutableClass {
private:
    int regular_member;
    mutable int mutable_member;  // DW_AT_mutable
    
public:
    explicit ExplicitMutableClass(int x) : regular_member(x), mutable_member(0) {}  // DW_AT_explicit
    void modify() const { mutable_member++; }  // Can modify mutable in const context
};

// For DW_AT_ordering
enum OrderedEnum {
    VALUE_FIRST = 10,
    VALUE_SECOND = 5,    // Not in declaration order
    VALUE_THIRD = 20,
    VALUE_FOURTH = 15    // Creates non-sequential ordering
};

// For DW_AT_small (packed struct with bit-fields)
struct __attribute__((packed)) SmallPackedStruct {
    unsigned char tiny : 3;      // Very small bit-field
    signed char small : 5;       // Small signed bit-field
    unsigned int normal;
    unsigned char another_tiny : 2;
    
    // Constructor to ensure it's used
    SmallPackedStruct() : tiny(1), small(-2), normal(100), another_tiny(0) {}
};

// For DW_AT_string_length family
struct StringWithLength {
    size_t length;      // DW_AT_string_length_byte_size
    size_t bit_length;  // DW_AT_string_length_bit_size (if different)
    char data[];        // Flexible array member
};

// For DW_AT_picture_string (simulating COBOL-like type)
typedef char PictureString[20];  // Simple placeholder
struct CobolLike {
    PictureString picture;  // Could trigger DW_AT_picture_string in certain frontends
};

// For DW_AT_lower_bound (simulating non-zero lower bound)
template<int LOWER_BOUND, int UPPER_BOUND>
struct BoundedArray {
    int data[UPPER_BOUND - LOWER_BOUND + 1];
    
    int& operator[](int index) {
        return data[index - LOWER_BOUND];
    }
};

// For DW_AT_threads_scaled
#ifdef _OPENMP
extern int thread_scaled_array[100];
#pragma omp threadprivate(thread_scaled_array)
#endif

// For DW_AT_segment
#ifdef __x86_64__
#define SEGMENT_ATTR __attribute__((section(".my_segment")))
#else
#define SEGMENT_ATTR
#endif

// For DW_AT_prototyped (old-style K&R function)
#ifdef __cplusplus
extern "C" {
#endif
    int old_style_prototype();  // Declaration without parameter types
#ifdef __cplusplus
}
#endif

// For DW_AT_is_optional
std::optional<int> create_optional(bool create);

// For DW_AT_location (complex variable)
int create_complex_location();

#endif // DWARF_TEST_H
