#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
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
long double global_ldouble = 1.618033988749895L;
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic C structure
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

// Nested structure
struct OuterStruct {
    int id;
    struct InnerStruct {
        float data;
        char tag;
    } inner;
    struct SimpleStruct simple;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
    unsigned int : 18;  // Padding
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
    double data[];  // Flexible array member
};
#endif

// Anonymous structure
struct {
    int anonymous_id;
    float anonymous_data;
} anonymous_global = {1, 3.14f};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Base class with different access specifiers
class BaseClass {
public:
    BaseClass() : public_data(100) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_func() { std::cout << "BaseClass::virtual_func\n"; }
    void public_func() {}
    
protected:
    int protected_data;
    
private:
    int private_data;
    int public_data;
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(200) {}
    void virtual_func() override { std::cout << "DerivedClass::virtual_func\n"; }
    
private:
    int derived_data;
};

// Multiple inheritance
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

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    void interface1_func() override {}
    void interface2_func() override {}
};

// Template class
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    
    T get_data() const { return data; }
    void set_data(T val) { data = val; }
    
private:
    T data;
};

// Class with static members
class StaticMemberClass {
public:
    static int static_counter;
    static const double static_pi;
    
    StaticMemberClass() { static_counter++; }
};

int StaticMemberClass::static_counter = 0;
const double StaticMemberClass::static_pi = 3.14159;

// Class with member functions of different types
class FunctionClass {
public:
    void normal_func() {}
    void const_func() const {}
    static void static_func() {}
    virtual void pure_virtual() = 0;
    virtual void virtual_func() {}
};

#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char string_value[16];
    void* ptr_value;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct {
            char a;
            char b;
        } char_pair;
    } data;
};

// Anonymous union in C++11
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        char as_string[20];
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

// Pointer to incomplete type
incomplete_ptr_t incomplete_ptr = nullptr;

// Pointers to structures
struct SimpleStruct* simple_struct_ptr = nullptr;
struct OuterStruct* outer_struct_ptr = nullptr;

// Pointer to array
int (*array_ptr)[10] = nullptr;

// Pointer to function
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);

// Pointer to member function (C++)
#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];  // 2D array
double double_array[2][3][4];  // 3D array
char* string_array[5] = {"one", "two", "three", "four", "five"};

// Array of structures
struct SimpleStruct struct_array[3];

// Array of pointers
int* pointer_array[8];

// Incomplete array type in structure (C only)
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int data[0];  // Incomplete array
};
#endif

