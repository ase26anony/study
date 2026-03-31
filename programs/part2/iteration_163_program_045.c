/* gengtype_test.c - Test program to trigger all gengtype type categories */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;
#else
#include <stdio.h>
#include <stdbool.h>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  /* Forward declaration - never defined */
typedef struct incomplete_struct* incomplete_ptr_t;

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
#else
_Bool global_bool = 1;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character array";
const wchar_t* global_wstring = L"Wide string";
const wchar_t global_wchar_array[] = L"Wide array";

/* ========== TYPE_STRUCT ========== */
/* Simple C structure */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct OuterStruct {
    struct SimpleStruct inner;
    int outer_value;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
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

/* Global structure instances */
struct SimpleStruct global_struct = {10, 3.14, "Test"};
struct OuterStruct global_outer = {{{5, 2.718, "Inner"}, 100}};

/* ========== TYPE_USER_STRUCT ========== */
#ifdef __cplusplus
/* Base class with different access specifiers */
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass() : private_member(0), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() { private_member++; }
};

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass() : derived_private(0.0) {}
    void virtual_func() override { derived_private = 1.0; }
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

class MultipleInheritance : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

/* Template class */
template<typename T>
class TemplateClass {
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T get() const { return value; }
    void set(T v) { value = v; }
};

/* Instantiate template classes */
TemplateClass<int> global_template_int(42);
TemplateClass<double> global_template_double(3.14);
TemplateClass<const char*> global_template_string("Template");

/* Class with complex members */
class ComplexClass {
    int* dynamic_array;
    size_t size;
public:
    ComplexClass(size_t s) : size(s) {
        dynamic_array = new int[s];
    }
    ~ComplexClass() {
        delete[] dynamic_array;
    }
    ComplexClass(const ComplexClass&) = delete;
    ComplexClass& operator=(const ComplexClass&) = delete;
};
#endif

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
        char* string_value;
    } data;
};

/* C++11 anonymous union */
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        void* as_pointer;
    };
};
#endif

/* Global union instances */
union DataUnion global_union = {.i = 42};
struct UnionContainer global_union_container = {1, {.int_value = 100}};

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

/* Pointer to function */
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to structure */
struct SimpleStruct* struct_ptr = &global_struct;

/* Pointer to incomplete type */
incomplete_ptr_t incomplete_ptr = NULL;

#ifdef __cplusplus
/* Pointer to member function */
typedef void (BaseClass::*member_func_ptr_t)();
/* Pointer to member data */
typedef int BaseClass::*member_data_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int int_array[10] = {0,1,2,3,4,5,6,7,8,9};
float float_array[5][4];  /* 2D array */
double triple_array[2][3][4];  /* 3D array */
char* pointer_array[8];  /* Array of pointers */

/* Array of structures */
struct SimpleStruct struct_array[3] = {
    {1, 1.1, "First"},
    {2, 2.2, "Second"},
    {3, 3.3, "Third"}
};

/* Array of unions */
union DataUnion union_array[4];

/* Incomplete array in structure (C only) */
#ifndef __cplusplus
struct IncompleteArrayStruct {
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
/* Function pointer typedef */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

/* Structure with function pointer */
struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

/* Function using callback */
void register_callback(callback_t cb, void* data) {
    /* Callback would be stored somewhere */
    (void)cb;
    (void)data;
}

/* Actual callback function */
void my_callback(int value, void* data) {
    (void)value;
    (void)data;
}

#ifdef __cplusplus
/* C++ function objects and lambdas */
template<typename Func>
void template_callback(Func f) {
    f(42);
}

/* Function returning function pointer */
std::function<int(int)> get_function() {
    return [](int x) { return x * 2; };
}
#endif

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
/* Lambda expressions with different captures */
auto lambda_no_capture = []() { return 42; };
auto lambda_by_value = [global_int]() { return global_int + 1; };
auto lambda_by_ref = [&global_double]() { return global_double * 2.0; };
auto lambda_mixed = [global_int, &global_double](float f) -> double {
    return global_int + global_double + f;
};

/* std::initializer_list usage */
auto init_list_func(std::initializer_list<int> list) {
    int sum = 0;
    for (auto x : list) sum += x;
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
    T value;
public:
    LanguageFeatureContainer(std::initializer_list<T> init) {
        for (const auto& v : init) {
            value = v;  /* Just take the last one */
        }
    }
    
    auto get_lambda() const {
        return [this]() { return value; };
    }
};
#endif

/* ========== Helper functions ========== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void process_scalars(int argc) {
    /* Local scalar variables */
    int local_int = argc;
    unsigned int local_uint = argc * 2u;
    char local_char = 'A' + (argc % 26);
    float local_float = argc * 0.5f;
    double local_double = argc * 1.5;
#ifdef __cplusplus
    bool local_bool = argc > 0;
#else
    _Bool local_bool = argc > 0;
#endif
    
