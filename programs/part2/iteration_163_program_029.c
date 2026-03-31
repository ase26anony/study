/* gengtype_test.c - Test program to trigger all gengtype type categories */

#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <functional>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct undefined_struct;  /* Forward declaration - never defined */
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
long double global_ldouble = 1.4142135623730950488L;
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char* const global_const_string = "Constant pointer to constant string";

/* ========== TYPE_STRUCT ========== */
/* Basic structure */
struct BasicStruct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct OuterStruct {
    struct BasicStruct inner;
    int outer_field;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
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
    int data[];
};
#endif

/* Anonymous structure */
struct {
    int anonymous_field;
} global_anonymous_struct = {99};

/* ========== TYPE_UNION ========== */
/* C-style union */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
    struct BasicStruct struct_val;
};

/* Union within structure */
struct UnionContainer {
    int type;
    union {
        int int_member;
        float float_member;
        struct BasicStruct struct_member;
    } data;
};

/* Anonymous union (C++11) */
#ifdef __cplusplus
struct AnonymousUnionContainer {
    int tag;
    union {
        int as_int;
        double as_double;
    };
};
#endif

/* ========== TYPE_ARRAY ========== */
/* Various arrays */
int global_int_array[10] = {0,1,2,3,4,5,6,7,8,9};
double global_double_array[5][3];  /* Multi-dimensional */
struct BasicStruct global_struct_array[4];
int* global_pointer_array[8];

/* Array of function pointers */
typedef int (*func_ptr_t)(int);
func_ptr_t global_func_ptr_array[3];

/* Variable-length array (C99) */
#ifndef __cplusplus
void process_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointers */
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
int* const global_int_const_ptr = &global_int;
const int* const global_const_int_const_ptr = &global_int;
volatile int* global_volatile_int_ptr = &global_int;
int* restrict global_restrict_int_ptr = &global_int;

/* Double pointer */
int** global_double_ptr = &global_int_ptr;

/* Pointer to array */
int (*global_ptr_to_array)[10] = &global_int_array;

/* Pointer to structure */
struct BasicStruct* global_struct_ptr;

/* Pointer to union */
union DataUnion* global_union_ptr;

/* Pointer to pointer to function */
int (*(*global_ppfunc)(int))(int);

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);
typedef double (*MathFunc)(double);

/* Structure with function pointer */
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

/* Function using callback */
void register_callback(Callback cb, void* data) {
    /* Callback would be stored somewhere */
    (void)cb;
    (void)data;
}

/* Actual callback function */
void my_callback(int value, void* data) {
    *(int*)data = value * 2;
}

/* ========== C++ Specific Types ========== */
#ifdef __cplusplus

/* ========== TYPE_USER_STRUCT ========== */
/* Base class */
class BaseClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_method() = 0;
    void concrete_method() { private_data++; }
};

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    virtual void virtual_method() override {
        derived_data = protected_data * 2.0;
    }
    void derived_method() { derived_data += 1.0; }
};

/* Multiple inheritance */
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

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    virtual void method1() override {}
    virtual void method2() override {}
};

/* Template class */
template<typename T>
class TemplateClass {
private:
    T data;
public:
    TemplateClass(const T& val) : data(val) {}
    T get_data() const { return data; }
    void set_data(const T& val) { data = val; }
};

/* Class with complex members */
class ComplexClass {
public:
    ComplexClass() : ptr(new int(42)), ref(*ptr) {}
    ~ComplexClass() { delete ptr; }
    
    ComplexClass(const ComplexClass&) = delete;
    ComplexClass& operator=(const ComplexClass&) = delete;
    
private:
    int* ptr;
    int& ref;
    static int static_member;
};

int ComplexClass::static_member = 100;

/* ========== TYPE_LANG_STRUCT ========== */
/* Lambda expressions */
auto lambda1 = []() { return 42; };
auto lambda2 = [global_int](int x) -> int { return x + global_int; };
auto lambda3 = [&global_double](double d) mutable { global_double += d; };

/* Function using lambda */
template<typename Func>
void execute_lambda(Func f) {
    f();
}

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

/* Initializer list usage */
class InitializerListUser {
public:
    InitializerListUser(std::initializer_list<int> list) {
        sum = 0;
        for (int val : list) {
            sum += val;
        }
    }
private:
    int sum;
};

