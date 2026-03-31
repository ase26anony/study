/* gengtype_test.cc - Comprehensive type coverage test for gengtype */

#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <cstddef>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  // Forward declaration, never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
/* Global scalar variables */
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
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
/* Basic C structure */
struct SimpleStruct {
    int x;
    double y;
    char z;
};

/* Nested structure */
struct OuterStruct {
    int id;
    struct InnerStruct {
        float data;
        char tag;
    } inner;
    struct SimpleStruct simple;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 4;  /* Padding */
};

/* Packed structure */
struct PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((packed));

/* Structure with flexible array member (C only) */
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    double data[];
};
#endif

/* Anonymous structure (C11/C++) */
struct AnonymousStruct {
    struct {
        int x;
        int y;
    } point;
    int id;
};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus

/* Simple class with different access specifiers */
class BaseClass {
public:
    BaseClass() : public_data(0) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_func() { std::cout << "BaseClass::virtual_func\n"; }
    void public_func() {}
    
    int public_data;
    
protected:
    void protected_func() {}
    int protected_data;
    
private:
    void private_func() {}
    int private_data;
};

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(0) {}
    void virtual_func() override { std::cout << "DerivedClass::virtual_func\n"; }
    
    int derived_data;
};

/* Multiple inheritance */
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

class MultipleInheritanceClass : public BaseClass, public Interface1, public Interface2 {
public:
    void interface1_func() override {}
    void interface2_func() override {}
    void virtual_func() override {}
};

/* Template class */
template<typename T, typename U>
class TemplateClass {
public:
    TemplateClass(T t, U u) : t_(t), u_(u) {}
    
    T get_t() const { return t_; }
    U get_u() const { return u_; }
    
private:
    T t_;
    U u_;
};

/* Class with static members */
class StaticMemberClass {
public:
    static int static_int;
    static double static_double;
    
    static void static_func() {}
};

int StaticMemberClass::static_int = 100;
double StaticMemberClass::static_double = 3.14;

#endif  /* __cplusplus */

/* ========== TYPE_UNION ========== */
/* C-style union */
union DataUnion {
    int i;
    float f;
    double d;
    char str[16];
};

/* Union within structure */
struct UnionContainer {
    int type;
    union {
        int int_value;
        float float_value;
        double double_value;
        void* ptr_value;
    } data;
};