    /* Use all locals to prevent optimization */
    local_int += local_uint + local_char;
    local_float += local_double;
    (void)local_bool;
}

void process_arrays(int argc) {
    /* Use arrays */
    for (int i = 0; i < 10 && i < argc; i++) {
        int_array[i] = argc + i;
    }
    
    /* Multi-dimensional array access */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                triple_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
}

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    unsigned long hash = 0;
    
    /* Use argc to control execution flow */
    if (argc > 1) {
        /* Branch 1: Process scalars and basic types */
        process_scalars(argc);
        
        /* Use incomplete type pointer */
        incomplete_ptr = (incomplete_ptr_t)&global_int;
        hash += (unsigned long)incomplete_ptr;
        
        /* Use strings */
        hash += (unsigned long)global_string[0];
        hash += (unsigned long)global_wstring[0];
    }
    
    if (argc > 2) {
        /* Branch 2: Process structures and unions */
        global_struct.x = argc;
        global_outer.outer_value = argc * 2;
        
        /* Use unions */
        global_union.f = argc * 0.5f;
        global_union_container.data.float_value = argc * 1.5f;
        
        hash += global_struct.x + global_outer.outer_value;
        hash += (unsigned long)&global_union;
    }
    
    if (argc > 3) {
        /* Branch 3: Process arrays */
        process_arrays(argc);
        
        /* Use array of structures */
        for (int i = 0; i < 3 && i < argc; i++) {
            struct_array[i].x = argc + i * 10;
            hash += struct_array[i].x;
        }
    }
    
    if (argc > 4) {
        /* Branch 4: Use pointers */
        *int_ptr = argc;
        **double_int_ptr = argc * 2;
        
        /* Use function pointer */
        comparator_t comp = compare_ints;
        int a = 5, b = 10;
        hash += comp(&a, &b);
        
        /* Use callback */
        struct CallbackContainer cb_container = {my_callback, &hash};
        register_callback(cb_container.callback, cb_container.user_data);
    }
    
#ifdef __cplusplus
    if (argc > 5) {
        /* Branch 5: C++ specific features */
        DerivedClass derived;
        BaseClass* base_ptr = &derived;
        base_ptr->virtual_func();
        
        /* Use template classes */
        global_template_int.set(argc);
        hash += global_template_int.get();
        
        /* Use lambdas */
        auto result = lambda_mixed(3.14f);
        hash += (unsigned long)result;
        
        /* Use initializer_list */
        hash += init_list_func({1, 2, 3, argc});
        
        /* Use structured bindings */
        auto [x, y] = get_point();
        hash += (unsigned long)(x + y);
        
        /* Use fold expression */
        hash += sum_all(1, 2, 3, argc);
        
        /* Use language feature container */
        LanguageFeatureContainer<int> lfc({10, 20, 30, argc});
        auto lambda = lfc.get_lambda();
        hash += lambda();
        
        /* Use template callback */
        template_callback([](int val) { (void)val; });
    }
#endif
    
#ifndef __cplusplus
    /* C-specific features */
    if (argc > 6) {
        /* Use VLA */
        use_vla(argc > 10 ? 10 : argc);
    }
#endif
    
    /* Final output to prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Type coverage hash: " << hash << std::endl;
#else
    printf("Type coverage hash: %lu\n", hash);
#endif
    
    return (int)(hash % 256);
}
