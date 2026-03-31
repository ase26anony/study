/* gengtype_test.c - Test program to trigger all gengtype type categories */

#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <functional>
#endif

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct undefined_struct;
typedef struct undefined_struct* undefined_ptr_t;

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

#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
/* String literals and character arrays */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
/* Simple C structure */
struct simple_struct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct outer_struct {
    int id;
    struct simple_struct inner;
    struct {
        int anonymous_x;
        double anonymous_y;
    } anonymous;
};

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 16; /* Padding */
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
    int count;
    double data[];
};
#endif

/* ========== TYPE_USER_STRUCT ========== */
#ifdef __cplusplus
/* C++ classes with various features */

/* Simple class with different access specifiers */
class BaseClass {
public:
    BaseClass() : public_var(0) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_func() {}
    void public_func() {}
    
protected:
    int protected_var;
    
private:
    int private_var;
    int public_var;
};

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_var(0) {}
    void virtual_func() override {}
    
private:
    int derived_var;
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

class MultiInheritClass : public BaseClass, public Interface1, public Interface2 {
public:
    void interface1_func() override {}
    void interface2_func() override {}
};

/* Template class */
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : value(val) {}
    
    T get_value() const { return value; }
    void set_value(T val) { value = val; }
    
private:
    T value;
};

/* Class with static members */
class StaticClass {
public:
    static int static_var;
    static void static_func() {}
};

int StaticClass::static_var = 0;

#endif /* __cplusplus */

/* ========== TYPE_UNION ========== */
/* C-style union */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
    struct simple_struct struct_val;
};

/* Union within structure */
struct union_container {
    int type;
    union {
        int int_data;
        float float_data;
        char char_data[16];
    } data;
};

#ifdef __cplusplus
/* C++11 anonymous union */
struct anonymous_union_struct {
    int tag;
    union {
        int as_int;
        double as_double;
        void* as_pointer;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;

const char* const* string_ptr_ptr = &global_string;
void* void_ptr = (void*)&global_int;

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to structure */
struct simple_struct* struct_ptr = 0;

/* Pointer to union */
union data_union* union_ptr = 0;

#ifdef __cplusplus
/* Pointer to member function */
typedef void (BaseClass::*member_func_ptr)();
/* Pointer to member data */
typedef int BaseClass::*member_data_ptr;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const char* string_array[] = {"one", "two", "three", "four"};
double multi_dim_array[2][3][4];

/* Array of structures */
struct simple_struct struct_array[5];

/* Array of pointers */
int* pointer_array[8];

/* Incomplete array in structure (C only) */
#ifndef __cplusplus
struct incomplete_array_struct {
    int count;
    int values[];
};
#endif

/* Variable length array (C99) */
#ifndef __cplusplus
void use_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*binary_func_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef const char* (*string_func_t)(void);

/* Structure with function pointer member */
struct callback_container {
    const char* name;
    callback_t callback;
    void* user_data;
};

/* Function using callback */
int process_with_callback(int a, int b, binary_func_t func) {
    return func(a, b);
}

/* Actual functions to point to */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

#ifdef __cplusplus
/* C++ function objects and lambdas */
template<typename Func>
void execute_callback(Func f) {
    f(42);
}

/* Function template expecting callback */
template<typename T, typename Callback>
void process_template(T value, Callback cb) {
    cb(value);
}

/* std::function */
#include <functional>
std::function<int(int, int)> std_func = add;
#endif

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
/* Complex C++ language constructs */

/* Lambda expressions with different captures */
auto lambda_no_capture = [](int x) { return x * 2; };
auto lambda_by_value = [global_int](int x) { return x + global_int; };
auto lambda_by_ref = [&global_int](int x) { global_int = x; return x; };
auto lambda_mutable = [counter = 0](int x) mutable { counter += x; return counter; };

/* std::initializer_list */
auto init_list_func(std::initializer_list<int> list) -> int {
    int sum = 0;
    for (auto val : list) {
        sum += val;
    }
    return sum;
}

/* Structured bindings (C++17) */
struct Point { double x, y; };
auto get_point() -> Point { return {1.0, 2.0}; }

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
#endif

/* Variadic templates */
template<typename... Ts>
struct TupleWrapper {
    std::tuple<Ts...> data;
};

/* Alias template */
template<typename T>
using AliasTemplate = TemplateClass<T>;

#endif /* __cplusplus */

/* ========== MAIN FUNCTION ========== */
int main(int argc, char* argv[]) {
    /* Force usage of argc/argv to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    undefined_ptr_t undefined_ptr = 0;
    if (mode == 'U') {
        /* This branch uses undefined type pointer */
        struct undefined_struct* local_undefined = 0;
        undefined_ptr = local_undefined;
    }
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 5.678;
    _Bool local_bool = 0;
    
#ifdef __cplusplus
    bool local_cppbool = false;
#endif
    
