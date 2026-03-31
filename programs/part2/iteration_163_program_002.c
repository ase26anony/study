/* gengtype_test.c - Test program to trigger all gengtype type categories */

#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <utility>
#include <coroutine>
#include <functional>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  /* Forward declaration - never defined */
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
long double global_ldouble = 1.618033988749895L;
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char* const global_const_string = "Constant pointer to constant string";

/* ========== TYPE_STRUCT ========== */
/* Simple structure */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct OuterStruct {
    int id;
    struct SimpleStruct inner;
    struct {
        int anonymous_x;
        double anonymous_y;
    } anonymous;
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
    double c;
} __attribute__((packed));

/* Structure with flexible array member (C only) */
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    double data[];
};
#endif

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Base class */
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

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
public:
    DerivedClass() : extra_data(0) {}
    void virtual_func() override { protected_data = 1.0f; }
private:
    int extra_data;
};

/* Multiple inheritance */
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

/* Template class */
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(const T& d) : data(d) {}
    T get_data() const { return data; }
    void set_data(const T& d) { data = d; }
};

/* Class with complex members */
class ComplexClass {
public:
    ComplexClass() : ptr(new int(0)), ref(*ptr) {}
    ~ComplexClass() { delete ptr; }
    
    ComplexClass(const ComplexClass& other) : ptr(new int(*other.ptr)), ref(*ptr) {}
    
    void method_with_exception() noexcept(false) {
        if (*ptr < 0) throw "Negative value";
    }
    
private:
    int* ptr;
    int& ref;
    static int static_member;
};

int ComplexClass::static_member = 100;
#endif

/* ========== TYPE_UNION ========== */
/* C-style union */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char* string_ptr;
    struct SimpleStruct struct_val;
};

/* Union within structure */
struct UnionContainer {
    int type;
    union {
        int as_int;
        double as_double;
        char as_string[16];
    } data;
};

#ifdef __cplusplus
/* C++11 anonymous union */
struct AnonymousUnionStruct {
    int tag;
    union {
        int int_member;
        float float_member;
        struct {
            short x, y;
        } point;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_ptr = &int_ptr;
const char* const* const_string_ptr_ptr = &global_string;

/* Function pointers */
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidFunc)(void);
typedef const char* (*StringFunc)(int);

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to structure */
struct SimpleStruct* struct_ptr = 0;

#ifdef __cplusplus
/* Pointer to member function */
typedef void (BaseClass::*MemberFuncPtr)();
/* Pointer to data member */
typedef int BaseClass::*DataMemberPtr;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double double_array[5][10];
char char_3d_array[2][3][4];
struct SimpleStruct struct_array[5];

/* Array of pointers */
int* pointer_array[20];

/* Incomplete array in structure (C only) */
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int values[];
};
#endif

/* Variable Length Array (C99) */
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

/* Structure with function pointer */
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

/* Functions for callbacks */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
/* Template with callback */
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

