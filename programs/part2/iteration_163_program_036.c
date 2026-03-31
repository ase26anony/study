/* gengtype_test.c - Test program to trigger all gengtype type categories */
#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <utility>
#include <coroutine>
#include <functional>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_type;  /* Forward declaration, never defined */
typedef struct incomplete_type* incomplete_ptr_t;

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
#ifdef __cplusplus
bool global_bool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char* const global_const_string_ptr = "Constant pointer to constant string";

/* ========== TYPE_STRUCT ========== */
/* Simple C structure */
struct SimpleStruct {
    int id;
    char name[32];
    float value;
};

/* Nested structure */
struct OuterStruct {
    struct SimpleStruct inner;
    int outer_id;
    struct {
        int anonymous_member;
    } anonymous;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
    unsigned int : 2;  /* Padding */
};

/* Packed structure */
struct PackedStruct {
    char a;
    int b;
    char c;
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
/* Simple class */
class SimpleClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    SimpleClass() : private_data(0), protected_data(0.0f) {}
    virtual ~SimpleClass() {}
    virtual void virtual_method() {}
    void public_method() {}
};

/* Derived class with single inheritance */
class DerivedClass : public SimpleClass {
public:
    DerivedClass() : additional_data(0) {}
    void virtual_method() override {}
private:
    int additional_data;
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

class MultipleInheritance : public Base1, public Base2 {
public:
    void base1_method() override {}
    void base2_method() override {}
};

/* Template class */
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(const T& d) : data(d) {}
    T get_data() const { return data; }
};

/* Class with static members */
class StaticMemberClass {
public:
    static int static_counter;
    static constexpr double PI = 3.14159;
};
int StaticMemberClass::static_counter = 0;

#endif /* __cplusplus */

/* ========== TYPE_UNION ========== */
/* C-style union */
union DataUnion {
    int int_value;
    float float_value;
    char char_value;
    void* ptr_value;
};

/* Union within structure */
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct SimpleStruct struct_data;
    } data;
};

#ifdef __cplusplus
/* C++11 anonymous union */
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        char* as_string;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

/* Pointer to function */
typedef int (*func_ptr_t)(int, int);
typedef void (*void_func_ptr_t)(void);

/* Pointer to array */
typedef int (*array_ptr_t)[10];

/* Pointer to structure */
struct SimpleStruct* struct_ptr = 0;

/* Pointer to incomplete type */
incomplete_ptr_t incomplete_ptr = 0;

#ifdef __cplusplus
/* Pointer to member function */
typedef void (SimpleClass::*member_func_ptr_t)();
/* Pointer to data member */
typedef int SimpleClass::*data_member_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int simple_array[10] = {0};
int multi_dim_array[2][3][4] = {{{0}}};
struct SimpleStruct struct_array[5];
char* pointer_array[20];

/* Array of function pointers */
func_ptr_t func_ptr_array[5];

/* Variable Length Array (C99 only) */
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

/* Structure with function pointer member */
struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

/* Functions to use as callbacks */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    /* Do nothing */
}

#ifdef __cplusplus
/* Template with callback */
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

/* Lambda as callback */
auto lambda_callback = [](int x) { return x * 2; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Lambda expressions with different captures */
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_with_ref_capture = [&global_double](double x) -> double {
    return x * global_double;
};

/* std::initializer_list usage */
void use_initializer_list(std::initializer_list<int> list) {
    for (auto x : list) {
        /* Process each element */
    }
}

/* Structured bindings (C++17) */
struct Point { int x; int y; };
void use_structured_binding() {
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
}

/* Fold expressions (C++17) */
template<typename... Args>
auto sum_all(Args... args) {
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

SimpleCoroutine sample_coroutine() {
    co_return;
}
#endif

/* Variadic template with complex type deduction */
template<typename... Ts>
struct VariadicTemplate {
    static constexpr size_t count = sizeof...(Ts);
};

/* Using decltype with complex expressions */
auto complex_decltype = [](auto x) -> decltype(x * 2.0) {
    return x * 2.0;
};

#endif /* __cplusplus */

/* ========== Helper functions ========== */
int add_numbers(int a, int b) {
    return a + b;
}

void process_array(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] *= 2;
    }
}

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_SCALAR - Local variables */
    int local_int = 100;
    unsigned local_uint = 200u;
    char local_char = 'B';
    float local_float = 1.234f;
    double local_double = 5.678;
