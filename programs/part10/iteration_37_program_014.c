#ifndef TEST_DWARF_ATTRIBUTES_H
#define TEST_DWARF_ATTRIBUTES_H

#include <optional>
#include <cstddef>

// For DW_AT_explicit and DW_AT_mutable
class ExplicitConstructor {
private:
    int value;
public:
    explicit ExplicitConstructor(int v) : value(v) {}
    
    // For DW_AT_mutable
    mutable int mutable_counter;
    
    void increment() const { mutable_counter++; }
};

// For DW_AT_ordering - enum with non-sequential values
enum OrderedEnum {
    FIRST = 10,
    SECOND = 5,
    THIRD = 20,
    FOURTH = 1
};

// For DW_AT_small - packed struct with bit-fields
struct __attribute__((packed)) SmallPackedStruct {
    unsigned char tiny : 3;
    unsigned char small : 5;
    unsigned short medium : 9;
    int large;
    
    // Constructor to ensure it's used
    SmallPackedStruct() : tiny(1), small(2), medium(3), large(4) {}
};

// For DW_AT_lower_bound - simulate array with non-zero lower bound
template<int LOWER_BOUND, int SIZE>
struct ArrayWithLowerBound {
    int data[SIZE];
    
    // Access with conceptual lower bound
    int& operator[](int index) {
        return data[index - LOWER_BOUND];
    }
    
    const int& operator[](int index) const {
        return data[index - LOWER_BOUND];
    }
};

// For DW_AT_string_length family
struct StringWithLength {
    size_t length;
    size_t capacity;
    char data[]; // Flexible Array Member
    
    // Note: Actual allocation would need custom allocator
};

// For DW_AT_picture_string - simulate COBOL-like picture string
typedef char PICTURE[32];
struct CobolLike {
    PICTURE picture_string;
    
    CobolLike() {
        // Initialize with a pattern
        for (int i = 0; i < 32; i++) {
            picture_string[i] = '9'; // COBOL picture of all 9s
        }
        picture_string[31] = '\0';
    }
};

// For DW_AT_prototyped - old-style K&R function
#ifdef __cplusplus
extern "C" {
#endif

// Old K&R style prototype (parameter types in definition, not declaration)
int old_style_function();  // Declaration without parameter types

// Modern prototype for comparison
int modern_function(int a, int b);

#ifdef __cplusplus
}
#endif

// For DW_AT_segment
#ifdef __GNUC__
#define SECTION_ATTR __attribute__((section(".mysection")))
#else
#define SECTION_ATTR
#endif

// For DW_AT_threads_scaled
extern thread_local int thread_local_array[100];

// For DW_AT_is_optional
std::optional<int> create_optional(bool create);

// For complex DW_AT_location
int create_complex_location_variable();

#endif // TEST_DWARF_ATTRIBUTES_H