#ifdef __cplusplus
/* C++11 anonymous union within class */
class AnonymousUnionClass {
public:
    int tag;
    union {
        int as_int;
        float as_float;
        double as_double;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
const volatile int* const_volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

/* Pointer to incomplete type */
incomplete_ptr_t incomplete_ptr = nullptr;

/* Pointers to structures */
struct SimpleStruct* simple_struct_ptr = nullptr;
struct OuterStruct* outer_struct_ptr = nullptr;

/* Pointer to array */
int (*array_ptr)[10] = nullptr;

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
char char_array[] = "Array Initializer";
double multi_dim_array[2][3][4] = {0};

/* Array of pointers */
int* pointer_array[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

/* Array of structures */
struct SimpleStruct struct_array[3] = {
    {1, 2.0, 'A'},
    {2, 3.0, 'B'},
    {3, 4.0, 'C'}
};

/* Incomplete array in structure (C only) */
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int data[];
};
#endif

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*IntCallback)(int, int);
typedef void (*VoidCallback)(void);
typedef double (*MathCallback)(double, double);

/* Structure with function pointer */
struct CallbackContainer {
    IntCallback int_callback;
    VoidCallback void_callback;
    MathCallback math_callback;
};

/* Function using callback */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void do_nothing(void) {}

#ifdef __cplusplus
/* C++ function objects and lambdas */
template<typename Func>
void call_with_10(Func f) {
    f(10);
}

/* Function pointer to member function */
typedef void (BaseClass::*MemberFuncPtr)();

/* std::function callback */
std::function<int(int, int)> std_callback = [](int a, int b) { return a + b; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus

/* Lambda expressions with different captures */
auto lambda_no_capture = []() { return 42; };
auto lambda_by_value = [global_int]() { return global_int; };
auto lambda_by_ref = [&global_int]() { return global_int; };
auto lambda_mutable = [global_int]() mutable { return global_int++; };

/* std::initializer_list usage */
auto init_list_func(std::initializer_list<int> list) {
    return list.size();
}

/* Structured bindings (C++17) */
struct Point { int x; int y; };
auto structured_binding_test() {
    Point p{10, 20};
    auto [x, y] = p;
    return x + y;
}

/* Fold expressions (C++17) */
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

/* Coroutine-related types (C++20) */
#if __cplusplus >= 202002L
struct DummyCoroutine {
    struct promise_type {
        DummyCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

DummyCoroutine dummy_coroutine() {
    co_return;
}
#endif

/* Template with complex type deduction */
template<typename T, typename... Args>
auto make_complex(Args&&... args) {
    return T(std::forward<Args>(args)...);
}

#endif  /* __cplusplus */

/* ========== MAIN FUNCTION ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    incomplete_ptr_t local_incomplete_ptr = nullptr;
    
    /* TYPE_SCALAR local variables */
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
#ifdef __cplusplus
    bool local_cppbool = false;
#endif
    
    /* TYPE_STRING usage */
    const char* local_string = "Local String";
    char local_char_buffer[50] = "Buffer Contents";
    
    /* TYPE_STRUCT instances */
    struct SimpleStruct simple = {1, 2.0, 'X'};
    struct OuterStruct outer = {100, {3.14f, 'Y'}, {2, 4.0, 'Z'}};
    struct BitFieldStruct bitfield = {1, 3, 5, -10};
    struct PackedStruct packed = {'A', 123, 456};
    struct AnonymousStruct anon = {{10, 20}, 30};
    
    /* TYPE_USER_STRUCT instances (C++ only) */
#ifdef __cplusplus
    BaseClass base;
    DerivedClass derived;
    MultipleInheritanceClass multiple;
    
    /* Template instantiations */
    TemplateClass<int, double> template_int_double(42, 3.14);
    TemplateClass<char*, float> template_charptr_float("test", 2.71f);
    
    /* Use static members */
    StaticMemberClass::static_int = 200;
#endif
    
    /* TYPE_UNION instances */
    union DataUnion data_union;
    data_union.i = 42;
    
    struct UnionContainer union_container;
    union_container.type = 1;
    union_container.data.int_value = 100;
    
#ifdef __cplusplus
    AnonymousUnionClass anon_union;
    anon_union.tag = 2;
    anon_union.as_float = 3.14f;
#endif
    
    /* TYPE_POINTER operations */
    int* local_int_ptr = &local_int;
    const int* local_const_int_ptr = &local_int;
    int** local_double_ptr = &local_int_ptr;
    
    /* Take address of structures */
    struct SimpleStruct* local_struct_ptr = &simple;
    struct OuterStruct* local_outer_ptr = &outer;
    
    /* TYPE_ARRAY usage */
    int local_int_array[5] = {10, 20, 30, 40, 50};
    float local_float_array[3] = {1.1f, 2.2f, 3.3f};
    
    /* Multi-dimensional array access */
    multi_dim_array[0][0][0] = 1.0;
    multi_dim_array[1][2][3] = 2.0;
    
    /* TYPE_CALLBACK usage */
    IntCallback local_callback = (mode == 'A') ? add : multiply;
    int callback_result = local_callback(10, 20);
    
    struct CallbackContainer callbacks = {add, do_nothing, nullptr};
    int container_result = callbacks.int_callback(5, 6);
    
#ifdef __cplusplus
    /* Lambda usage */
    auto local_lambda = [](int x) { return x * 2; };
    int lambda_result = local_lambda(21);
    
    /* std::function usage */
    std::function<int(int)> std_func = [](int x) { return x * 3; };
    int std_func_result = std_func(7);
    
    /* Member function pointer */
    MemberFuncPtr member_ptr = &BaseClass::virtual_func;
    (base.*member_ptr)();
    
    /* Template callback */
    call_with_10([](int x) { /* do something */ });
#endif
    
    /* TYPE_LANG_STRUCT usage (C++ only) */
#ifdef __cplusplus
    /* Lambda with capture */
    int capture_var = 100;
    auto capturing_lambda = [capture_var](int x) { return x + capture_var; };
    capturing_lambda(50);
    
    /* std::initializer_list */
    auto list = {1, 2, 3, 4, 5};
    size_t list_size = init_list_func(list);
    
    /* Structured binding */
    int sb_result = structured_binding_test();
    
    /* Fold expression */
    int fold_result = sum(1, 2, 3, 4, 5);
    
    /* Complex template instantiation */
    auto complex_obj = make_complex<SimpleStruct>(1, 2.0, 'C');
#endif
    
    /* Generate observable output based on all types */
    unsigned long hash = 0;
    
    /* Hash scalar types */
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)global_uint << 8;
    hash ^= (unsigned long)global_char << 16;
    hash ^= (unsigned long)global_float;
    
    /* Hash structure sizes */
    hash ^= sizeof(struct SimpleStruct);
    hash ^= sizeof(struct OuterStruct) << 8;
    hash ^= sizeof(struct BitFieldStruct) << 16;
    hash ^= sizeof(struct PackedStruct) << 24;
    
    /* Hash pointer addresses */
    hash ^= (unsigned long)(void*)int_ptr;
    hash ^= (unsigned long)(void*)&simple << 8;
    hash ^= (unsigned long)(void*)local_callback << 16;
    
    /* Hash array contents */
    for (int i = 0; i < 5; i++) {
        hash ^= (unsigned long)local_int_array[i] << (i * 4);
    }
    
    /* Final output to prevent optimization */
#ifdef __cplusplus
    std::cout << "Type coverage test hash: " << hash 
              << ", Callback result: " << callback_result
              << ", Mode: " << mode << std::endl;
#else
    printf("Type coverage test hash: %lu, Callback result: %d, Mode: %c\n", 
           hash, callback_result, mode);
#endif
    
    /* Return value based on mode to ensure all code paths are considered */
    switch (mode) {
        case 'A': return (int)(hash & 0xFF);
        case 'B': return callback_result;
        case 'C': return sizeof(struct SimpleStruct);
        case 'D': return (int)global_string[0];
        default:  return 0;
    }
}