#endif /* __cplusplus */

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    undefined_ptr_t undefined_ptr = 0;
    (void)undefined_ptr;
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned local_uint = 200u;
    float local_float = 1.5f;
    double local_double = 3.14;
    _Bool local_bool = 0;
    
    /* TYPE_STRING usage */
    const char* local_string = "Local string";
    char local_char_array[] = "Array string";
    (void)local_string;
    (void)local_char_array;
    
    /* TYPE_STRUCT usage */
    struct BasicStruct local_struct = {10, 20.5, "Test"};
    struct OuterStruct local_outer = {{5, 10.2, "Inner"}, 99};
    struct BitFieldStruct local_bitfield = {1, 3, 7, 0xFF};
    struct PackedStruct local_packed = {'X', 42, 3.14};
    
    /* TYPE_UNION usage */
    union DataUnion local_union;
    local_union.int_val = 100;
    
    struct UnionContainer local_union_container;
    local_union_container.type = 1;
    local_union_container.data.int_member = 42;
    
    /* TYPE_ARRAY usage */
    int local_int_array[5] = {1, 2, 3, 4, 5};
    double local_multi_array[2][3] = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    struct BasicStruct local_struct_array[2] = {{1, 2.0, "A"}, {3, 4.0, "B"}};
    
#ifndef __cplusplus
    /* C-specific array features */
    if (argc > 2) {
        process_vla(10);
    }
#endif
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    int** local_double_ptr = &local_int_ptr;
    struct BasicStruct* local_struct_ptr = &local_struct;
    union DataUnion* local_union_ptr = &local_union;
    
    /* TYPE_CALLBACK usage */
    Callback local_callback = my_callback;
    int callback_data = 0;
    register_callback(local_callback, &callback_data);
    
    struct CallbackContainer callback_container = {my_callback, &callback_data};
    
    /* Switch based on mode to use different types */
    switch (mode) {
        case 'A':
            /* Use scalar types */
            local_int += global_int;
            local_float *= global_float;
            break;
            
        case 'B':
            /* Use structure types */
            local_struct.x += local_outer.outer_field;
            local_outer.inner.y = local_packed.c;
            break;
            
        case 'C':
            /* Use array types */
            for (int i = 0; i < 5; i++) {
                local_int_array[i] += global_int_array[i % 10];
            }
            break;
            
        case 'D':
            /* Use pointer types */
            **local_double_ptr = 999;
            local_struct_ptr->x = 123;
            break;
            
        case 'E':
            /* Use callback */
            local_callback(42, &callback_data);
            break;
    }
    
#ifdef __cplusplus
    /* ========== C++ Specific Usage ========== */
    
    /* TYPE_USER_STRUCT usage */
    DerivedClass derived_obj;
    derived_obj.virtual_method();
    derived_obj.concrete_method();
    
    MultipleInheritanceClass multi_obj;
    multi_obj.method1();
    multi_obj.method2();
    
    /* Template instantiation */
    TemplateClass<int> int_template(100);
    TemplateClass<double> double_template(3.14);
    TemplateClass<BasicStruct> struct_template({1, 2.0, "Template"});
    
    /* TYPE_LANG_STRUCT usage */
    /* Lambda usage */
    int lambda_result = lambda1();
    execute_lambda(lambda1);
    
    /* Structured bindings */
    auto [x, y] = get_point();
    (void)x;
    (void)y;
    
    /* Fold expressions */
    int fold_result = sum(1, 2, 3, 4, 5);
    (void)fold_result;
    
    /* Initializer list */
    InitializerListUser il_user = {1, 2, 3, 4, 5};
    (void)il_user;
    
#if __cplusplus >= 202002L
    /* Coroutine */
    auto coroutine_func = []() -> SimpleCoroutine {
        co_return;
    };
    coroutine_func();
#endif
    
    /* Complex class */
    ComplexClass complex_obj;
    (void)complex_obj;
    
#endif /* __cplusplus */
    
    /* Generate observable output to prevent optimization */
    unsigned long hash = 0;
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)&local_struct;
    hash ^= (unsigned long)local_int_ptr;
    hash ^= (unsigned long)sizeof(local_union);
    
#ifdef __cplusplus
    std::cout << "Hash: " << hash << std::endl;
#else
    printf("Hash: %lu\n", hash);
#endif
    
    return (int)(hash % 256);
}

/* Additional functions to ensure types are used */
void use_all_pointers(void) {
    /* Force analysis of all pointer types */
    (void)global_int_ptr;
    (void)global_const_int_ptr;
    (void)global_int_const_ptr;
    (void)global_const_int_const_ptr;
    (void)global_volatile_int_ptr;
    (void)global_restrict_int_ptr;
    (void)global_double_ptr;
    (void)global_ptr_to_array;
    (void)global_struct_ptr;
    (void)global_union_ptr;
    (void)global_ppfunc;
}

/* Function with complex parameter types */
void complex_parameters(int scalar, 
                       struct BasicStruct s,
                       union DataUnion u,
                       int array[5],
                       int* ptr,
                       Callback cb) {
    (void)scalar;
    (void)s;
    (void)u;
    (void)array;
    (void)ptr;
    (void)cb;
}
