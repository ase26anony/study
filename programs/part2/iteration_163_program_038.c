/* gengtype_test.c - Comprehensive type coverage test */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <vector>
using namespace std;

// For TYPE_LANG_STRUCT: C++20 coroutines (if supported)
#ifdef __cpp_coroutines
template<typename T>
struct generator {
    struct promise_type;
    using handle_type = coroutine_handle<promise_type>;
    
    struct promise_type {
        T current_value;
        
        auto get_return_object() { return generator{handle_type::from_promise(*this)}; }
        auto initial_suspend() { return suspend_always{}; }
        auto final_suspend() noexcept { return suspend_always{}; }
        void unhandled_exception() { terminate(); }
        auto yield_value(T value) {
            current_value = value;
            return suspend_always{};
        }
        void return_void() {}
    };
    
    handle_type coro;
    
    explicit generator(handle_type h) : coro(h) {}
    ~generator() { if (coro) coro.destroy(); }
    
    bool move_next() {
        if (!coro.done()) {
            coro.resume();
            return !coro.done();
        }
        return false;
    }
    T current_value() { return coro.promise().current_value; }
};
#endif

// Forward declarations for TYPE_UNDEFINED
struct undefined_struct;  // Never defined
union undefined_union;    // Never defined
class undefined_class;    // Never defined

// TYPE_UNDEFINED: Pointer to incomplete type
undefined_struct* global_undefined_ptr = nullptr;
undefined_union** double_undefined_ptr = nullptr;
undefined_class* volatile volatile_undefined_ptr = nullptr;

#endif

/* TYPE_SCALAR: Fundamental types in various contexts */
int global_int = 42;
static unsigned int static_uint = 100U;
const float global_float = 3.14f;
volatile double volatile_double = 2.71828;
_Bool global_bool = 1;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 5000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;

/* TYPE_STRING: String literals and character arrays */
const char* global_cstring = "Hello, World!";
static const char* static_cstring = "Static string";
char global_char_array[50] = "Character array";
const wchar_t* global_wstring = L"Wide string";
char16_t utf16_string[] = u"UTF-16 string";
char32_t utf32_string[] = U"UTF-32 string";

/* TYPE_STRUCT: C structures with various compositions */
struct SimpleStruct {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
};

struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

/* TYPE_UNION: C-style unions */
union SimpleUnion {
    int as_int;
    float as_float;
    double as_double;
    char as_char[8];
};

union ComplexUnion {
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            void* ptr_value;
        } data;
    } tagged;
    long long raw;
};

/* TYPE_ARRAY: Arrays of various dimensions */
int global_1d_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int global_2d_array[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};
int global_3d_array[2][3][4];
char* global_ptr_array[5];
SimpleStruct global_struct_array[5];

/* TYPE_CALLBACK: Function pointers */
typedef int (*SimpleCallback)(int, int);
typedef void (*VoidCallback)(void);
typedef char* (*StringCallback)(const char*);

struct CallbackContainer {
    SimpleCallback func1;
    VoidCallback func2;
    StringCallback func3;
};

/* Function prototypes */
int add(int a, int b);
void print_message(void);
char* duplicate_string(const char* str);
void use_callbacks(SimpleCallback cb1, VoidCallback cb2);

#ifdef __cplusplus
/* TYPE_USER_STRUCT: C++ classes with various features */

// Simple class with different access specifiers
class BaseClass {
public:
    BaseClass() : public_member(0) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_method() { cout << "BaseClass::virtual_method" << endl; }
    void public_method() { cout << "BaseClass::public_method" << endl; }
    
    int public_member;
    
protected:
    void protected_method() { cout << "BaseClass::protected_method" << endl; }
    int protected_member;
    
private:
    void private_method() { cout << "BaseClass::private_method" << endl; }
    int private_member;
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_member(0) {}
    ~DerivedClass() override {}
    
    void virtual_method() override { cout << "DerivedClass::virtual_method" << endl; }
    
    int derived_member;
};

