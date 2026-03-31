#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ==================== TYPE_UNDEFINED ==================== */
struct incomplete_struct;  // Forward declaration, never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ==================== TYPE_SCALAR ==================== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 5000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.618033988749895L;
#ifdef __cplusplus
bool global_bool = true;
#endif

/* ==================== TYPE_STRING ==================== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ==================== TYPE_STRUCT ==================== */
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char z;
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
    int data[];
};
#endif

// Global struct instances
struct SimpleStruct global_simple_struct = {1, 2.0, 'A'};
struct OuterStruct global_outer_struct = {100, {3.14f, 'B'}, {2, 4.0, 'C'}};

/* ==================== TYPE_USER_STRUCT (C++ only) ==================== */
#ifdef __cplusplus
// Base class with different access specifiers
class BaseClass {
public:
    BaseClass() : public_data(10) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "BaseClass::virtual_func\n"; }
    
    int public_data;
    
protected:
    float protected_data;
    
private:
    double private_data;
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : extra_data(20) {}
    void virtual_func() override { std::cout << "DerivedClass::virtual_func\n"; }
    
private:
    int extra_data;
};

// Multiple inheritance
class Interface1 {
public:
    virtual void interface1_func() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void interface2_func() = 0;
    virtual ~Interface2() {}
};

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    void interface1_func() override {}
    void interface2_func() override {}
};

// Template class
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
    
private:
    T data;
};

// Class with static members
class StaticMemberClass {
public:
    static int static_counter;
    static constexpr double PI = 3.14159;
    
    void increment() { instance_counter++; }
    
private:
    int instance_counter = 0;
};

int StaticMemberClass::static_counter = 0;

// Class with member functions of different types
class FunctionClass {
public:
    void regular_func() {}
    void const_func() const {}
    static void static_func() {}
    virtual void pure_virtual() = 0;
};

// Instantiate template classes
TemplateClass<int> template_int_instance(42);
TemplateClass<double> template_double_instance(3.14);
TemplateClass<SimpleStruct> template_struct_instance({5, 6.0, 'D'});
#endif

/* ==================== TYPE_UNION ==================== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char* string_value;
    struct SimpleStruct struct_value;
};

// Union within a structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        char char_data[20];
    } data;
};

// Anonymous union (C++11)
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    };
};
#endif

// Global union instances
union DataUnion global_union = {.int_value = 100};
struct UnionContainer global_union_container = {1, {.int_data = 42}};

/* ==================== TYPE_POINTER ==================== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
const volatile int* const_volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointer to incomplete type (triggers TYPE_UNDEFINED indirectly)
incomplete_ptr_t incomplete_ptr = nullptr;

// Pointers to structures
struct SimpleStruct* struct_ptr = &global_simple_struct;
struct OuterStruct* outer_struct_ptr = &global_outer_struct;

// Pointer to union
union DataUnion* union_ptr = &global_union;

// Pointer to array
int(*array_ptr)[10];

// Function pointers
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(const char*);

// Pointer to member function (C++)
#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
#endif

/* ==================== TYPE_ARRAY ==================== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];
double double_array[2][3][4];
char* string_array[] = {"one", "two", "three", nullptr};

// Array of structures
struct SimpleStruct struct_array[3] = {
    {1, 1.0, 'A'},
    {2, 2.0, 'B'},
    {3, 3.0, 'C'}
};

// Array of pointers
int* pointer_array[5];

// Incomplete array in struct (C only)
#ifndef __cplusplus
struct IncompleteArrayHolder {
    int count;
    int values[];
};
#endif

// Variable length array (C99)
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ==================== TYPE_CALLBACK ==================== */
// Function pointer types
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

// Structure with function pointer
struct CallbackHolder {
    const char* name;
    callback_t callback;
    void* user_data;
};

// Function using callback
void register_callback(callback_t cb, void* data) {
    // Simulate callback registration
    (void)cb;
    (void)data;
}

// Actual callback function
void my_callback(int value, void* data) {
    (void)value;
    (void)data;
}

#ifdef __cplusplus
// C++ callbacks with std::function
std::function<int(int)> cpp_callback;

// Template with callback
template<typename Func>
void template_callback(Func f) {
    f(42);
}

// Lambda as callback
auto lambda_callback = [](int x) -> int {
    return x * 2;
};
#endif

/* ==================== TYPE_LANG_STRUCT (C++ only) ==================== */
#ifdef __cplusplus
// Lambda expressions with different captures
auto lambda_with_capture = [global_int, &global_double](int x) -> double {
    return x + global_int + global_double;
};

auto lambda_by_value = [=](int x) { return x + global_int; };
auto lambda_by_reference = [&](int x) { return x + global_int; };
auto lambda_mutable = [global_int]() mutable { global_int = 100; };

// std::initializer_list usage
auto init_list_func(std::initializer_list<int> list) {
    int sum = 0;
    for (auto x : list) {
        sum += x;
    }
    return sum;
}

// Structured bindings (C++17)
struct Point { double x, y; };
auto get_point() -> Point { return {1.0, 2.0}; }

