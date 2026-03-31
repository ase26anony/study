/* gengtype_test.cc - Comprehensive type coverage test for gengtype */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <cstddef>
#else
#include <stdio.h>
#include <stddef.h>
#endif

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration never defined */
struct undefined_struct;
typedef struct undefined_struct* undefined_ptr_t;

/* ========== TYPE_SCALAR ========== */
/* Global scalar variables */
int global_int = 42;
unsigned int global_uint = 4294967295u;
char global_char = 'A';
signed char global_schar = -128;
unsigned char global_uchar = 255;
short global_short = -32768;
unsigned short global_ushort = 65535;
long global_long = -2147483647L;
unsigned long global_ulong = 4294967295UL;
long long global_llong = -9223372036854775807LL;
unsigned long long global_ullong = 18446744073709551615ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.6180339887498948482L;
_Bool global_bool = 1;

#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
/* String literals and character arrays */
const char* global_cstring = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
/* Basic C structure */
struct basic_struct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct outer_struct {
    int id;
    struct basic_struct inner;
    struct {
        float extra_data;
        char tag;
    } anonymous;
};

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 4; /* Padding */
};

/* Packed structure */
struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* Structure with flexible array member (C only) */
#ifndef __cplusplus
struct flex_array_struct {
    size_t count;
    int data[];
};
#endif

/* Global structure instances */
struct basic_struct global_basic = {10, 3.14, "Test"};
struct outer_struct global_outer = {1, {5, 2.71, "Inner"}, {1.618f, 'X'}};
struct bitfield_struct global_bitfield = {1, 7, 15, -128};

/* ========== TYPE_USER_STRUCT ========== */
#ifdef __cplusplus
/* Simple C++ class */
class SimpleClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    SimpleClass() : private_data(0), protected_data(0.0f) {}
    virtual ~SimpleClass() {}
    virtual void virtual_method() {}
    void public_method(int x) { private_data = x; }
    static int static_method() { return 42; }
};

/* Derived class with inheritance */
class DerivedClass : public SimpleClass {
private:
    double extra_data;
public:
    DerivedClass() : SimpleClass(), extra_data(0.0) {}
    void virtual_method() override {}
    void derived_method() {}
};

/* Multiple inheritance */
class Base1 {
public:
    virtual void base1_method() = 0;
    virtual ~Base1() {}
};

class Base2 {
public:
    virtual void base2_method() = 0;
    virtual ~Base2() {}
};

class MultipleDerived : public Base1, public Base2 {
public:
    void base1_method() override {}
    void base2_method() override {}
};

/* Template class */
template<typename T, typename U>
class TemplateClass {
private:
    T data1;
    U data2;
public:
    TemplateClass(T t, U u) : data1(t), data2(u) {}
    T get_first() const { return data1; }
    U get_second() const { return data2; }
    
    template<typename V>
    V process(V v) { return v + static_cast<V>(data1); }
};

/* Instantiate template classes */
TemplateClass<int, double> global_template_int_double(42, 3.14);
TemplateClass<float, char*> global_template_float_string(2.71f, "Template");

/* Class with complex members */
class ComplexClass {
public:
    class NestedClass {
    public:
        int nested_data;
        NestedClass() : nested_data(0) {}
    };
    
    enum class Color { RED, GREEN, BLUE };
    
    union InternalUnion {
        int as_int;
        float as_float;
        void* as_ptr;
    };
    
private:
    NestedClass nested;
    Color color;
    InternalUnion data;
    
public:
    ComplexClass() : nested(), color(Color::RED), data{0} {}
};

#endif /* __cplusplus */

/* ========== TYPE_UNION ========== */
/* C-style union */
union data_union {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
    char as_string[16];
};

/* Union within structure */
struct union_container {
    int type;
    union {
        int int_value;
        float float_value;
        char string_value[32];
    } data;
};

/* Global union instances */
union data_union global_union = {.as_int = 42};
struct union_container global_union_container = {1, {.int_value = 100}};

