#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <utility>
#include <coroutine>
#include <functional>
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
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.4142135623730950488L;
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
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Nested structure
struct OuterStruct {
    SimpleStruct inner;
    int outer_value;
};

// Structure with flexible array member (C only)
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    double data[];
};
#endif

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
    DerivedClass() : derived_member(100) {}
    void virtual_func() override {}
private:
    int derived_member;
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

class MultipleInheritance : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T, typename U>
class TemplateClass {
    T first;
    U second;
public:
    TemplateClass(T a, U b) : first(a), second(b) {}
    T get_first() const { return first; }
    U get_second() const { return second; }
};

// Class with member functions of various types
class ComplexClass {
    static int static_member;
    mutable int mutable_member;
    volatile int volatile_member;
public:
    ComplexClass() : mutable_member(0), volatile_member(0) {}
    int const_method() const { return mutable_member; }
    volatile int volatile_method() volatile { return volatile_member; }
};

int ComplexClass::static_member = 0;

// Instantiate template classes
TemplateClass<int, double> template_instance(10, 3.14);
TemplateClass<SimpleStruct, char*> template_instance2({1, 2.0, "test"}, nullptr);
#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char* string_value;
};

// Union within a structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct {
            short x, y;
        } point_data;
    } data;
};

// Anonymous union (C++11)
#ifdef __cplusplus
struct AnonymousUnionContainer {
    int tag;
    union {
        int as_int;
        double as_double;
        void* as_pointer;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
const volatile int* const_volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointers to different types
SimpleStruct* struct_ptr = nullptr;
DataUnion* union_ptr = nullptr;
char** string_ptr_ptr = nullptr;
void* void_ptr = nullptr;
const void* const_void_ptr = nullptr;

// Function pointers
typedef int (*FuncPtr)(int, double);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// Pointer to member function (C++)
#ifdef __cplusplus
typedef void (BaseClass::*MemberFuncPtr)();
#endif

// Pointer to array
typedef int (*ArrayPtr)[10];

/* ========== TYPE_ARRAY ========== */
// Various arrays
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];
double double_array[2][3][4];
char* string_array[] = {"one", "two", "three", nullptr};

// Array of structures
SimpleStruct struct_array[5];

// Array of pointers
int* pointer_array[20];

// Incomplete array in structure (C only)
#ifndef __cplusplus
struct IncompleteArray {
    int count;
    int values[];
};
#endif

// Variable length array (C99)
#ifndef __cplusplus
void use_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) vla[i] = i;
}
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Functions using callbacks
void register_callback(Callback cb, void* data) {
    if (cb) cb(42, data);
}

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

// C++ lambda as callback
#ifdef __cplusplus
template<typename Func>
void template_callback(Func f) {
    f(100);
}

auto lambda = [](int x) { return x * 2; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_mutable = [counter = 0]() mutable {
    return ++counter;
};

// std::initializer_list
void use_initializer_list(std::initializer_list<int> list) {
    for (auto x : list) {
        // Process x
    }
}

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// Coroutine (C++20)
#if __cplusplus >= 202002L
struct ReturnObject {
    struct promise_type {
        ReturnObject get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

ReturnObject example_coroutine() {
    co_await std::suspend_never{};
}
#endif

// Complex template with language features
template<typename T>
class LanguageFeatureContainer {
    T value;
public:
    LanguageFeatureContainer(std::initializer_list<T> init) {
        for (const auto& v : init) {
            // Process
        }
    }
    
    auto get_as_tuple() const {
        return std::make_tuple(value);
    }
};
#endif

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    incomplete_ptr_t undefined_ptr = nullptr;
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'B';
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
    /* TYPE_STRING usage */
    const char* local_string = "Local string";
    wchar_t local_wchar = L'X';
    char local_buffer[100] = "Buffer";
    
    /* TYPE_STRUCT usage */
    SimpleStruct simple = {1, 2.0, "test"};
    BitFieldStruct bitfield = {1, 2, 3, -5};
    PackedStruct packed = {'X', 42, 3.14};
    OuterStruct outer = {{10, 20.5, "inner"}, 30};
    
    /* TYPE_USER_STRUCT usage (C++ only) */
    #ifdef __cplusplus
    DerivedClass derived;
    MultipleInheritance multi;
    ComplexClass complex;
    LanguageFeatureContainer<int> lfc{1, 2, 3, 4, 5};
    #endif
    
    /* TYPE_UNION usage */
    DataUnion data_union;
    data_union.int_value = 42;
    
    UnionContainer union_container;
    union_container.type = 1;
    union_container.data.int_data = 100;
    
    #ifdef __cplusplus
    AnonymousUnionContainer anonymous_union;
    anonymous_union.tag = 2;
    anonymous_union.as_double = 3.14159;
    #endif
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    SimpleStruct* local_struct_ptr = &simple;
    
    // Function pointer usage
    FuncPtr func_ptr = nullptr;
    VoidFuncPtr void_func_ptr = nullptr;
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    SimpleStruct local_struct_array[2] = {{1, 1.0, "a"}, {2, 2.0, "b"}};
    
    #ifndef __cplusplus
    // Use VLA in C mode
    if (argc > 2) {
        int size = argv[2][0] - '0';
        use_vla(size);
    }
    #endif
    
    /* TYPE_CALLBACK usage */
    CallbackContainer container = {nullptr, nullptr};
    
    // Use function pointer
    Comparator comp = compare_ints;
    if (comp) {
        int a = 5, b = 10;
        comp(&a, &b);
    }
    
    #ifdef __cplusplus
    // Use lambda as callback
    template_callback(lambda);
    template_callback([mode](int x) { return x + mode; });
    #endif
    
    /* TYPE_LANG_STRUCT usage (C++ only) */
    #ifdef __cplusplus
    // Use initializer_list
    use_initializer_list({1, 2, 3, 4, 5});
    
    // Use structured bindings
    auto [x, y] = get_point();
    
    // Use fold expression
    int fold_result = sum(1, 2, 3, 4, 5);
    
    // Use lambda with capture
    int lambda_result = lambda_with_capture(10);
    int mutable_result = lambda_mutable();
    
    // Instantiate template with language features
    LanguageFeatureContainer<double> lfc_double{1.1, 2.2, 3.3};
    #endif
    
    // Force usage of all types based on argc
    unsigned long long hash = 0;
    
    switch (mode) {
        case 'A':
            hash += (unsigned long long)undefined_ptr;
            hash += global_int + local_int;
            break;
        case 'B':
            hash += (unsigned long long)global_string[0];
            hash += simple.x + outer.outer_value;
            break;
        case 'C':
            #ifdef __cplusplus
            hash += derived.virtual_func != nullptr;
            hash += template_instance.get_first();
            #endif
            hash += data_union.int_value;
            break;
        case 'D':
            hash += (unsigned long long)int_ptr;
            hash += int_array[0] + local_array[0];
            break;
        case 'E':
            if (comp) hash += 1;
            #ifdef __cplusplus
            hash += lambda_result + mutable_result;
            #endif
            break;
        default:
            hash = 0xDEADBEEF;
    }
    
    // Print something to ensure execution
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << std::endl;
    #else
    printf("Hash: %llu\n", hash);
    #endif
    
    return (int)(hash % 256);
}
