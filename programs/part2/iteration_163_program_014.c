/* gengtype_test.c - Test program to trigger all gengtype type categories */

#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <vector>
using namespace std;

// For TYPE_LANG_STRUCT: C++17 fold expression
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // Fold expression
}

// For TYPE_LANG_STRUCT: C++17 structured bindings
struct Point3D {
    float x, y, z;
};

// For TYPE_LANG_STRUCT: Coroutine (C++20)
#ifdef __cpp_coroutines
struct ReturnObject {
    struct promise_type {
        ReturnObject get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

ReturnObject test_coroutine() {
    co_return;
}
#endif

// For TYPE_LANG_STRUCT: Lambda with different captures
auto create_lambdas() {
    int x = 10;
    static int y = 20;
    
    auto lambda1 = []() { return 1; };  // No capture
    auto lambda2 = [x]() { return x; }; // By value
    auto lambda3 = [&x]() { return x; }; // By reference
    auto lambda4 = [=]() { return x + y; }; // Default by value
    auto lambda5 = [&]() { return x + y; }; // Default by reference
    
    return lambda1;
}

// For TYPE_LANG_STRUCT: std::initializer_list
void use_initializer_list(std::initializer_list<int> list) {
    for (auto& item : list) {
        // Process items
    }
}

// For TYPE_USER_STRUCT: Template classes
template<typename T>
class TemplateClass {
private:
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get() const { return data; }
    virtual ~TemplateClass() {}
};

// For TYPE_USER_STRUCT: Complex class hierarchy
class Base {
protected:
    int base_data;
public:
    Base(int d) : base_data(d) {}
    virtual void print() = 0;
    virtual ~Base() {}
};

class Derived1 : public Base {
private:
    float derived_data;
public:
    Derived1(int b, float d) : Base(b), derived_data(d) {}
    void print() override {
        // Implementation
    }
};

class Derived2 : public Base {
protected:
    char* str_data;
public:
    Derived2(int b, char* s) : Base(b), str_data(s) {}
    void print() override {
        // Implementation
    }
};

class MultipleInheritance : public Derived1, public Derived2 {
public:
    MultipleInheritance(int b1, float d1, int b2, char* s) 
        : Derived1(b1, d1), Derived2(b2, s) {}
    void print() override {
        Derived1::print();
        Derived2::print();
    }
};

#endif

/* For TYPE_UNDEFINED: Forward declaration never defined */
struct undefined_struct;
typedef struct undefined_struct* undefined_ptr_t;

/* For TYPE_SCALAR: Fundamental types in various contexts */
int global_int = 42;
static unsigned int static_uint = 100;
char global_char = 'A';
float global_float = 3.14f;
double global_double = 2.71828;
#ifdef __cplusplus
bool global_bool = true;
#endif
long global_long = 1000L;
unsigned long global_ulong = 2000UL;
short global_short = 10;
unsigned short global_ushort = 20;
long long global_llong = 10000LL;
unsigned long long global_ullong = 20000ULL;

/* For TYPE_STRING: String literals and character arrays */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char char_array[50] = "Character Array";
const char* string_array[] = {"One", "Two", "Three"};
wchar_t wchar_array[] = L"Wide Array";

/* For TYPE_STRUCT: Various C structures */
struct SimpleStruct {
    int a;
    char b;
    float c;
};

struct NestedStruct {
    SimpleStruct inner;
    double extra;
};

struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

/* For TYPE_UNION: Various unions */
union SimpleUnion {
    int as_int;
    float as_float;
    char* as_ptr;
};

union ComplexUnion {
    struct {
        int type;
    } header;
    struct {
        int type;
        int value;
    } integer;
    struct {
        int type;
        float value;
    } floating;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    };
};
#endif

/* For TYPE_POINTER: Pointers to everything */
int* int_ptr = &global_int;
float* float_ptr = &global_float;
char** string_ptr_ptr = (char**)&global_string;
const int* const_ptr = &global_int;
volatile int* volatile_ptr = &global_int;
int* restrict restrict_ptr = &global_int;
undefined_ptr_t undefined_ptr = NULL;
SimpleStruct* struct_ptr = NULL;
SimpleUnion* union_ptr = NULL;

/* For TYPE_ARRAY: Various arrays */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_2d_array[3][4];
double double_3d_array[2][3][4];
char* pointer_array[5];
SimpleStruct struct_array[5];
SimpleUnion union_array[3];

/* For TYPE_CALLBACK: Function pointers */
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidFunc)(void);
typedef char* (*StringFunc)(const char*);

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void do_nothing(void) {}
char* duplicate_string(const char* str) {
    static char buffer[256];
    // Simple duplication (not thread-safe)
    int i = 0;
    while (str[i] && i < 255) {
        buffer[i] = str[i];
        i++;
    }
    buffer[i] = '\0';
    return buffer;
}

struct CallbackContainer {
    BinaryFunc func;
    VoidFunc cleanup;
};

/* Global instances */
SimpleStruct global_struct = {1, 'X', 3.14f};
SimpleUnion global_union;
NestedStruct global_nested = {{2, 'Y', 2.718f}, 42.0};
BitfieldStruct global_bitfield = {1, 3, 5, 100};
PackedStruct global_packed = {'Z', 123, 456};

#ifdef __cplusplus
Derived1 global_derived1(10, 20.5f);
Derived2 global_derived2(20, (char*)"Test");
TemplateClass<int> global_template(42);
TemplateClass<double> global_template_double(3.14);
#endif