#ifdef __cplusplus
/* C++11 anonymous union within class */
class ClassWithAnonymousUnion {
private:
    int tag;
    union {
        int int_member;
        double double_member;
        SimpleClass* class_ptr;
    };
public:
    ClassWithAnonymousUnion() : tag(0), int_member(0) {}
};
#endif

/* ========== TYPE_POINTER ========== */
/* Pointers to various types */
int* int_ptr = &global_int;
double* double_ptr = &global_double;
const char** string_ptr_ptr = &global_cstring;
void* void_ptr = (void*)0x1000;
volatile int* volatile volatile_int_ptr = &global_int;
const volatile long* const_volatile_long_ptr = &global_long;
int* restrict restricted_ptr = &global_int;

/* Pointer to array */
int (*array_ptr)[10];

/* Function pointers */
typedef int (*binary_func_t)(int, int);
typedef void (*void_func_t)(void);
typedef const char* (*string_func_t)(const char*);

/* Function pointer instances */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
binary_func_t func_ptr_array[2] = {add, multiply};

/* Pointer to structure */
struct basic_struct* struct_ptr = &global_basic;
struct outer_struct** struct_ptr_ptr = &struct_ptr;

#ifdef __cplusplus
/* Pointers to member functions */
typedef void (SimpleClass::*SimpleClassMethodPtr)();
typedef int (SimpleClass::*SimpleClassConstMethodPtr)(int) const;

/* Pointer to member data */
typedef int SimpleClass::*SimpleClassDataPtr;

/* Member pointers */
SimpleClassMethodPtr virtual_method_ptr = &SimpleClass::virtual_method;
SimpleClassDataPtr data_ptr = nullptr;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various arrays */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];
double double_3d_array[2][3][4];
char* string_ptr_array[] = {"One", "Two", "Three", NULL};

/* Array of structures */
struct basic_struct struct_array[5];

/* Incomplete array in structure (C only) */
#ifndef __cplusplus
struct with_incomplete_array {
    int count;
    char data[];
};
#endif

/* Variable length array function (C99) */
#ifndef __cplusplus
void process_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
/* Callback function types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*event_handler_t)(int event_id, void* user_data);

/* Structure with function pointer */
struct callback_container {
    event_handler_t handler;
    void* user_data;
    comparator_t compare;
};

/* Function using callback */
void register_callback(event_handler_t handler, void* data) {
    /* Callback registration */
}

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

#ifdef __cplusplus
/* C++ function objects and lambdas */
template<typename Func>
void execute_callback(Func f, int value) {
    f(value);
}

/* Lambda expressions */
auto lambda = [](int x) -> int { return x * x; };

/* std::function */
std::function<int(int, int)> std_func = [](int a, int b) { return a + b; };
#endif

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
/* Lambda with different captures */
auto lambda_with_capture = [global_int, &global_double](float x) -> double {
    return x + global_int + global_double;
};

/* Lambda with complex capture */
int capture_me = 100;
auto complex_lambda = [capture_me](auto x) -> decltype(x) {
    return x + capture_me;
};

/* std::initializer_list usage */
class InitializerListUser {
    std::initializer_list<int> init_list;
public:
    InitializerListUser(std::initializer_list<int> list) : init_list(list) {}
    
    int sum() const {
        int total = 0;
        for (int val : init_list) {
            total += val;
        }
        return total;
    }
};

/* Structured bindings (C++17) */
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

/* Fold expressions (C++17) */
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

/* Coroutine-related types (C++20) */
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
#endif

/* Complex template with language features */
template<typename T>
class LanguageFeatureContainer {
    T data;
    
public:
    LanguageFeatureContainer(std::initializer_list<T> init) {
        auto it = init.begin();
        if (it != init.end()) data = *it;
    }
    
    auto process() const {
        if constexpr (std::is_arithmetic_v<T>) {
            return data * 2;
        } else {
            return data;
        }
    }
    
    template<typename... Args>
    auto fold_test(Args... args) const {
        return (data + ... + args);
    }
};

/* Instantiate with different types */
LanguageFeatureContainer<int> lfc_int{1, 2, 3, 4};
LanguageFeatureContainer<double> lfc_double{1.1, 2.2, 3.3};

#endif /* __cplusplus */

