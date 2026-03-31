/* gengtype_test.c - Comprehensive type coverage test for gengtype */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct IncompleteStruct;  // Never defined
class IncompleteClass;    // Never defined
#endif

/* ========== TYPE_UNDEFINED ========== */
struct IncompleteStruct;  // Forward declaration only
typedef struct IncompleteStruct* UndefPtr;

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
float global_float = 3.14159f;
double global_double = 2.718281828459045;
#ifdef __cplusplus
bool global_bool = true;
wchar_t global_wchar = L'Ω';
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
// Simple C structure
struct SimpleStruct {
    int x;
    double y;
    char z;
};

// Nested structure
struct OuterStruct {
    struct InnerStruct {
        int a;
        float b;
    } inner;
    struct SimpleStruct simple;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 12;
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

// Anonymous structure (C11/C++)
struct AnonymousStruct {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Base class with different access specifiers
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass() : private_member(0), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() {}
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : extra_data(0) {}
    ~DerivedClass() override {}
    void virtual_func() override {}
private:
    int extra_data;
};

// Multiple inheritance
class Interface1 {
public:
    virtual void method1() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void method2() = 0;
    virtual ~Interface2() {}
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
    TemplateClass(const T& d) : data(d) {}
    T get() const { return data; }
    void set(const T& d) { data = d; }
};

// Class with static members
class StaticMemberClass {
public:
    static int static_counter;
    static const double static_pi;
};
int StaticMemberClass::static_counter = 0;
const double StaticMemberClass::static_pi = 3.14159;

// Class with member functions of different types
class FunctionClass {
public:
    void normal_func() {}
    void const_func() const {}
    static void static_func() {}
    virtual void virtual_func() {}
    virtual void pure_virtual() = 0;
};
#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int i;
    float f;
    double d;
    char str[16];
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
};

// Anonymous union (C11/C++)
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    };
};

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_ptr = &int_ptr;
int* restrict restricted_ptr = &global_int;

// Pointers to different types
struct SimpleStruct* struct_ptr = 0;
union DataUnion* union_ptr = 0;
UndefPtr undefined_ptr = 0;  // Pointer to incomplete type

// Function pointers
typedef int (*FuncPtr)(int, int);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// Pointer to array
int (*array_ptr)[10];

// Pointer to pointer to function
int (*(*complex_func_ptr)(int))(int);

#ifdef __cplusplus
// Pointers to member functions
typedef void (BaseClass::*MemberFuncPtr)();
typedef int (TemplateClass<int>::*TemplateMemberPtr)() const;

// Pointer to data member
typedef int BaseClass::*DataMemberPtr;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];  // 2D array
double triple_array[2][3][4];  // 3D array

// Array of pointers
int* ptr_array[5];

// Array of structures
struct SimpleStruct struct_array[3];

// Array of function pointers
FuncPtr func_ptr_array[3];

// String arrays
const char* string_array[] = {"one", "two", "three", NULL};