/* Lambda expressions */
auto lambda = [](int x) -> int { return x * x; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Lambda with different captures */
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto mutable_lambda = [count = 0]() mutable -> int {
    return count++;
};

/* Initializer list usage */
std::initializer_list<int> init_list = {1, 2, 3, 4, 5};

/* Structured bindings (C++17) */
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

/* Fold expressions (C++17) */
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

/* Coroutine handle (C++20) */
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

/* Complex template instantiation */
template<typename T, typename U, typename V>
class TripleTemplate {
    T first;
    U second;
    V third;
public:
    TripleTemplate(T f, U s, V t) : first(f), second(s), third(t) {}
    
    auto get_all() {
        return std::make_tuple(first, second, third);
    }
};
#endif

/* ========== Helper functions ========== */
/* Function using incomplete pointer */
void use_incomplete_pointer(incomplete_ptr_t ptr) {
    /* Can't dereference, but can pass around */
    (void)ptr;
}

/* Function with many parameters to force analysis */
void complex_function(
    int scalar_param,
    const char* string_param,
    struct SimpleStruct struct_param,
    union DataUnion union_param,
    int* pointer_param,
    int array_param[],
    Callback callback_param
) {
    /* Use all parameters to prevent optimization */
    scalar_param++;
    (void)string_param;
    struct_param.x++;
    union_param.int_val++;
    (*pointer_param)++;
    array_param[0]++;
    if (callback_param) {
        callback_param(42, &scalar_param);
    }
}

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* Force TYPE_UNDEFINED */
    incomplete_ptr_t incomplete_ptr = 0;
    use_incomplete_pointer(incomplete_ptr);
    
    /* Force TYPE_SCALAR */
    int local_int = 100;
    unsigned local_uint = 200;
    float local_float = 3.14f;
    double local_double = 2.71;
    _Bool local_bool = 0;
    
    /* Force TYPE_STRING */
    const char* local_string = "Local string";
    char local_buffer[64] = "Buffer contents";
    
    /* Force TYPE_STRUCT */
    struct SimpleStruct simple = {1, 2.0, "Test"};
    struct OuterStruct outer = {100, {10, 20.0, "Inner"}, {30, 40.0}};
    struct BitFieldStruct bits = {1, 7, 15, -50};
    struct PackedStruct packed = {'X', 123, 456.789};
    
    /* Force TYPE_UNION */
    union DataUnion data_union;
    data_union.int_val = 42;
    struct UnionContainer union_container = {1, {.as_int = 100}};
    
#ifdef __cplusplus
    /* Force TYPE_USER_STRUCT */
    DerivedClass derived;
    derived.public_method();
    
    MultiInherit multi;
    multi.method1();
    multi.method2();
    
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    
    ComplexClass complex;
    
    AnonymousUnionStruct anon_union;
    anon_union.int_member = 100;
#endif
    
    /* Force TYPE_POINTER */
    int* local_ptr = &local_int;
    int** local_double_ptr = &local_ptr;
    BinaryFunc func_ptr = 0;
    
    /* Force TYPE_ARRAY */
    int local_array[5] = {1, 2, 3, 4, 5};
    double matrix[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
    
#ifndef __cplusplus
    /* VLA in C mode */
    if (argc > 1) {
        use_vla(argc);
    }
#endif
    
    /* Force TYPE_CALLBACK */
    CallbackContainer container = {sample_callback, &local_int};
    if (container.callback) {
        container.callback(10, container.user_data);
    }
    
#ifdef __cplusplus
    /* Force TYPE_CALLBACK with lambda */
    template_callback([](int x) { return x * 2; }, 5);
    
    /* Force TYPE_LANG_STRUCT */
    /* Lambda usage */
    int lambda_result = lambda(5);
    int capture_result = lambda_with_capture(10);
    
    /* Initializer list */
    for (auto val : init_list) {
        local_int += val;
    }
    
    /* Structured bindings */
    auto [x, y] = get_point();
    local_int += x + y;
    
    /* Fold expression */
    int fold_result = sum(1, 2, 3, 4, 5);
    
    /* Template instantiation */
    TripleTemplate<int, double, char> triple(1, 2.0, '3');
    
#if __cplusplus >= 202002L
    /* Coroutine */
    SimpleCoroutine coro;
#endif
    
    /* Output to prevent optimization */
    std::cout << "Mode: " << mode 
              << " Lambda: " << lambda_result
              << " Fold: " << fold_result
              << " Local: " << local_int << std::endl;
#else
    /* C output */
    printf("Mode: %c Int: %d Float: %f\n", mode, local_int, local_float);
#endif
    
    /* Create observable side effect based on all types */
    unsigned long hash = 0;
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)global_string[0];
    hash ^= (unsigned long)simple.x;
    hash ^= (unsigned long)data_union.int_val;
    hash ^= (unsigned long)local_ptr;
    hash ^= (unsigned long)local_array[0];
    
#ifdef __cplusplus
    hash ^= (unsigned long)derived.public_method;
    hash ^= (unsigned long)&lambda;
#endif
    
    /* Use mode to select different execution paths */
    switch (mode) {
        case 'A':
            complex_function(local_int, local_string, simple, data_union,
                           local_ptr, local_array, sample_callback);
            break;
        case 'B':
            /* Different type usage pattern */
            data_union.float_val = 3.14f;
            outer.anonymous.anonymous_x = 1000;
            break;
        case 'C':
#ifdef __cplusplus
            int_template.set_data(100);
            double_template.set_data(6.28);
#endif
            break;
        default:
            /* Use all arrays */
            for (int i = 0; i < 10; i++) {
                int_array[i] = i * mode;
            }
            break;
    }
    
    /* Final observable output */
#ifdef __cplusplus
    std::cout << "Final hash: " << hash << std::endl;
#else
    printf("Final hash: %lu\n", hash);
#endif
    
    return (int)(hash % 256);
}
