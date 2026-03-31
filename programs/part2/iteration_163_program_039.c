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
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
// Basic C structure
struct SimpleStruct {
    int x;
    double y;
    char name[32];
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
    signed int value : 8;
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
    int data[];
};
#endif

// Anonymous structure
struct {
    int anonymous_id;
    float anonymous_data;
} anonymous_global;

/* ========== TYPE_USER_STRUCT ========== */
#ifdef __cplusplus
// Simple C++ class
class BaseClass {
public:
    BaseClass() : public_data(0), protected_data(0) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_func() {}
    void public_func() {}
    
    int public_data;
protected:
    float protected_data;
private:
    double private_data;
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(0) {}
    void virtual_func() override {}
    
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

class MultiInheritClass : public Interface1, public Interface2 {
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
    
private:
    T data;
};

// Class with static members
class StaticMemberClass {
public:
    static int static_counter;
    static double static_value;
    
    void increment() { static_counter++; }
};

int StaticMemberClass::static_counter = 0;
double StaticMemberClass::static_value = 3.14;

// Instantiate template classes
TemplateClass<int> int_template(42);
TemplateClass<double> double_template(3.14);
TemplateClass<SimpleStruct> struct_template({1, 2.0, "template"});

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
        int as_int;
        float as_float;
        char as_string[32];
    } data;
};

// Anonymous union in C++
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int i;
        float f;
        double d;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;

double** double_ptr_ptr = &int_ptr;  // Will be reassigned
void* void_ptr = nullptr;
const void* const_void_ptr = nullptr;

// Pointer to incomplete type (triggers TYPE_UNDEFINED)
incomplete_ptr_t incomplete_ptr = nullptr;

// Pointers to structures
struct SimpleStruct* struct_ptr = nullptr;
struct OuterStruct* outer_struct_ptr = nullptr;

// Pointer to union
union DataUnion* union_ptr = nullptr;

// Pointer to array
int (*array_ptr)[10] = nullptr;

// Function pointers
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(const char*);

// Member function pointers (C++)
#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
typedef int (TemplateClass<int>::*template_member_ptr_t)() const;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];  // 2D array
double double_array[2][3][4];  // 3D array

// Array of pointers
int* pointer_array[5];

// Array of structures
struct SimpleStruct struct_array[3] = {
    {1, 1.1, "first"},
    {2, 2.2, "second"},
    {3, 3.3, "third"}
};

// Array of unions
union DataUnion union_array[4];

// String arrays
const char* string_array[] = {"one", "two", "three", nullptr};

// Incomplete array in struct (C only)
#ifndef __cplusplus
struct IncompleteArrayHolder {
    int count;
    int values[];
};
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer
struct CallbackHolder {
    Callback callback;
    void* user_data;
};

// Functions to use as callbacks
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void simple_callback(int value, void* data) {
    // Do nothing
}

#ifdef __cplusplus
// C++ function objects and lambdas
std::function<int(int)> func_object = [](int x) { return x * 2; };

// Template with callback
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

// Lambda with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};
#endif

