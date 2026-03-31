Here's a comprehensive C++17 program designed to trigger the specific DWARF attributes in the uncovered lines:

```cpp
// Compile with: g++ -std=c++17 -g -O0 -fno-inline -fno-eliminate-unused-debug-types -pthread -o dwarf_test dwarf_test.cpp
// Also try: g++ -std=c++17 -g -O2 -fvar-tracking-assignments -fno-omit-frame-pointer -pthread -o dwarf_test dwarf_test.cpp

#include <iostream>
#include <optional>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <array>
#include <tuple>

// ==================== 1. C++11 explicit constructors and conversion operators ====================
class ExplicitInt {
    int value;
public:
    explicit ExplicitInt(int v) : value(v) {}
    explicit operator bool() const { return value != 0; }
    explicit operator int() const { return value; }
};

class ExplicitString {
    const char* str;
public:
    explicit ExplicitString(const char* s) : str(s) {}
    explicit operator const char*() const { return str; }
};

class MultiExplicit {
    int a, b;
public:
    explicit MultiExplicit(int x) : a(x), b(x) {}
    explicit MultiExplicit(int x, int y) : a(x), b(y) {}
    explicit operator float() const { return static_cast<float>(a + b); }
};

// ==================== 2. maybe_unused and optional attributes ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> get_optional_value(bool flag) {
    if (flag) return 42;
    return std::nullopt;
}

void process_with_unused_params([[maybe_unused]] int required, 
                                [[maybe_unused]] std::optional<int> opt) {
    [[maybe_unused]] auto local_unused = 3.14;
    // Structured binding with maybe_unused
    auto tuple = std::make_tuple(1, 2.0, "three");
    [[maybe_unused]] auto [x, y, z] = tuple;
}

// ==================== 3. Complex types with mutable and bit-fields ====================
struct MutableStruct {
    int normal;
    mutable int mutable_member;
    mutable double mutable_double;
    
    struct Inner {
        mutable int inner_mutable;
        int normal_inner;
    };
    
    mutable Inner inner;
    
    // Bit-fields
    unsigned int length : 8;
    unsigned int flags : 4;
    mutable unsigned int mutable_bits : 3;
};

union UnionWithMutable {
    int x;
    mutable double y;
    MutableStruct ms;
};

struct StringDescriptor {
    char* data;
    // Explicit length fields (not null-terminated)
    size_t string_length;
    size_t string_length_byte_size;
    unsigned int string_length_bit_size : 6;
    mutable size_t mutable_length;
};

// ==================== 4. Fortran-like array descriptors ====================
template<typename T, int DIM>
struct FortranArray {
    T* data;
    // Non-zero lower bounds
    int lower_bounds[DIM];
    int upper_bounds[DIM];
    // Ordering: 0=row-major (C), 1=column-major (Fortran)
    int ordering;
    // Segment information
    void* segment;
    size_t segment_size;
    
    T& element(const int indices[DIM]) {
        size_t offset = 0;
        if (ordering == 1) { // Column-major
            size_t stride = 1;
            for (int i = 0; i < DIM; ++i) {
                offset += (indices[i] - lower_bounds[i]) * stride;
                stride *= (upper_bounds[i] - lower_bounds[i] + 1);
            }
        } else { // Row-major
            size_t stride = 1;
            for (int i = DIM - 1; i >= 0; --i) {
                offset += (indices[i] - lower_bounds[i]) * stride;
                stride *= (upper_bounds[i] - lower_bounds[i] + 1);
            }
        }
        return data[offset];
    }
};

// ==================== 5. String types with explicit length and picture strings ====================
class ExplicitLengthString {
    char* buffer;
    size_t length;  // Explicit length, not null-terminated
    size_t capacity;
    
public:
    ExplicitLengthString(const char* str) {
        length = strlen(str);
        capacity = length + 1;
        buffer = new char[capacity];
        memcpy(buffer, str, length);
        buffer[length] = '\0';
    }
    
    size_t get_string_length() const { return length; }
    size_t get_string_length_byte_size() const { return sizeof(length); }
    
    ~ExplicitLengthString() { delete[] buffer; }
};

// Simulating COBOL picture clauses
class PictureString {
    char* picture;
    size_t length;
    mutable bool validated;
    
public:
    PictureString(const char* pic) : validated(false) {
        length = strlen(pic);
        picture = new char[length + 1];
        strcpy(picture, pic);
    }
    
    const char* get_picture_string() const { 
        validated = true;
        return picture; 
    }
    
    ~PictureString() { delete[] picture; }
};

struct FinancialRecord {
    PictureString amount_picture;
    ExplicitLengthString currency;
    mutable double exchange_rate;
    
    FinancialRecord() : amount_picture("999V.99"), currency("USD") {}
};

// ==================== 6. Function prototypes ====================
// Full prototypes
int fully_prototyped(int a, double b, const char* c);
void another_prototype(float x, float y, float z);

// Old style declaration (no prototype) - will be defined with prototype
int old_style();

// ==================== 7. Scaled thread-local storage ====================
thread_local int thread_specific = 0;
thread_local std::array<int, 100> thread_local_array;
thread_local double thread_scaled_double = 1.0;

std::atomic<int> global_counter{0};

void thread_function(int thread_id) {
    // Dynamic initialization of thread_local
    thread_specific = thread_id * 100;
    
    // Scaled access pattern
    for (int i = 0; i < 100; ++i) {
        // Scale index based on thread_id
        int scaled_index = (i * thread_id) % 100;
        thread_local_array[scaled_index] = thread_id + i;
        
        // More complex scaling calculation
        thread_scaled_double = thread_scaled_double * 1.01 + thread_id * 0.1;
    }
    
    global_counter.fetch_add(thread_specific);
}

// ==================== Function definitions ====================
int fully_prototyped(int a, double b, const char* c) {
    return a + static_cast<int>(b) + strlen(c);
}

void another_prototype(float x, float y, float z) {
    [[maybe_unused]] float result = x * y + z;
}

int old_style() {
    return 42;  // Defined with full prototype (implied by previous declaration)
}

// ==================== Main test driver ====================
int main() {
    unsigned long long hash = 0;
    
    // 1. Test explicit constructors and conversions
    ExplicitInt ei(42);
    ExplicitString es("test");
    MultiExplicit me(10);
    
    // Explicit conversions (implicit would fail)
    bool b = static_cast<bool>(ei);
    int i = static_cast<int>(ei);
    const char* s = static_cast<const char*>(es);
    float f = static_cast<float>(me);
    
    hash += i + b + reinterpret_cast<uintptr_t>(s) + static_cast<unsigned long long>(f);
    
    // 2. Test maybe_unused and optional
    auto opt1 = get_optional_value(true);
    auto opt2 = get_optional_value(false);
    
    process_with_unused_params(1, opt1);
    
    if (opt1) hash += *opt1;
    if (!opt2) hash += 1000;
    
    // 3. Test mutable and bit-fields
    MutableStruct ms;
    ms.normal = 1;
    ms.mutable_member = 2;  // Can modify even if const
    ms.mutable_double = 3.14;
    ms.inner.inner_mutable = 4;
    ms.length = 255;
    ms.flags = 7;
    ms.mutable_bits = 3;
    
    UnionWithMutable uwm;
    uwm.ms = ms;
    uwm.y = 2.718;  // Modifying mutable member in union
    
    StringDescriptor sd;
    sd.data = new char[100];
    sd.string_length = 50;
    sd.string_length_byte_size = sizeof(size_t);
    sd.string_length_bit_size = 32;
    sd.mutable_length = 50;
    
    hash += ms.mutable_member + static_cast<int>(uwm.y) + sd.string_length;
    
    // 4. Test Fortran-like arrays
    FortranArray<int, 3> fa;
    int data[100] = {0};
    fa.data = data;
    fa.lower_bounds[0] = -5;  // Non-zero lower bound
    fa.lower_bounds[1] = 1;
    fa.lower_bounds[2] = 0;
    fa.upper_bounds[0] = 5;
    fa.upper_bounds[1] = 10;
    fa.upper_bounds[2] = 3;
    fa.ordering = 1;  // Column-major (Fortran)
    fa.segment = data;
    fa.segment_size = sizeof(data);
    
    int indices[3] = {-5, 1, 0};
    fa.element(indices) = 42;
    hash += fa.element(indices);
    
    // 5. Test string types
    ExplicitLengthString els("Hello, World!");
    PictureString ps("999V.99");
    FinancialRecord fr;
    
    hash += els.get_string_length();
    hash += reinterpret_cast<uintptr_t>(ps.get_picture_string());
    hash += static_cast<unsigned long long>(fr.exchange_rate);
    
    // 6. Test function prototypes
    hash += fully_prototyped(1, 2.0, "three");
    another_prototype(1.0f, 2.0f, 3.0f);
    hash += old_style();
    
    // 7. Test thread-local scaled storage
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back(thread_function, t + 1);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Access thread_local from main thread (different initialization)
    thread_specific = 999;
    for (int i = 0; i < 100; ++i) {
        thread_local_array[i] = i * 2;
        hash += thread_local_array[i];
    }
    
    hash += thread_specific + global_counter.load();
    
    // Cleanup
    delete[] sd.data;
    
    std::cout << "Final hash: " << hash << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    
    return 0;
}
```

This program comprehensively addresses all the requirements:

1. **Explicit Constructors**: `ExplicitInt`, `ExplicitString`, and `MultiExplicit` classes with `explicit` constructors and conversion operators.

2. **Optional Attributes**: Uses `[[maybe_unused]]` on parameters, variables, and structured bindings, combined with `std::optional`.

3. **Mutable Members**: `MutableStruct` contains `mutable` members, nested structs with mutable, and bit-fields. `StringDescriptor` includes bit-size fields.

4. **Fortran-like Arrays**: `FortranArray` template with non-zero lower bounds, ordering attribute, and segment information.

5. **String Types**: `ExplicitLengthString` with explicit length fields and `PictureString` simulating COBOL picture clauses.

6. **Function Prototypes**: Mix of full prototypes and old-style declarations.

7. **Thread-Local Storage**: `thread_local` variables with dynamic initialization, scaled array access based on thread ID.

The `main()` function exercises all these constructs, performing calculations that generate a verifiable hash. The program should trigger the DWARF generator to emit debug information with the specific attributes in the uncovered lines when compiled with the recommended flags.