void use_structured_binding() {
    auto [x, y] = get_point();
    (void)x;
    (void)y;
}

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#if __cplusplus >= 202002L
#include <coroutine>
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleCoroutine simple_coro() {
    co_return;
}
#endif

// Variadic templates
template<typename... Ts>
struct VariadicStruct {};

// Alias template
template<typename T>
using AliasTemplate = T*;

// Template template parameter
template<template<typename> class Container, typename T>
class TemplateTemplateClass {
    Container<T> data;
};
#endif

/* ==================== Function Definitions ==================== */
// Function using function pointer
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

// Function with various parameter types
void complex_function(int scalar, const char* string, struct SimpleStruct* struct_ptr,
                     union DataUnion* union_ptr, int array[], callback_t cb) {
    // Use all parameters to prevent optimization
    (void)scalar;
    (void)string;
    (void)struct_ptr;
    (void)union_ptr;
    (void)array;
    (void)cb;
}

#ifdef __cplusplus
// C++ specific function with templates
template<typename T, typename U>
auto template_function(T t, U u) -> decltype(t + u) {
    return t + u;
}

// Function with noexcept specification
void noexcept_function() noexcept {
    // Do nothing
}

// Constexpr function
constexpr int constexpr_func(int x) {
    return x * 2;
}
#endif

/* ==================== Main Function ==================== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    // Local scalar variables
    int local_int = 10;
    unsigned local_uint = 20;
    float local_float = 3.14f;
    double local_double = 2.71828;
    
    // Local structure instances
    struct SimpleStruct local_struct = {local_int, local_double, 'X'};
    struct OuterStruct local_outer = {99, {1.5f, 'Y'}, {2, 3.0, 'Z'}};
    
    // Local union
    union DataUnion local_union;
    local_union.float_value = 3.14159f;
    
    // Array usage
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * mode;
    }
    
    // Pointer operations
    int* dynamic_int = &local_int;
    *dynamic_int = 100;
    
    // Function pointer usage
    comparator_t comp_func = compare_ints;
    int a = 5, b = 10;
    comp_func(&a, &b);
    
    // Callback registration
    struct CallbackHolder holder = {"test", my_callback, nullptr};
    register_callback(holder.callback, holder.user_data);
    
#ifdef __cplusplus
    // C++ specific code
    BaseClass* base_ptr = new DerivedClass();
    base_ptr->virtual_func();
    delete base_ptr;
    
    // Use template instances
    int template_result = template_int_instance.get_data();
    (void)template_result;
    
    // Lambda usage
    cpp_callback = [](int x) { return x * 3; };
    int lambda_result = cpp_callback(10);
    (void)lambda_result;
    
    // Template callback
    template_callback([](int x) { (void)x; });
    
    // Use structured binding
    use_structured_binding();
    
    // Fold expression
    int fold_result = sum_all(1, 2, 3, 4, 5);
    (void)fold_result;
    
    // Initializer list
    int init_result = init_list_func({1, 2, 3, 4, 5});
    (void)init_result;
    
    // Variadic template instantiation
    VariadicStruct<int, double, char> variadic_instance;
    (void)variadic_instance;
    
    // Template template parameter
    TemplateTemplateClass<AliasTemplate, int> ttc_instance;
    (void)ttc_instance;
    
    // Constexpr usage
    constexpr int ce_result = constexpr_func(21);
    (void)ce_result;
#endif
    
#ifndef __cplusplus
    // C-specific code
    if (argc > 2) {
        use_vla(argc);
    }
#endif
    
    // Complex function call
    complex_function(local_int, "test", &local_struct, &local_union, 
                    int_array, my_callback);
    
    // Generate observable output based on all types
    unsigned long long hash = 0;
    
    // Hash sizes of various types
    hash += sizeof(incomplete_ptr_t);
    hash ^= sizeof(int) * 31;
    hash ^= sizeof(struct SimpleStruct) * 37;
    hash ^= sizeof(union DataUnion) * 41;
    hash ^= sizeof(int*) * 43;
    hash ^= sizeof(int[10]) * 47;
    
#ifdef __cplusplus
    hash ^= sizeof(BaseClass) * 53;
    hash ^= sizeof(DerivedClass) * 59;
    hash ^= sizeof(TemplateClass<int>) * 61;
#endif
    
    // Use mode to select different code paths
    switch (mode) {
        case 'A':
            // Use scalar types
            global_int += local_int;
            global_float *= local_float;
            break;
        case 'B':
            // Use structure types
            local_struct.x = global_simple_struct.y;
            local_outer.inner.data = global_outer_struct.simple.y;
            break;
        case 'C':
            // Use pointer types
            *int_ptr = global_uint;
            struct_ptr->y = global_double;
            break;
        case 'D':
            // Use array types
            for (int i = 0; i < 10; i++) {
                int_array[i] = i * global_int;
            }
            break;
#ifdef __cplusplus
        case 'E':
            // Use C++ specific types
            {
                DerivedClass derived;
                derived.virtual_func();
            }
            break;
        case 'F':
            // Use template types
            {
                auto result = template_function(10, 3.14);
                (void)result;
            }
            break;
#endif
    }
    
    // Final observable output
    printf("Type analysis hash: %llu\n", hash);
    
    return (int)(hash % 256);
}