    /* TYPE_STRING usage */
    const char* local_string = "Local String";
    char local_char_array[] = "Local Array";
    wchar_t local_wchar = L'W';
    
    /* TYPE_STRUCT usage */
    struct simple_struct simple = {10, 3.14, "Test"};
    struct outer_struct outer = {1, {20, 2.718, "Inner"}, {30, 1.618}};
    struct bitfield_struct bitfield = {1, 5, 9, -50};
    struct packed_struct packed = {'X', 123, 456.789};
    
#ifndef __cplusplus
    /* Flexible array member usage (C only) */
    struct flex_array_struct* flex = malloc(sizeof(struct flex_array_struct) + 10 * sizeof(double));
    if (flex) {
        flex->count = 10;
        for (int i = 0; i < 10; i++) {
            flex->data[i] = i * 1.5;
        }
        free(flex);
    }
    
    /* Variable length array usage */
    if (argc > 2) {
        use_vla(argc);
    }
#endif
    
    /* TYPE_USER_STRUCT usage (C++) */
#ifdef __cplusplus
    BaseClass base;
    DerivedClass derived;
    MultiInheritClass multi;
    
    TemplateClass<int> template_int(42);
    TemplateClass<double> template_double(3.14);
    TemplateClass<const char*> template_string("Hello");
    
    StaticClass::static_var = 100;
    StaticClass::static_func();
#endif
    
    /* TYPE_UNION usage */
    union data_union data_u;
    data_u.int_val = 42;
    
    struct union_container container = {1, {.int_data = 100}};
    
#ifdef __cplusplus
    anonymous_union_struct anon_union = {0};
    anon_union.as_int = 200;
#endif
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    int** local_double_ptr = &local_int_ptr;
    
    struct simple_struct* local_struct_ptr = &simple;
    union data_union* local_union_ptr = &data_u;
    
    /* Function pointers */
    binary_func_t func_ptr = (mode == 'A') ? add : multiply;
    
#ifdef __cplusplus
    member_func_ptr mem_func_ptr = &BaseClass::virtual_func;
    member_data_ptr mem_data_ptr = &BaseClass::public_var;
#endif
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    double local_multi[2][3] = {{1.1, 1.2, 1.3}, {2.1, 2.2, 2.3}};
    
    struct simple_struct local_struct_array[3] = {
        {1, 1.1, "First"},
        {2, 2.2, "Second"},
        {3, 3.3, "Third"}
    };
    
    int* local_pointer_array[4] = {&local_array[0], &local_array[1], &local_array[2], &local_array[3]};
    
    /* TYPE_CALLBACK usage */
    int result = process_with_callback(10, 20, func_ptr);
    
    struct callback_container cb_container = {
        "test_callback",
        [](void* data, int res) { /* Do nothing */ },
        &result
    };
    
#ifdef __cplusplus
    /* Lambda usage */
    execute_callback([](int x) { /* Do nothing */ });
    
    process_template(42, [](int x) { return x * 2; });
    
    /* std::function usage */
    int std_result = std_func(5, 6);
#endif
    
    /* TYPE_LANG_STRUCT usage (C++) */
#ifdef __cplusplus
    /* Lambda execution */
    int lambda_result = lambda_no_capture(21);
    lambda_by_ref(42);
    
    /* Initializer list */
    int init_sum = init_list_func({1, 2, 3, 4, 5});
    
    /* Structured bindings */
    auto [x, y] = get_point();
    
    /* Fold expression */
    int fold_result = sum_all(1, 2, 3, 4, 5);
    
    /* Template instantiations with complex types */
    TupleWrapper<int, double, const char*> tuple_wrapper;
    AliasTemplate<float> alias_instance(3.14f);
    
#if __cplusplus >= 202002L
    /* Coroutine usage if C++20 */
    if (mode == 'C') {
        SimpleCoroutine coro;
    }
#endif
#endif
    
    /* Generate observable output based on all type usages */
    unsigned long hash = 0;
    
    /* Mix in sizes of various types */
    hash ^= (unsigned long)sizeof(struct simple_struct);
    hash ^= (unsigned long)sizeof(union data_union);
    hash ^= (unsigned long)sizeof(int_array);
    
    /* Mix in addresses (shifted to avoid exposing actual addresses) */
    hash ^= ((unsigned long)&global_int >> 4);
    hash ^= ((unsigned long)&simple >> 4);
    hash ^= ((unsigned long)&data_u >> 4);
    
    /* Mix in values */
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)result;
    hash ^= (unsigned long)local_int;
    
#ifdef __cplusplus
    hash ^= (unsigned long)lambda_result;
    hash ^= (unsigned long)fold_result;
#endif
    
    /* Print hash to ensure execution */
    printf("Type coverage hash: 0x%016lx\n", hash);
    
    /* Return value based on mode to ensure all code paths are considered */
    return (int)(hash % 256);
}