#ifdef __cplusplus
    bool local_bool = false;
#endif
    
    /* TYPE_STRUCT - Instantiate structures */
    struct SimpleStruct simple = {1, "Test", 3.14f};
    struct OuterStruct outer = {{2, "Inner", 2.718f}, 3, {4}};
    struct BitFieldStruct bitfield = {1, 3, 5, -100};
    struct PackedStruct packed = {'X', 123, 'Y'};
    
    /* TYPE_UNION - Use unions */
    union DataUnion data_union;
    data_union.int_value = 42;
    
    struct UnionContainer union_container;
    union_container.type = 1;
    union_container.data.int_data = 100;
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT - Instantiate C++ classes */
    SimpleClass simple_obj;
    DerivedClass derived_obj;
    MultipleInheritance multi_obj;
    TemplateClass<int> template_int_obj(42);
    TemplateClass<double> template_double_obj(3.14);
    
    /* TYPE_LANG_STRUCT - Use C++ language features */
    use_initializer_list({1, 2, 3, 4, 5});
    
    if (mode == 'L') {
        use_structured_binding();
    }
    
    /* Lambda usage */
    auto result = lambda_with_capture(10);
    template_callback(lambda_callback, 5);
    
    #if __cplusplus >= 202002L
    if (mode == 'C') {
        sample_coroutine();
    }
    #endif
    
    /* Fold expression */
    if (mode == 'F') {
        auto total = sum_all(1, 2, 3, 4, 5);
    }
    
    /* Variadic template instantiation */
    VariadicTemplate<int, double, char, SimpleClass> variadic_instance;
#endif
    
    /* TYPE_POINTER - Use various pointers */
    int* local_ptr = &local_int;
    struct_ptr = &simple;
    
    /* Function pointer usage */
    func_ptr_t add_func = add_numbers;
    if (mode == 'P') {
        int sum = add_func(10, 20);
    }
    
    /* TYPE_ARRAY - Use arrays */
    process_array(simple_array, 10);
    
#ifndef __cplusplus
    /* VLA in C mode */
    if (mode == 'V') {
        use_vla(20);
    }
#endif
    
    /* TYPE_CALLBACK - Use callback system */
    struct CallbackContainer cb_container = {sample_callback, NULL};
    if (mode == 'C') {
        cb_container.callback(42, NULL);
    }
    
    /* Array of function pointers */
    func_ptr_array[0] = add_numbers;
    
    /* TYPE_STRING - Use strings */
    const char* local_string = "Local string";
    wchar_t local_wstring[] = L"Local wide string";
    
    /* TYPE_UNDEFINED - Use incomplete type pointer */
    incomplete_ptr_t local_incomplete_ptr = incomplete_ptr;
    
    /* Generate observable output based on all types */
    unsigned long hash = 0;
    
    /* Hash sizes of different types */
    hash ^= (unsigned long)sizeof(struct SimpleStruct);
    hash ^= (unsigned long)sizeof(union DataUnion);
    hash ^= (unsigned long)sizeof(simple_array);
#ifdef __cplusplus
    hash ^= (unsigned long)sizeof(SimpleClass);
    hash ^= (unsigned long)sizeof(TemplateClass<int>);
#endif
    
    /* Hash addresses (shifted to avoid exposing actual addresses) */
    hash ^= ((unsigned long)&global_int >> 4);
    hash ^= ((unsigned long)&simple >> 4);
    hash ^= ((unsigned long)add_numbers >> 4);
    
    /* Hash based on mode */
    hash ^= (unsigned long)mode;
    
    /* Print hash to ensure execution */
#ifdef __cplusplus
    std::cout << "Type coverage hash: " << hash << std::endl;
#else
    printf("Type coverage hash: %lu\n", hash);
#endif
    
    /* Return based on mode to ensure different code paths */
    switch (mode) {
        case 'A': return (int)(hash & 0xFF);
        case 'B': return local_int;
        case 'C': return (int)global_char;
        case 'D': return (int)local_float;
        case 'E': return (int)(hash >> 32);
        default:  return 0;
    }
}
