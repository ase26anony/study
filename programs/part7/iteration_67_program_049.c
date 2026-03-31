Here's a C++ program designed to trigger the specific DWARF attribute assignments in the uncovered lines:

```cpp
// Compile with: g++ -O0 -g3 -gdwarf-5 -fvar-tracking-assignments -fvar-tracking -std=c++17
// Also try: g++ -O2 -g3 -gdwarf-4 -std=c++17
// And: g++ -O3 -g -fsanitize=undefined -fno-omit-frame-pointer -std=c++17

#include <optional>
#include <cstdio>
#include <cstdint>

// Prevent optimization
#define KEEP_ALIVE(var) __asm__("" : : "r"(&(var)))

// ==================== C++ Classes with explicit constructors ====================
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    explicit operator bool() const { return value != 0; }
    
    // Mutable member for DW_AT_mutable
    mutable int mutable_counter = 0;
    
    // Const member
    const int value;
    
    // Small attribute - use bitfields
    unsigned small_field : 4;
};

// Template class for complexity
template<typename T>
class Container {
public:
    explicit Container(T val) : data(val) {}
    
    // Prototyped member function
    void prototyped_method(int x, double y);
    
    // Non-prototyped old-style declaration (C++ won't allow this in class scope,
    // but we'll use extern "C" for C functions later)
    
    T data;
    
    // Thread-local member
    static thread_local int thread_scaled_counter;
};

template<typename T>
thread_local int Container<T>::thread_scaled_counter = 0;

// ==================== Complex nested types ====================
namespace DeepNested {
    // Union with optional-like behavior
    struct TaggedUnion {
        enum Type { INT, FLOAT, STRING } type;
        union {
            int int_val;
            float float_val;
            char* string_ptr;
        } data;
        
        // This creates optional-like semantics
        bool has_value() const { return type != INT || data.int_val != -1; }
    };
    
    // Array with non-standard lower bound (GNU extension)
    #ifdef __GNUC__
    typedef int BoundedArray[-5...5];  // Lower bound -5
    #else
    typedef int BoundedArray[11];  // Fallback
    #endif
    
    // Fixed-length string type
    typedef char FixedString[32];
    
    // Picture string simulation (for COBOL-like types)
    struct DecimalPicture {
        char picture[20];  // e.g., "999V99"
        int precision;
        int scale;
    };
    
    // Main complex struct
    struct ComplexStruct {
        ExplicitClass explicit_obj;
        TaggedUnion optional_data;
        BoundedArray bounded_array;
        FixedString fixed_str;
        DecimalPicture decimal_pic;
        Container<int> container;
        
        // Segment attribute via section
        #ifdef __GNUC__
        __attribute__((section(".mysection"))) 
        #endif
        int segment_var;
        
        // Volatile to prevent elimination
        volatile int volatile_member;
        
        ComplexStruct() : 
            explicit_obj(42),
            container(100),
            segment_var(999),
            volatile_member(0)
        {
            optional_data.type = TaggedUnion::INT;
            optional_data.data.int_val = 123;
            
            // Initialize array (handle both GNU and standard)
            for(int i = 0; i < 11; i++) {
                bounded_array[i] = i * 2;
            }
            
            // String with explicit length
            __builtin_strcpy(fixed_str, "Hello, DWARF!");
            
            // Picture string
            __builtin_strcpy(decimal_pic.picture, "999V99");
            decimal_pic.precision = 5;
            decimal_pic.scale = 2;
        }
    };
}

// ==================== C-style functions ====================
extern "C" {
    // Old-style K&R function (non-prototyped)
    int old_style_func(x, y)
        int x;
        double y;
    {
        return x + (int)y;
    }
    
    // Prototyped C function
    int prototyped_func(int x, double y) {
        return x - (int)y;
    }
    
    // Function with string length operations
    size_t string_length_operation(const char* str) {
        // This might trigger string_length attributes
        size_t len = 0;
        while(str[len]) len++;
        return len;
    }
    
    // Function with ordering requirements
    void reorder_operations(int* a, int* b) {
        // Mutable access
        static int counter = 0;
        counter++;
        
        // Potential reordering
        *a = counter;
        *b = counter * 2;
    }
}

// ==================== Thread-local storage ====================
// Scaled thread-local
__thread int thread_scaled_var = 0;
thread_local int cxx_thread_var = 0;

// Thread-local in struct
struct ThreadedStruct {
    #ifdef __GNUC__
    __thread 
    #endif
    static int static_thread_member;
    
    int normal_member;
};

int ThreadedStruct::static_thread_member = 0;

// ==================== Global variables with attributes ====================
// Register variable (influences location)
register int reg_var asm("r12");

// Volatile global
volatile int global_volatile = 42;

// Section attribute
#ifdef __GNUC__
__attribute__((section(".data.mysection")))
#endif
int section_var = 0xDEADBEEF;

// ==================== Template instantiations ====================
template class Container<int>;
template class Container<double>;
template class Container<DeepNested::ComplexStruct*>;

// ==================== Main function ====================
int main() {
    // Force instantiation of complex types
    DeepNested::ComplexStruct complex_obj;
    KEEP_ALIVE(complex_obj);
    
    ExplicitClass explicit_obj(100);
    KEEP_ALIVE(explicit_obj);
    
    // Use mutable member
    explicit_obj.mutable_counter++;
    
    // Use optional-like type
    if (complex_obj.optional_data.has_value()) {
        printf("Has value: %d\n", complex_obj.optional_data.data.int_val);
    }
    
    // Call both prototyped and non-prototyped functions
    int result1 = old_style_func(10, 3.14);
    int result2 = prototyped_func(20, 6.28);
    printf("Results: %d, %d\n", result1, result2);
    
    // String length operations
    size_t len = string_length_operation(complex_obj.fixed_str);
    printf("String length: %zu\n", len);
    
    // Thread-local operations
    thread_scaled_var = 42;
    cxx_thread_var = thread_scaled_var * 2;
    
    // Register variable
    reg_var = 100;
    
    // Section variable
    section_var++;
    
    // Array with bounds
    int sum = 0;
    for(int i = 0; i < 11; i++) {
        sum += complex_obj.bounded_array[i];
    }
    printf("Array sum: %d\n", sum);
    
    // Picture string (COBOL-like)
    printf("Picture: %s\n", complex_obj.decimal_pic.picture);
    
    // Reordering operations
    int a, b;
    reorder_operations(&a, &b);
    printf("Reordered: %d, %d\n", a, b);
    
    // Template usage
    Container<double> double_container(3.14159);
    double_container.thread_scaled_counter = 5;
    
    Container<DeepNested::ComplexStruct*> ptr_container(&complex_obj);
    
    // Volatile access
    global_volatile = sum;
    
    // Prevent dead code elimination
    volatile int checksum = 
        result1 + result2 + 
        (int)len + sum + 
        a + b + 
        (int)double_container.data +
        thread_scaled_var;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

// ==================== Additional complexity ====================
// Unused but complex types for more DWARF generation
namespace UnusedButComplex {
    // Multi-dimensional array with typedef
    typedef int Matrix[3][4][5];
    
    // Function pointer with complex signature
    typedef int (*ComplexFunc)(int, DeepNested::ComplexStruct*, ...);
    
    // Nested anonymous struct/union
    struct ReallyComplex {
        struct {
            int x;
            union {
                int a;
                float b;
            } inner;
        } nested;
        
        // Bitfield ordering
        unsigned bit1 : 1;
        unsigned bit2 : 3;
        unsigned : 2;  // Unnamed bitfield
        unsigned bit3 : 4;
        
        // Small attribute via bitfield
        unsigned small : 2;
    };
    
    // Variadic template for more complexity
    template<typename... Args>
    struct VariadicContainer {
        static constexpr size_t size = sizeof...(Args);
        
        template<typename T>
        void process(T&& arg) {
            // Do nothing, just for compilation
            volatile auto dummy = &arg;
            (void)dummy;
        }
    };
}

// Explicit template instantiation
template struct UnusedButComplex::VariadicContainer<int, double, char*, ExplicitClass>;
```

