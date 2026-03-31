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
struct undefined_struct;  // Forward declaration, never defined
typedef undefined_struct* undefined_ptr_t;

/* ========== TYPE_SCALAR ========== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 5000;
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
// String literals and character arrays
const char* global_string = "Hello, World!";
const char global_array[] = "Character array";
const wchar_t* global_wstring = L"Wide string";
const wchar_t global_warray[] = L"Wide array";
char mutable_string[] = "Mutable string";

/* ========== TYPE_STRUCT ========== */
// Basic C structures
struct SimpleStruct {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int signed_field : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

// Global struct instances
struct SimpleStruct global_struct = {10, 3.14, 'X'};
static struct NestedStruct static_nested = {{5, 2.71, 'Y'}, 100};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass(int x) : private_member(x), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() { private_member++; }
};

class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass(int x, double d) : BaseClass(x), derived_private(d) {}
    void virtual_func() override { derived_private *= 2.0; }
};

class MultipleBase1 {
public:
    virtual void base1_func() {}
};

class MultipleBase2 {
protected:
    int base2_data;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    void base1_func() override {}
};

// Template classes
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T val) : data(val) {}
    T get() const { return data; }
    void set(T val) { data = val; }
};

// Explicit template instantiations
template class TemplateClass<int>;
template class TemplateClass<double>;
template class TemplateClass<SimpleStruct>;

// Class with complex inheritance
class AbstractBase {
public:
    virtual void pure_virtual() = 0;
    virtual ~AbstractBase() {}
};

class ConcreteClass : public AbstractBase {
    int* dynamic_member;
public:
    ConcreteClass() : dynamic_member(new int(42)) {}
    ~ConcreteClass() { delete dynamic_member; }
    void pure_virtual() override {}
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union ComplexUnion {
    struct {
        int type;
    } header;
    struct {
        int type;
        double value;
    } numeric;
    struct {
        int type;
        char text[32];
    } string;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int tag;
    union {
        int int_val;
        double double_val;
        char* string_val;
    };
};
#endif

// Global union instances
union SimpleUnion global_union = {.as_int = 0x41424344};
static union ComplexUnion static_complex_union;

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr_ptr = &global_string;

// Pointers to different types
struct SimpleStruct* struct_ptr = &global_struct;
union SimpleUnion* union_ptr = &global_union;
undefined_ptr_t undefined_ptr = nullptr;

// Function pointers
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(int);

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*member_func_ptr_t)();
// Pointer to data member
typedef int BaseClass::*member_data_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int simple_array[10] = {0,1,2,3,4,5,6,7,8,9};
const char* string_array[] = {"one", "two", "three", nullptr};
double multi_dim_array[2][3][4] = {0};
struct SimpleStruct struct_array[5];

// Array of pointers
int* pointer_array[8];

// Incomplete array in struct
struct WithIncompleteArray {
    int count;
    int items[];  // Incomplete array
};

#ifdef __cplusplus
// Array references
int (&array_ref)[10] = simple_array;
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types and usage
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

// Functions to use as callbacks
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ callbacks with std::function
std::function<int(int)> cpp_callback = [](int x) { return x * x; };

// Template with callback
template<typename Func>
void execute_callback(Func f, int value) {
    f(value);
}