/* ========== TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
// Lambda expressions
auto simple_lambda = []() { return 42; };
auto capturing_lambda = [global_int, &global_double]() {
    return global_int + static_cast<int>(global_double);
};

// std::initializer_list
auto init_list_func(std::initializer_list<int> list) -> int {
    int sum = 0;
    for (auto x : list) sum += x;
    return sum;
}

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine-related types (C++20)
#ifdef __cpp_coroutines
struct DummyCoroutine {
    struct promise_type {
        DummyCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

DummyCoroutine dummy_coro() {
    co_return;
}
#endif

// Variadic templates
template<typename... Ts>
struct VariadicStruct {};

// Type traits usage
using is_int = std::is_same<int, int>;

#endif

/* ========== Helper Functions ========== */
// Function using various types
void process_types(int argc, char** argv) {
    // Use scalars
    int local_int = argc;
    unsigned int local_uint = static_cast<unsigned int>(argc);
    float local_float = 3.14f * argc;
    
    // Use strings
    const char* local_string = argv[0];
    char local_buffer[256];
    
    // Use structures
    struct SimpleStruct local_struct = {argc, 3.14, "local"};
    struct OuterStruct local_outer = {1, {2.0f, 'X'}, {3, 4.0, "nested"}};
    
    // Use unions
    union DataUnion local_union;
    local_union.int_value = argc;
    
    // Use pointers
    int* local_ptr = &local_int;
    void** local_void_ptr_ptr = &void_ptr;
    
    // Use arrays
    int local_array[argc > 10 ? 10 : 5];  // Variable length if C99
    for (int i = 0; i < (argc > 10 ? 10 : 5); i++) {
        local_array[i] = i * argc;
    }
    
    // Use function pointers
    Comparator local_comparator = compare_ints;
    Callback local_callback = simple_callback;
    
    // Use callback structure
    struct CallbackHolder local_callback_holder = {simple_callback, nullptr};
    
#ifdef __cplusplus
    // Use C++ classes
    BaseClass* base_ptr = new DerivedClass();
    base_ptr->virtual_func();
    delete base_ptr;
    
    // Use template instances
    int_template.get_data();
    double_template.get_data();
    
    // Use lambdas
    auto result = simple_lambda();
    auto captured_result = capturing_lambda();
    
    // Use initializer_list
    auto list_sum = init_list_func({1, 2, 3, 4, 5});
    
    // Use structured bindings
    auto [x, y] = get_point();
    
    // Use fold expression
    auto total = sum_all(1, 2, 3, 4, 5);
    
    // Use variadic template
    VariadicStruct<int, float, double> variadic_instance;
#endif
    
    // Prevent optimization
    volatile int anti_optimize = local_int + local_union.int_value;
    (void)anti_optimize;
}

/* ========== Main Function ========== */
int main(int argc, char** argv) {
    // Initialize global pointers
    double_ptr_ptr = (double**)&int_ptr;
    
    // Initialize arrays
    for (int i = 0; i < 5; i++) {
        pointer_array[i] = &int_array[i];
    }
    
    // Process types based on argc
    if (argc > 1) {
        // Branch 1: Use scalar types heavily
        int sum = 0;
        sum += global_int + global_uint + global_short;
        sum += static_cast<int>(global_float + global_double);
        
        // Use string types
        const char* arg_string = argv[1];
        char buffer[100];
        
        // Use structure types
        struct SimpleStruct s = {sum, 3.14, arg_string};
        struct OuterStruct o = {1, {2.0f, 'A'}, s};
        
        // Use union
        union DataUnion u;
        u.int_value = sum;
        
        // Print something observable
        printf("Branch 1: sum = %d, string = %s\n", sum, arg_string);
    } else {
        // Branch 2: Use pointer and array types
        int* dynamic_array = (int*)malloc(sizeof(int) * 10);
        for (int i = 0; i < 10; i++) {
            dynamic_array[i] = i * i;
        }
        
        // Use function pointers
        qsort(dynamic_array, 10, sizeof(int), compare_ints);
        
        // Use callback
        struct CallbackHolder holder = {simple_callback, dynamic_array};
        if (holder.callback) {
            holder.callback(42, holder.user_data);
        }
        
        // Print something observable
        printf("Branch 2: first element = %d\n", dynamic_array[0]);
        
        free(dynamic_array);
    }
    
    // Always call process_types to ensure all type usages
    process_types(argc, argv);
    
#ifdef __cplusplus
    // Additional C++ specific execution
    if (argc > 2) {
        // Use C++ classes
        DerivedClass derived;
        derived.virtual_func();
        
        // Use template
        auto val = int_template.get_data();
        
        // Use lambda
        auto lambda_result = [val]() { return val * 2; }();
        
        std::cout << "C++ branch: " << lambda_result << std::endl;
    }
#endif
    
    // Compute and print a hash based on sizeof various types
    size_t hash = 0;
    hash ^= sizeof(global_int);
    hash ^= sizeof(global_double) << 8;
    hash ^= sizeof(struct SimpleStruct) << 16;
    hash ^= sizeof(union DataUnion) << 24;
    hash ^= (size_t)main;
    
    printf("Final hash: 0x%zx\n", hash);
    
    return 0;
}