// Variable Length Array (C99)
#ifndef __cplusplus
void use_vla(int n) {
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = i * 2;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

// Structure with function pointer member
struct CallbackContainer {
    const char* name;
    callback_t callback;
    void* user_data;
};

// Function using callback
void register_callback(callback_t cb, void* data) {
    if (cb) cb(42, data);
}

// Example callback function
void example_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename Func>
void template_callback(Func f) {
    f(100);
}

// std::function
std::function<void(int)> std_function_callback;

// Lambda with different captures
auto lambda_with_capture = [global_int](int x) {
    return x + global_int;
};
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions
auto simple_lambda = [](int x) { return x * 2; };
auto capturing_lambda = [global_int, &global_double](float f) {
    return f * global_int + global_double;
};
auto mutable_lambda = [counter = 0]() mutable {
    return ++counter;
};

// std::initializer_list
auto init_list_example = {1, 2, 3, 4, 5};

// Structured bindings (C++17)
struct Point { int x; int y; };
auto structured_binding_example() {
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
    return x_coord + y_coord;
}

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#if __cplusplus >= 202002L
#include <coroutine>
struct SimpleAwaitable {
    struct promise_type {
        SimpleAwaitable get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleAwaitable coroutine_example() {
    co_return;
}
#endif

// Template with complex type deduction
template<typename T, typename U>
auto add_values(T t, U u) -> decltype(t + u) {
    return t + u;
}

// Variadic template
template<typename... Ts>
struct TupleWrapper {
    std::tuple<Ts...> data;
};

#endif

/* ========== Helper functions ========== */
int compare_ints(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

void process_scalars(int argc) {
    // Use all scalar types
    char local_char = 'Z';
    int local_int = argc;
    float local_float = 3.14f * argc;
    double local_double = 2.71828 * argc;
    _Bool local_bool = argc > 1;
    
    // Use signed and unsigned
    signed int signed_int = -argc;
    unsigned int unsigned_int = argc * 2u;
    
    // Force usage
    global_int += local_int;
    global_float += local_float;
}

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* ========== TYPE_UNDEFINED usage ========== */
    incomplete_ptr_t local_incomplete_ptr = nullptr;
    
    /* ========== TYPE_SCALAR usage ========== */
    process_scalars(argc);
    
    /* ========== TYPE_STRING usage ========== */
    const char* local_string = "Local string";
    wchar_t wide_local[] = L"Wide local";
    char buffer[100];
    
    /* ========== TYPE_STRUCT usage ========== */
    struct SimpleStruct simple = {10, 3.14, "test"};
    struct OuterStruct outer = {1, {2.5f, 'X'}, {20, 6.28, "nested"}};
    struct BitFieldStruct bitfield = {1, 3, 7, -500};
    struct PackedStruct packed = {'Z', 999, 2.71828};
    
    // Use anonymous struct
    anonymous_global.anonymous_id = argc;
    
    /* ========== TYPE_UNION usage ========== */
    union DataUnion data_union;
    data_union.int_value = argc;
    
    struct UnionContainer union_container;
    union_container.type = 1;
    union_container.data.int_data = 100;
    
    #ifdef __cplusplus
    AnonymousUnionStruct anon_union;
    anon_union.tag = 2;
    anon_union.as_int = 200;
    #endif
    
    /* ========== TYPE_POINTER usage ========== */
    int* local_int_ptr = &global_int;
    int** local_double_ptr = &local_int_ptr;
    
    // Function pointer
    func_ptr_t local_func_ptr = nullptr;
    
    /* ========== TYPE_ARRAY usage ========== */
    int local_array[5] = {1, 2, 3, 4, 5};
    float matrix[2][2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};
    
    // Use arrays
    for (int i = 0; i < 5; i++) {
        local_array[i] *= argc;
    }
    
    #ifndef __cplusplus
    // Use VLA in C mode
    if (argc > 1) {
        use_vla(argc);
    }
    #endif
    
    /* ========== TYPE_CALLBACK usage ========== */
    struct CallbackContainer cb_container = {"test", example_callback, &global_int};
    
    int callback_data = 0;
    register_callback(example_callback, &callback_data);
    
    // Use function pointer
    comparator_t cmp_ptr = compare_ints;
    qsort(local_array, 5, sizeof(int), cmp_ptr);
    
    #ifdef __cplusplus
    // C++ callbacks
    template_callback([](int x) { global_int += x; });
    
    std_function_callback = [](int x) { global_double += x; };
    std_function_callback(50);
    #endif
    
    /* ========== TYPE_USER_STRUCT usage (C++ only) ========== */
    #ifdef __cplusplus
    BaseClass* base_ptr = nullptr;
    DerivedClass derived;
    base_ptr = &derived;
    
    if (mode == 'B') {
        base_ptr->virtual_func();
    }
    
    // Template instantiation
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    TemplateClass<BaseClass*> ptr_template(nullptr);
    
    // Multiple template instantiations
    int_template.set_data(argc);
    double_template.set_data(global_double);
    
    // Static members
    StaticMemberClass obj1, obj2;
    #endif
    
    /* ========== TYPE_LANG_STRUCT usage (C++ only) ========== */
    #ifdef __cplusplus
    // Lambda usage
    auto result = simple_lambda(argc);
    auto result2 = capturing_lambda(3.14f);
    
    // Initializer list
    for (auto val : init_list_example) {
        global_int += val;
    }
    
    // Structured binding
    if (mode == 'C') {
        auto sum = structured_binding_example();
        global_int += sum;
    }
    
    // Fold expression
    if (mode == 'D') {
        auto total = sum_all(1, 2, 3, 4, 5);
        global_int += total;
    }
    
    // Complex template
    auto added = add_values(global_int, global_double);
    
    // Variadic template instantiation
    TupleWrapper<int, double, char> tuple_wrapper;
    
    #if __cplusplus >= 202002L
    // Coroutine
    if (mode == 'E') {
        coroutine_example();
    }
    #endif
    #endif
    
    /* ========== Prevent optimization ========== */
    // Create observable output based on all types
    unsigned long long hash = 0;
    
    // Mix in addresses and sizes
    hash ^= (unsigned long long)&global_int;
    hash ^= (unsigned long long)&simple;
    hash ^= (unsigned long long)&data_union;
    hash ^= (unsigned long long)local_array;
    #ifdef __cplusplus
    hash ^= (unsigned long long)&derived;
    #endif
    
    // Mix in sizes
    hash ^= sizeof(struct SimpleStruct);
    hash ^= sizeof(union DataUnion);
    hash ^= sizeof(int_array);
    #ifdef __cplusplus
    hash ^= sizeof(DerivedClass);
    #endif
    
    // Mix in values
    hash ^= global_int;
    hash ^= (unsigned long long)global_double;
    hash ^= callback_data;
    
    // Final output to prevent dead code elimination
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << "\n";
    std::cout << "Mode: " << mode << "\n";
    std::cout << "Global int: " << global_int << "\n";
    std::cout << "Global double: " << global_double << "\n";
    #else
    printf("Hash: %llu\n", hash);
    printf("Mode: %c\n", mode);
    printf("Global int: %d\n", global_int);
    printf("Global double: %f\n", global_double);
    #endif
    
    return (int)(hash % 256);
}