// Multiple inheritance
class Interface1 {
public:
    virtual void interface1_method() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void interface2_method() = 0;
    virtual ~Interface2() {}
};

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    void interface1_method() override { cout << "interface1_method" << endl; }
    void interface2_method() override { cout << "interface2_method" << endl; }
};

// Template class
template<typename T>
class TemplateClass {
public:
    TemplateClass(T value) : data(value) {}
    
    T get_data() const { return data; }
    void set_data(T value) { data = value; }
    
    template<typename U>
    U convert() const { return static_cast<U>(data); }
    
private:
    T data;
};

// Class with anonymous union (C++11)
class ClassWithAnonymousUnion {
public:
    ClassWithAnonymousUnion() : type(0) {}
    
    union {
        int int_value;
        float float_value;
        double double_value;
    };
    
    int type;
};

// Class with std::initializer_list (TYPE_LANG_STRUCT)
class InitializerListClass {
public:
    InitializerListClass(initializer_list<int> list) {
        for (auto val : list) {
            values.push_back(val);
        }
    }
    
private:
    vector<int> values;
};

// Lambda expressions (TYPE_LANG_STRUCT)
auto lambda1 = [](int x) -> int { return x * 2; };
auto lambda2 = [global_int](double x) -> double { return x + global_int; };
auto lambda3 = [&](auto x) { return x + 1; };  // Generic lambda

// Structured bindings (C++17, TYPE_LANG_STRUCT)
struct Point3D {
    float x, y, z;
};

auto get_point() -> Point3D {
    return {1.0f, 2.0f, 3.0f};
}

// Fold expressions (C++17, TYPE_LANG_STRUCT)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

#endif

/* TYPE_POINTER: Various pointer types */
int* global_int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;

double** global_double_ptr_ptr = nullptr;
SimpleStruct* global_struct_ptr = nullptr;
SimpleUnion* global_union_ptr = nullptr;

/* Function pointer variations */
int (*global_func_ptr)(int, int) = &add;
void (*global_void_func_ptr)(void) = &print_message;
char* (*global_string_func_ptr)(const char*) = &duplicate_string;

/* Pointer to array */
int (*global_array_ptr)[10] = &global_1d_array;
int (*global_2d_array_ptr)[3][4] = &global_2d_array;

#ifdef __cplusplus
/* Pointer to member function */
typedef void (BaseClass::*MemberFuncPtr)();
MemberFuncPtr global_member_func_ptr = &BaseClass::virtual_method;

/* Pointer to data member */
typedef int BaseClass::*MemberDataPtr;
MemberDataPtr global_member_data_ptr = &BaseClass::public_member;
#endif

/* Function definitions */
int add(int a, int b) {
    return a + b;
}

void print_message(void) {
#ifdef __cplusplus
    cout << "Message printed" << endl;
#else
    printf("Message printed\n");
#endif
}

char* duplicate_string(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) strcpy(copy, str);
    return copy;
}

void use_callbacks(SimpleCallback cb1, VoidCallback cb2) {
    if (cb1) {
        int result = cb1(10, 20);
#ifdef __cplusplus
        cout << "Callback result: " << result << endl;
#else
        printf("Callback result: %d\n", result);
#endif
    }
    if (cb2) {
        cb2();
    }
}