/* Function using all types */
void use_all_types(int argc, char** argv) {
    /* TYPE_SCALAR: Local scalars */
    int local_int = argc;
    unsigned int local_uint = argc * 2;
    char local_char = 'B';
    float local_float = 1.234f;
    double local_double = 9.876;
    #ifdef __cplusplus
    bool local_bool = false;
    #endif
    
    /* TYPE_UNDEFINED: Use undefined type pointer */
    undefined_ptr_t local_undefined_ptr = NULL;
    if (argc > 1) {
        // Force analysis of undefined pointer
        local_undefined_ptr = (undefined_ptr_t)argv[0];
    }
    
    /* TYPE_STRING: Local strings */
    const char* local_string = "Local String";
    char local_array[] = "Local Array";
    wchar_t local_wstring[] = L"Local Wide";
    
    /* TYPE_STRUCT: Local structs */
    SimpleStruct local_struct = {argc, 'C', 4.56f};
    NestedStruct local_nested = {{argc + 1, 'D', 5.67f}, 3.14159};
    
    /* TYPE_UNION: Local unions */
    SimpleUnion local_union;
    local_union.as_int = argc;
    
    #ifdef __cplusplus
    StructWithAnonymousUnion local_anon_union;
    local_anon_union.tag = 1;
    local_anon_union.int_val = argc;
    #endif
    
    /* TYPE_ARRAY: Local arrays */
    int local_int_array[5] = {argc, argc+1, argc+2, argc+3, argc+4};
    SimpleStruct local_struct_array[2] = {{1, 'A', 1.1f}, {2, 'B', 2.2f}};
    
    /* TYPE_POINTER: Local pointers */
    int* local_int_ptr = &local_int;
    SimpleStruct* local_struct_ptr = &local_struct;
    int** local_double_ptr = &local_int_ptr;
    
    /* TYPE_CALLBACK: Function pointer usage */
    BinaryFunc local_func_ptr = (argc % 2 == 0) ? add : multiply;
    int result = local_func_ptr(argc, argc + 1);
    
    CallbackContainer local_callbacks = {add, do_nothing};
    local_callbacks.func(argc, argc);
    
    #ifdef __cplusplus
    /* TYPE_USER_STRUCT: Local C++ objects */
    Derived1 local_derived(argc, argc * 1.5f);
    TemplateClass<float> local_template(3.14f);
    
    /* TYPE_LANG_STRUCT: C++ specific constructs */
    // Lambda usage
    auto lambda = [&local_int, local_float]() -> float {
        return local_int + local_float;
    };
    float lambda_result = lambda();
    
    // Structured bindings
    Point3D point = {1.0f, 2.0f, 3.0f};
    auto [x, y, z] = point;
    
    // Fold expression
    int fold_result = sum(1, 2, 3, 4, 5);
    
    // Initializer list
    use_initializer_list({1, 2, 3, 4, 5});
    
    // Coroutine if available
    #ifdef __cpp_coroutines
    test_coroutine();
    #endif
    
    // Function template with lambda
    std::function<int(int)> func_obj = [](int x) { return x * 2; };
    #endif
    
    /* Prevent dead code elimination */
    volatile int dummy = 0;
    dummy += global_int + local_int + result;
    
    #ifdef __cplusplus
    dummy += lambda_result + fold_result + x + y + z;
    #endif
    
    /* Output to ensure execution */
    #ifdef __cplusplus
    std::cout << "Types test executed. argc=" << argc 
              << ", dummy=" << dummy << std::endl;
    #else
    printf("Types test executed. argc=%d, dummy=%d\n", argc, dummy);
    #endif
}

/* Main function with branching based on argc */
int main(int argc, char** argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * argc;
    }
    
    for (int i = 0; i < 5; i++) {
        pointer_array[i] = argv[0];
    }
    
    /* Branch based on argc to use different types */
    if (argc == 1) {
        /* Use scalar types */
        global_int = argc;
        global_float = argc * 1.5f;
        global_union.as_int = argc;
    } 
    else if (argc == 2) {
        /* Use struct and union types */
        global_struct.a = argc;
        global_nested.inner.a = argc;
        global_bitfield.flag1 = argc & 1;
    }
    else if (argc == 3) {
        /* Use pointer and array types */
        int_ptr = &argc;
        float_ptr = &global_float;
        int_array[0] = argc;
    }
    else if (argc >= 4) {
        /* Use callback and string types */
        BinaryFunc func = (argc % 2 == 0) ? add : multiply;
        int result = func(argc, argc + 1);
        char* dup = duplicate_string(argv[0]);
        
        #ifdef __cplusplus
        /* Use C++ specific types */
        global_derived1.print();
        global_template.get();
        
        /* TYPE_LANG_STRUCT features */
        create_lambdas();
        Point3D p = {1, 2, 3};
        auto [px, py, pz] = p;
        #endif
    }
    
    /* Always use all types */
    use_all_types(argc, argv);
    
    /* Take addresses to force pointer creation */
    void* addresses[] = {
        &global_int,
        &global_struct,
        &global_union,
        &int_array,
        &add,
        &global_string,
        #ifdef __cplusplus
        &global_derived1,
        &global_template,
        #endif
        &global_bitfield,
        &global_packed
    };
    
    /* Calculate a simple hash from addresses */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        hash = hash * 31 + (unsigned long)addresses[i];
    }
    
    #ifdef __cplusplus
    std::cout << "Address hash: " << hash << std::endl;
    #else
    printf("Address hash: %lu\n", hash);
    #endif
    
    return (int)(hash % 256);
}