// Incomplete array in struct (C only)
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int values[];
};
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer member
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Function using callback
void register_callback(Callback cb, void* data) {
    // Callback would be stored somewhere
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
using StdCallback = std::function<int(int)>;

// Template with callback
template<typename F>
void template_callback(F func) {
    func(42);
}

// Lambda expressions as callbacks
auto lambda_callback = [](int x) -> int { return x * 2; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda with different captures
auto lambda_with_capture = [global_int](int x) {
    return x + global_int;
};

auto mutable_lambda = [global_float]() mutable {
    return global_float * 2.0f;
};

// std::initializer_list usage
auto init_list_func(std::initializer_list<int> list) {
    return list.size();
}

// Structured bindings (C++17)
struct Point { int x; int y; };
auto structured_binding_test() {
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
    return x_coord + y_coord;
}

// Fold expressions (C++17)
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// Coroutine test (C++20)
#if __cplusplus >= 202002L
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleCoroutine test_coroutine() {
    co_return;
}
#endif

// Variadic templates
template<typename... Ts>
struct VariadicStruct {};

// Alias templates
template<typename T>
using AliasTemplate = TemplateClass<T>;

// decltype and auto usage
auto auto_func(int x) -> decltype(x * 2.5) {
    return x * 2.5;
}

// constexpr if (C++17)
template<typename T>
auto constexpr_if_test(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else {
        return value + 1.0;
    }
}
#endif

/* ========== Function Definitions ========== */
// Function using various types
int process_data(int argc, char** argv) {
    // Local scalar variables
    int local_int = argc;
    unsigned local_uint = (unsigned)argc;
    double local_double = 3.14 * argc;
    
    // Structure instances
    struct SimpleStruct local_struct = {1, 2.0, 'X'};
    struct OuterStruct local_outer = {{10, 20.5f}, {100, 200.0, 'Z'}};
    
    // Union instance
    union DataUnion local_union;
    local_union.i = 42;
    
    // Array usage
    int local_array[5];
    for (int i = 0; i < 5; i++) {
        local_array[i] = i * argc;
    }
    
    // Pointer operations
    int* local_ptr = &local_int;
    *local_ptr = 100;
    
    // Function pointer usage
    FuncPtr local_func_ptr = NULL;
    
    // Use command line arguments to prevent dead code elimination
    if (argc > 1) {
        // Use TYPE_UNDEFINED pointer
        UndefPtr test_ptr = NULL;
        (void)test_ptr;
        
        // Use TYPE_SCALAR
        local_int += global_short;
        local_double += global_float;
        
        // Use TYPE_STRING
        const char* arg_str = argv[1];
        (void)arg_str;
        
        // Use TYPE_STRUCT
        local_struct.x += local_outer.inner.a;
        
        // Use TYPE_UNION
        local_union.f = (float)argc;
        
        // Use TYPE_POINTER
        local_ptr = &global_int;
        
        // Use TYPE_ARRAY
        local_array[0] = int_array[argc % 10];
        
        // Use TYPE_CALLBACK
        local_func_ptr = NULL;
        register_callback(my_callback, NULL);
    }
    
    if (argc > 2) {
        // Alternate path with different type usage
        local_int -= global_long;
        local_union.d = (double)argc;
    }
    
#ifdef __cplusplus
    // C++ specific type usage
    if (argc > 3) {
        // TYPE_USER_STRUCT
        DerivedClass derived;
        BaseClass* base_ptr = &derived;
        base_ptr->virtual_func();
        
        // Template instantiation
        TemplateClass<int> template_int(42);
        TemplateClass<double> template_double(3.14);
        
        // TYPE_LANG_STRUCT features
        auto result = lambda_with_capture(argc);
        (void)result;
        
        auto sum_result = sum(1, 2, 3, argc);
        (void)sum_result;
        
        // Structured binding
        auto coord_sum = structured_binding_test();
        (void)coord_sum;
    }
#endif
    
    // Return a hash of various sizes and addresses
    int hash = 0;
    hash ^= sizeof(local_struct);
    hash ^= (int)(intptr_t)&local_struct;
    hash ^= sizeof(local_array);
    hash ^= argc * 31;
    
    return hash;
}

/* ========== Main Function ========== */
int main(int argc, char** argv) {
    // Ensure all global types are referenced
    (void)global_int;
    (void)global_string;
    (void)struct_ptr;
    (void)union_ptr;
    
    // Create instances of user-defined types
    struct SimpleStruct instance1 = {0};
    struct OuterStruct instance2 = {{0}};
    struct BitFieldStruct instance3 = {0};
    struct PackedStruct instance4 = {0};
    
    union DataUnion instance5;
    instance5.i = 0;
    
    // Use arrays
    int_array[0] = argc;
    
    // Use function pointers
    CallbackContainer container = {my_callback, NULL};
    (void)container;
    
#ifdef __cplusplus
    // C++ specific instantiations
    DerivedClass cpp_instance;
    TemplateClass<float> template_instance(3.14f);
    MultiInherit multi_instance;
    
    // Use TYPE_LANG_STRUCT constructs
    auto lambda_result = lambda_callback(argc);
    (void)lambda_result;
    
    // Use initializer_list
    auto list_size = init_list_func({1, 2, 3, 4, 5});
    (void)list_size;
    
    // Use template callback
    template_callback([](int x) { return x + 1; });
#endif
    
    // Process data with command-line dependent logic
    int result = process_data(argc, argv);
    
    // Output result to prevent optimization
#ifdef __cplusplus
    std::cout << "Result hash: " << result << std::endl;
#else
    printf("Result hash: %d\n", result);
#endif
    
    return result == 0 ? 0 : 1;
}