/* Main function with command-line dependent execution */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? atoi(argv[1]) % 10 : 0;
    
    /* TYPE_SCALAR: Local variables */
    int local_int = 100;
    unsigned local_uint = 200U;
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
    /* TYPE_STRING: Local strings */
    const char* local_cstring = "Local string";
    char local_char_array[] = "Local array";
    
    /* TYPE_STRUCT: Local structure instances */
    struct SimpleStruct local_struct = {1, 2.0, 'A'};
    struct NestedStruct local_nested = {{10, 20.0, 'B'}, 30};
    struct BitfieldStruct local_bitfield = {1, 3, 5, 100};
    
    /* TYPE_UNION: Local union instances */
    union SimpleUnion local_union;
    local_union.as_int = 42;
    
    /* TYPE_ARRAY: Local arrays */
    int local_1d_array[5] = {1, 2, 3, 4, 5};
    int local_2d_array[2][3] = {{1,2,3}, {4,5,6}};
    
    /* TYPE_POINTER: Local pointers */
    int* local_int_ptr = &local_int;
    int** local_int_ptr_ptr = &local_int_ptr;
    
    /* TYPE_CALLBACK: Local function pointers */
    SimpleCallback local_callback = &add;
    
    /* Force usage based on mode */
    switch (mode) {
        case 0:
            /* Use all scalar types */
            local_int += global_int + static_uint + global_char;
            local_float += global_float + volatile_double;
            break;
            
        case 1:
            /* Use string types */
#ifdef __cplusplus
            cout << global_cstring << endl;
            cout << global_wstring << endl;
#else
            printf("%s\n", global_cstring);
#endif
            break;
            
        case 2:
            /* Use structures */
            local_struct.x = local_nested.outer;
            local_bitfield.flag1 = local_bitfield.flag2;
            break;
            
        case 3:
            /* Use unions */
            local_union.as_float = local_float;
            break;
            
        case 4:
            /* Use arrays */
            for (int i = 0; i < 5; i++) {
                local_1d_array[i] = global_1d_array[i];
            }
            break;
            
        case 5:
            /* Use pointers */
            *local_int_ptr = **local_int_ptr_ptr + 1;
            break;
            
        case 6:
            /* Use callbacks */
            use_callbacks(local_callback, &print_message);
            break;
            
        default:
            /* Mix everything */
            local_int = add(local_int, global_int);
            break;
    }
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT: C++ class usage */
    BaseClass* base_ptr = nullptr;
    DerivedClass derived_obj;
    base_ptr = &derived_obj;
    
    if (mode % 2 == 0) {
        base_ptr->virtual_method();
        base_ptr->public_method();
    }
    
    /* Template instantiation */
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    TemplateClass<SimpleStruct> struct_template({1, 2.0, 'A'});
    
    /* Multiple inheritance */
    MultipleInheritanceClass multi_inherit;
    Interface1* if1 = &multi_inherit;
    Interface2* if2 = &multi_inherit;
    
    if (mode % 3 == 0) {
        if1->interface1_method();
        if2->interface2_method();
    }
    
    /* TYPE_LANG_STRUCT: C++ specific constructs */
    
    // Lambda usage
    auto result1 = lambda1(21);
    auto result2 = lambda2(3.14);
    auto result3 = lambda3(10);
    
    // Initializer list
    InitializerListClass init_list = {1, 2, 3, 4, 5};
    
    // Structured bindings (C++17)
    auto [x, y, z] = get_point();
    
    // Fold expression
    auto fold_result = sum_all(1, 2, 3, 4, 5);
    
    // Coroutine (if supported)
    #ifdef __cpp_coroutines
    if (mode == 7) {
        auto coro_func = []() -> generator<int> {
            co_yield 1;
            co_yield 2;
            co_yield 3;
        };
        
        auto gen = coro_func();
        while (gen.move_next()) {
            auto val = gen.current_value();
        }
    }
    #endif
    
    /* Pointer to member usage */
    (derived_obj.*global_member_func_ptr)();
    derived_obj.*global_member_data_ptr = 100;
#endif
    
    /* TYPE_UNDEFINED: Force analysis of undefined types */
    /* These are declared but never defined, creating TYPE_UNDEFINED */
    undefined_struct** local_undefined_ptr_ptr = &global_undefined_ptr;
    (void)local_undefined_ptr_ptr;  // Suppress unused warning
    
    /* Generate observable output to prevent optimization */
    unsigned long hash = 0;
    hash ^= (unsigned long)global_int_ptr;
    hash ^= (unsigned long)local_int_ptr;
    hash ^= sizeof(local_struct);
    hash ^= sizeof(local_union);
    hash ^= (unsigned long)&add;
    hash ^= (unsigned long)&print_message;
    
#ifdef __cplusplus
    hash ^= (unsigned long)&derived_obj;
    hash ^= (unsigned long)base_ptr;
#endif
    
    /* Print hash to ensure execution */
#ifdef __cplusplus
    cout << "Hash: " << hash << endl;
#else
    printf("Hash: %lu\n", hash);
#endif
    
    return (int)(hash % 256);
}