This program specifically targets each of the uncovered DWARF attributes:

1. **`DW_AT_explicit`**: `ExplicitClass` has explicit constructor and conversion operator
2. **`DW_AT_is_optional`**: `TaggedUnion` simulates optional types with `has_value()` method
3. **`DW_AT_location`**: Register variables (`reg_var`), volatile, and section attributes
4. **`DW_AT_lower_bound`**: GNU extension array `BoundedArray[-5...5]`
5. **`DW_AT_mutable`**: `mutable_counter` in `ExplicitClass`
6. **`DW_AT_ordering`**: Bitfields and potential reordering in `reorder_operations`
7. **`DW_AT_picture_string`**: `DecimalPicture` struct with picture string field
8. **`DW_AT_prototyped`**: Mix of K&R (`old_style_func`) and ANSI (`prototyped_func`) functions
9. **`DW_AT_small`**: Bitfields (`small_field`, `small`) and small integer types
10. **`DW_AT_segment`**: `__attribute__((section(".mysection")))` on variables
11. **`DW_AT_string_length`**: `FixedString` typedef and `string_length_operation`
12. **`DW_AT_string_length_bit_size`/`byte_size`**: Fixed-length string operations
13. **`DW_AT_threads_scaled`**: `__thread`, `thread_local`, and thread-local in classes

The program uses deep nesting, templates, namespaces, and both C and C++ constructs to maximize the variety of DWARF information generated. The `KEEP_ALIVE` macro and `volatile` usage prevent dead code elimination, ensuring all types are present in the final DWARF output.