/* ========== MAIN FUNCTION ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    undefined_ptr_t undefined_ptr = NULL;
    if (mode == 'U') {
        /* This would cause issues if dereferenced, but we just use the pointer */
        printf("Undefined pointer: %p\n", (void*)undefined_ptr);
    }
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned local_uint = 200u;
    float local_float = 3.14f;
    double local_double = 2.71828;
    
    if (mode == 'S') {
        printf("Scalars: %d %u %.2f %.5f\n", 
               local_int, local_uint, local_float, local_double);
    }
    
    /* TYPE_STRING usage */
    const char* local_string = "Local String";
    wchar_t local_wstring[] = L"Local Wide";
    
    if (mode == 'T') {
        printf("String: %s\n", local_string);
    }
    
    /* TYPE_STRUCT usage */
    struct basic_struct local_basic = {20, 6.28, "Local"};
    struct outer_struct local_outer = {2, {15, 1.414, "Nested"}, {3.14f, 'Y'}};
    
    if (mode == 'R') {
        printf("Struct: %s\n", local_basic.name);
    }
    
    /* TYPE_UNION usage */
    union data_union local_union;
    local_union.as_int = 12345;
    
    if (mode == 'N') {
        printf("Union as int: %d\n", local_union.as_int);
    }
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    void** local_void_ptr_ptr = (void**)&local_int_ptr;
    
    if (mode == 'P') {
        printf("Pointer value: %d\n", *local_int_ptr);
    }
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {5, 4, 3, 2, 1};
    
    if (mode == 'A') {
        for (int i = 0; i < 5; i++) {
            printf("%d ", local_array[i]);
        }
        printf("\n");
    }
    
    /* TYPE_CALLBACK usage */
    callback_container container = {NULL, NULL, compare_ints};
    
    if (mode == 'C') {
        int nums[] = {5, 2, 8, 1, 9};
        qsort(nums, 5, sizeof(int), container.compare);
        printf("Sorted: %d %d %d %d %d\n", 
               nums[0], nums[1], nums[2], nums[3], nums[4]);
    }
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT usage */
    SimpleClass simple_obj;
    DerivedClass derived_obj;
    MultipleDerived multiple_obj;
    
    TemplateClass<int, float> local_template(100, 2.5f);
    
    if (mode == 'U') {
        simple_obj.public_method(42);
        derived_obj.derived_method();
        printf("Template: %d %.2f\n", 
               local_template.get_first(), local_template.get_second());
    }
    
    /* TYPE_LANG_STRUCT usage */
    auto result = lambda(5);
    auto captured_result = lambda_with_capture(2.5f);
    
    InitializerListUser il_user{1, 2, 3, 4, 5};
    
    if (mode == 'L') {
        printf("Lambda: %d\n", result);
        printf("InitializerList sum: %d\n", il_user.sum());
        
        /* Structured binding */
        auto [x, y] = get_point();
        printf("Point: %d, %d\n", x, y);
        
        /* Fold expression */
        printf("Fold sum: %d\n", sum_all(1, 2, 3, 4, 5));
        
        /* Complex lambda */
        printf("Complex lambda: %d\n", complex_lambda(50));
    }
    
    /* Execute template callback */
    execute_callback([](int v) { printf("Callback: %d\n", v); }, 42);
    
    /* Use std::function */
    int func_result = std_func(10, 20);
    printf("std::function result: %d\n", func_result);
#endif
    
#ifndef __cplusplus
    /* C-specific features */
    if (mode == 'V') {
        process_vla(10);
    }
#endif
    
    /* Force usage of all global variables to prevent optimization */
    unsigned long hash = 0;
    
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)global_uint;
    hash ^= (unsigned long)*global_cstring;
    hash ^= (unsigned long)global_basic.x;
    hash ^= (unsigned long)global_union.as_int;
    hash ^= (unsigned long)int_ptr;
    
#ifdef __cplusplus
    hash ^= (unsigned long)global_template_int_double.get_first();
    hash ^= (unsigned long)&SimpleClass::virtual_method;
#endif
    
    /* Print hash to ensure all code has effect */
    printf("Final hash: %lu\n", hash);
    
    return (int)(hash % 256);
}