// Lambda expressions
auto lambda = [](int x, int y) -> int { return x + y; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda with different captures
auto lambda_with_capture = [global_int, &global_double](int x) {
    return x + global_int + static_cast<int>(global_double);
};

// std::initializer_list usage
auto init_list_func(std::initializer_list<int> list) {
    int sum = 0;
    for (auto x : list) sum += x;
    return sum;
}

// Structured bindings (C++17)
struct Point { double x, y; };
auto get_point() -> Point { return {1.0, 2.0}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

// Coroutine-related types (C++20)
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

DummyCoroutine dummy_coro() {
    co_return;
}
#endif

// Complex template instantiation
template<typename T, size_t N>
class FixedArray {
    T data[N];
public:
    T& operator[](size_t idx) { return data[idx]; }
    const T& operator[](size_t idx) const { return data[idx]; }
    
    auto begin() { return &data[0]; }
    auto end() { return &data[N]; }
};

// Instantiate complex templates
FixedArray<int, 10> fixed_array;
FixedArray<SimpleStruct, 5> struct_fixed_array;
#endif

/* ========== Helper Functions ========== */
int process_scalars(int argc) {
    int local_int = argc * 10;
    unsigned local_uint = argc * 20u;
    float local_float = argc * 3.14f;
    double local_double = argc * 2.71828;
    
    return local_int + static_cast<int>(local_uint) + 
           static_cast<int>(local_float) + static_cast<int>(local_double);
}

void process_pointers(int argc) {
    int local = argc;
    int* local_ptr = &local;
    int** local_double_ptr = &local_ptr;
    
    // Use all global pointers
    if (int_ptr) *int_ptr = argc;
    if (struct_ptr) struct_ptr->x = argc;
    if (union_ptr) union_ptr->as_int = argc;
}

void process_arrays(int argc) {
    // Use arrays
    for (int i = 0; i < 10 && i < argc; i++) {
        simple_array[i] = i * argc;
    }
    
    // Multi-dimensional array
    if (argc > 0) {
        multi_dim_array[0][0][0] = argc * 1.0;
    }
}

#ifdef __cplusplus
void process_cpp_types(int argc) {
    // Instantiate C++ classes
    DerivedClass derived(argc, 3.14);
    derived.virtual_func();
    
    TemplateClass<int> tc_int(argc);
    TemplateClass<double> tc_double(argc * 1.5);
    
    ConcreteClass concrete;
    concrete.pure_virtual();
    
    // Use lambdas
    auto result = lambda(argc, argc * 2);
    
    // Use std::function
    int callback_result = cpp_callback(argc);
    
    // Use structured bindings
    auto [x, y] = get_point();
    
    // Use fold expression
    int fold_result = sum_all(argc, argc+1, argc+2);
    
    // Use initializer_list
    int list_sum = init_list_func({argc, argc+1, argc+2});
    
    // Use complex template
    fixed_array[0] = argc;
    struct_fixed_array[0].x = argc;
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Prevent dead code elimination
    if (argc < 2) {
        // Branch 1: Process scalars and basic types
        int scalar_result = process_scalars(argc);
        
        // Use undefined pointer (TYPE_UNDEFINED)
        undefined_ptr_t local_undefined_ptr = undefined_ptr;
        (void)local_undefined_ptr;
        
        // Use strings
        const char* local_string = argc ? argv[0] : "default";
        wchar_t wide_local[] = L"main";
        
        // Use structs
        struct SimpleStruct local_struct = {argc, argc * 1.0, 'A' + argc};
        struct NestedStruct local_nested = {local_struct, argc * 2};
        
        // Use bitfield struct
        struct BitFieldStruct bitfield = {1, 3, 7, -10};
        
        // Use packed struct
        struct PackedStruct packed = {'X', argc, argc * 3.14};
        
        // Use unions
        union SimpleUnion local_union;
        local_union.as_int = argc;
        
#ifdef __cplusplus
        // Use anonymous union
        StructWithAnonymousUnion anon_union;
        anon_union.tag = 1;
        anon_union.int_val = argc;
#endif
        
        // Output something observable
        printf("Branch 1: %d %c %s\n", scalar_result, local_struct.z, local_string);
    } else {
        // Branch 2: Process pointers, arrays, and callbacks
        process_pointers(argc);
        process_arrays(argc);
        
        // Use function pointers (TYPE_CALLBACK)
        comparator_t comp = compare_ints;
        int a = 5, b = 10;
        comp(&a, &b);
        
        struct CallbackContainer cb_container = {sample_callback, &a};
        if (cb_container.callback) {
            cb_container.callback(argc, cb_container.user_data);
        }
        
        // Use all array types
        for (int i = 0; i < argc && i < 10; i++) {
            pointer_array[i] = &simple_array[i];
        }
        
#ifdef __cplusplus
        // Process C++ types
        process_cpp_types(argc);
        
        // Execute template callback
        execute_callback([](int x) { return x * 3; }, argc);
        
        // Use lambda with capture
        int lambda_result = lambda_with_capture(argc);
        
#if __cplusplus >= 202002L
        // Use coroutine if available
        dummy_coro();
#endif
#endif
        
        // Output something observable
        printf("Branch 2: %d %p\n", argc, (void*)pointer_array[0]);
    }
    
    // Additional type usages to ensure all are processed
    // Take addresses of everything
    void* addresses[] = {
        &global_int,
        &global_struct,
        &global_union,
        &global_string,
        &int_ptr,
        &simple_array,
        &compare_ints,
#ifdef __cplusplus
        &cpp_callback,
        &fixed_array,
#endif
        nullptr
    };
    
    // Compute a simple hash from addresses and sizes
    size_t hash = 0;
    for (int i = 0; i < (int)(sizeof(addresses)/sizeof(addresses[0])) - 1; i++) {
        hash = hash * 31 + (size_t)addresses[i];
    }
    
    // Add sizes of various types
    hash += sizeof(struct SimpleStruct);
    hash += sizeof(union SimpleUnion);
    hash += sizeof(int*);
    hash += sizeof(double*);
    hash += sizeof(comparator_t);
#ifdef __cplusplus
    hash += sizeof(DerivedClass);
    hash += sizeof(TemplateClass<int>);
#endif
    
    printf("Final hash: %zu\n", hash % 1000000);
    
    return (int)(hash % 256);
}
