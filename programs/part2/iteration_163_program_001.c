#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  // Forward declaration, never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 2000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14f;
double global_double = 2.71828;
long double global_ldouble = 1.41421356L;
_Bool global_bool = 1;
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic C structure
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

// Nested structure
struct OuterStruct {
    int id;
    struct InnerStruct {
        float data;
        char tag;
    } inner;
    SimpleStruct simple;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Structure with flexible array member (C only)
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    double data[];
};
#endif

// Anonymous structure
struct {
    int anonymous_id;
    float anonymous_data;
} anonymous_global;

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Base class with different access specifiers
class BaseClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() { private_data++; }
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    void virtual_func() override { derived_data = 3.14; }
};

// Multiple inheritance
class Interface1 {
public:
    virtual void method1() = 0;
};

class Interface2 {
public:
    virtual void method2() = 0;
};

class MultiInherit : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
};

// Complex template with multiple parameters
template<typename T, typename U, int N>
class ComplexTemplate {
    T array[N];
    U value;
public:
    ComplexTemplate(U init) : value(init) {}
};

// Instantiate templates
TemplateClass<int> template_int(42);
TemplateClass<double> template_double(3.14);
ComplexTemplate<float, int, 10> complex_template(100);
#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char string_value[16];
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;
};

#ifdef __cplusplus
// C++11 anonymous union
struct AnonymousUnionStruct {
    int tag;
    union {
        int x;
        double y;
        char z[8];
    };  // Anonymous union
};
#endif

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointer to function
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);

// Pointer to array
int (*array_ptr)[10];

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*member_func_ptr_t)();
// Pointer to data member
typedef int BaseClass::*data_member_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
// Various arrays
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double double_array[5][3];  // 2D array
char char_3d[2][3][4];      // 3D array

// Array of pointers
int* pointer_array[5];

// Array of structures
SimpleStruct struct_array[3];

// Incomplete array in structure (C only)
#ifndef __cplusplus
struct IncompleteArray {
    int count;
    int data[];
};
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

// Function using callback
void register_callback(callback_t cb, void* data) {
    CallbackContainer container = {cb, data};
}

// Actual callback function
void my_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ function objects and lambdas
std::function<int(int)> func_object = [](int x) { return x * x; };

// Template with callback
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

// Lambda with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions
auto simple_lambda = []() { return 42; };
auto capturing_lambda = [global_int, &global_double]() {
    return global_int + static_cast<int>(global_double);
};
auto mutable_lambda = [counter = 0]() mutable { return counter++; };

// std::initializer_list
auto init_list_example = {1, 2, 3, 4, 5};

// Structured bindings (C++17)
struct Point { int x; int y; };
auto structured_binding_example() {
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
    return x_coord + y_coord;
}

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#if __cplusplus >= 202002L
#include <coroutine>
struct SimpleAwaitable {
    struct promise_type {
        SimpleAwaitable get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleAwaitable coroutine_example() {
    co_return;
}
#endif

// Complex template instantiation with language features
template<typename T>
class LanguageFeatureContainer {
    T data;
public:
    LanguageFeatureContainer(std::initializer_list<T> init) {
        for (const auto& item : init) {
            data = item;  // Just use the last item
        }
    }
    
    auto process_with_lambda() {
        return [this](T multiplier) {
            return data * multiplier;
        };
    }
};

LanguageFeatureContainer<int> lang_container{1, 2, 3, 4, 5};
#endif

/* ========== Helper Functions ========== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void process_array(int* arr, int size, callback_t cb) {
    for (int i = 0; i < size; i++) {
        cb(arr[i], &arr[i]);
    }
}

#ifdef __cplusplus
void cpp_specific_operations() {
    // Use all C++ specific types
    DerivedClass derived;
    derived.virtual_func();
    
    MultiInherit multi;
    multi.method1();
    multi.method2();
    
    // Use templates
    auto result = template_int.get_data();
    auto lambda_result = lang_container.process_with_lambda()(2);
    
    // Use lambdas
    template_callback([](int x) { return x * 3; }, 10);
    
    // Use fold expression
    auto total = sum_all(1, 2.5, 3.7f, 4);
    
    // Use structured binding
    auto coord_sum = structured_binding_example();
    
    std::cout << "C++ operations completed\n";
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    // TYPE_UNDEFINED: Use pointer to incomplete type
    incomplete_ptr_t undefined_ptr = nullptr;
    
    // TYPE_SCALAR: Use all scalar types
    int local_int = global_int + 1;
    unsigned int local_uint = global_uint * 2;
    float local_float = global_float * 2.0f;
    double local_double = global_double / 2.0;
    
    // TYPE_STRING: Use string types
    const char* local_string = global_string;
    char local_buffer[100];
    
    // TYPE_STRUCT: Instantiate and use structures
    SimpleStruct simple = {1, 2.0, "test"};
    OuterStruct outer = {100, {3.14f, 'X'}, {2, 3.0, "inner"}};
    BitFieldStruct bits = {1, 3, 7, -100};
    PackedStruct packed = {'Z', 999, 1.234};
    
    // TYPE_UNION: Use unions
    DataUnion data_union;
    data_union.int_value = 42;
    UnionContainer union_container = {0, {.as_int = 100}};
    
    #ifdef __cplusplus
    AnonymousUnionStruct anon_union;
    anon_union.x = 42;
    #endif
    
    // TYPE_POINTER: Use various pointers
    int* local_ptr = &local_int;
    int** local_double_ptr = &local_ptr;
    
    // TYPE_ARRAY: Use arrays
    int local_array[5] = {5, 4, 3, 2, 1};
    qsort(local_array, 5, sizeof(int), compare_ints);
    
    // TYPE_CALLBACK: Use function pointers
    CallbackContainer cb_container = {my_callback, &local_int};
    process_array(local_array, 5, my_callback);
    
    // Use all types based on command line argument
    unsigned long hash = 0;
    
    switch (mode) {
        case 'A':
            hash += sizeof(simple);
            hash += outer.id;
            hash += bits.flag1;
            break;
        case 'B':
            hash += (unsigned long)local_ptr;
            hash += (unsigned long)double_int_ptr;
            break;
        case 'C':
            hash += int_array[0];
            hash += (int)local_float;
            break;
        case 'D':
            #ifdef __cplusplus
            cpp_specific_operations();
            hash += template_int.get_data();
            #endif
            break;
        default:
            hash = 0xDEADBEEF;
    }
    
    // Ensure all globals are referenced
    hash += global_char + global_short + (int)global_bool;
    
    // Print something to prevent optimization
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << "\n";
    #else
    printf("Hash: %lu\n", hash);
    #endif
    
    return (int)(hash % 256);
}
