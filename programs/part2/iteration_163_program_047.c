#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct UndefinedType1;
class UndefinedType2;
template<typename T> class UndefinedTemplate;

// TYPE_UNDEFINED: Incomplete types with pointers
UndefinedType1* global_undefined_ptr1;
UndefinedType2* global_undefined_ptr2;
UndefinedTemplate<int>* global_undefined_template_ptr;

// TYPE_SCALAR: Fundamental types in various contexts
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
bool global_bool = true;
wchar_t global_wchar = L'Ω';
char16_t global_char16 = u'字';
char32_t global_char32 = U'🍕';

// TYPE_STRING: String literals and character arrays
const char* global_cstr = "Hello, World!";
const wchar_t* global_wstr = L"Wide String";
const char16_t* global_u16str = u"UTF-16 String";
const char32_t* global_u32str = U"UTF-32 String";
char global_char_array[100] = "Character Array";
wchar_t global_wchar_array[50] = L"Wide Array";
const char* const global_const_ptr_const_str = "Const Pointer to Const String";

// TYPE_STRUCT: C structures with various compositions
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
    unsigned int : 4; // Padding
    signed int value : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[]; // Flexible array member
};

// Instantiate structs
SimpleStruct global_simple_struct = {1, 2.0, 'A'};
NestedStruct global_nested_struct = {{10, 20.0, 'B'}, 30};
BitFieldStruct global_bitfield_struct = {1, 5, 10, -50};
PackedStruct global_packed_struct = {'X', 123, 456.789};

// TYPE_USER_STRUCT: C++ classes with various features
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { cout << "BaseClass::virtual_func" << endl; }
    void non_virtual_func() { cout << "BaseClass::non_virtual_func" << endl; }
private:
    int base_value;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    virtual ~DerivedClass() override {}
    virtual void virtual_func() override { cout << "DerivedClass::virtual_func" << endl; }
    void derived_only_func() { cout << "DerivedClass::derived_only_func" << endl; }
protected:
    int derived_value;
};

class MultipleBase1 {
public:
    virtual void base1_func() = 0;
    int base1_data;
};

class MultipleBase2 {
public:
    virtual void base2_func() = 0;
    double base2_data;
};

class MultipleInheritanceClass : public MultipleBase1, public MultipleBase2 {
public:
    void base1_func() override { cout << "MultipleInheritanceClass::base1_func" << endl; }
    void base2_func() override { cout << "MultipleInheritanceClass::base2_func" << endl; }
};

// Template classes
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
    void set_data(T val) { data = val; }
private:
    T data;
};

// Instantiate template classes
TemplateClass<int> global_template_int(42);
TemplateClass<double> global_template_double(3.14);
TemplateClass<SimpleStruct> global_template_struct{{5, 10.5, 'C'}};

// TYPE_UNION: C-style unions and C++11 anonymous unions
union SimpleUnion {
    int i;
    float f;
    double d;
    char c;
};

union ComplexUnion {
    int int_val;
    float float_val;
    SimpleStruct struct_val;
    void* ptr_val;
};

struct StructWithAnonymousUnion {
    int type;
    union {
        int int_member;
        float float_member;
        char char_member;
    };
    double regular_member;
};

// Instantiate unions
SimpleUnion global_simple_union = {.i = 42};
ComplexUnion global_complex_union;
StructWithAnonymousUnion global_struct_with_union = {1, {.int_member = 100}, 3.14};

// TYPE_POINTER: Various pointer types
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
int* const global_int_const_ptr = &global_int;
const int* const global_const_int_const_ptr = &global_int;
volatile int* global_volatile_int_ptr = &global_int;
int* volatile global_int_volatile_ptr = &global_int;
int** global_int_ptr_ptr = &global_int_ptr;
int*** global_int_ptr_ptr_ptr = &global_int_ptr_ptr;
SimpleStruct* global_struct_ptr = &global_simple_struct;
const SimpleStruct* global_const_struct_ptr = &global_simple_struct;
BaseClass* global_base_class_ptr = nullptr;
TemplateClass<int>* global_template_ptr = &global_template_int;
void* global_void_ptr = nullptr;
const void* global_const_void_ptr = nullptr;
void* volatile global_volatile_void_ptr = nullptr;

// TYPE_ARRAY: Various array types
int global_int_array[10] = {0,1,2,3,4,5,6,7,8,9};
int global_2d_array[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
int global_3d_array[2][3][4];
SimpleStruct global_struct_array[5];
SimpleUnion global_union_array[3];
int* global_ptr_array[10];
const char* global_string_array[] = {"one", "two", "three", nullptr};
int (*global_array_ptr)[10] = &global_int_array;
int (*global_2d_array_ptr)[3][4] = &global_2d_array;

// Variable Length Array simulation (C++ doesn't have VLAs, but we can use alloca)
#ifdef __GNUC__
#include <alloca.h>
#endif

// TYPE_CALLBACK: Function pointers and callbacks
typedef int (*SimpleCallback)(int, double);
typedef void (*ComplexCallback)(const char*, void*);
typedef int (*ArrayCallback)(int[], size_t);
typedef void (*MemberFuncPtr)(BaseClass*);

struct StructWithCallback {
    SimpleCallback callback;
    void* user_data;
};

// Function pointer variables
SimpleCallback global_simple_callback = nullptr;
ComplexCallback global_complex_callback = nullptr;
StructWithCallback global_struct_with_callback = {nullptr, nullptr};

// Functions to be used as callbacks
int callback_func1(int x, double y) { return static_cast<int>(x * y); }
void callback_func2(const char* str, void* data) { cout << "Callback: " << str << endl; }
int callback_func3(int arr[], size_t size) { return arr[0] + size; }

// Lambda expressions as callbacks
auto global_lambda = [](int x) -> int { return x * 2; };
auto global_capturing_lambda = [global_int](int x) -> int { return x + global_int; };

// Template with callback
template<typename Func>
void template_with_callback(Func f, int value) {
    f(value);
}

// TYPE_LANG_STRUCT: Complex C++ language constructs

// Lambda with different captures
auto lambda_with_capture = [global_int, &global_double](int x) -> double {
    return global_int * x + global_double;
};

// std::initializer_list usage
auto global_initializer_list = {1, 2, 3, 4, 5};

// Structured bindings (C++17)
struct Point { int x; int y; };
auto global_point = Point{10, 20};

// Fold expressions (C++17)
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#ifdef __cpp_coroutines
struct DummyCoroutine {
    struct promise_type {
        DummyCoroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

DummyCoroutine dummy_coroutine() {
    co_return;
}
#endif

// Variadic template with fold expression
template<typename... Ts>
class VariadicTemplate {
    static constexpr size_t count = sizeof...(Ts);
};

// Alias template
template<typename T>
using AliasTemplate = TemplateClass<T>;

// Main function that uses everything
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? atoi(argv[1]) : 0;
    
    // TYPE_UNDEFINED usage
    if (mode == 0) {
        // Use undefined type pointers
        void* temp1 = static_cast<void*>(global_undefined_ptr1);
        void* temp2 = static_cast<void*>(global_undefined_ptr2);
        cout << "Undefined pointers: " << temp1 << ", " << temp2 << endl;
    }
    
    // TYPE_SCALAR usage
    int local_int = 100;
    unsigned int local_uint = 200u;
    float local_float = 1.234f;
    double local_double = 5.678;
    bool local_bool = false;
    
    // TYPE_STRING usage
    const char* local_cstr = "Local String";
    char local_char_array[] = "Local Array";
    wchar_t local_wchar_array[] = L"Local Wide Array";
    
    // TYPE_STRUCT usage
    SimpleStruct local_struct = {local_int, local_double, 'Z'};
    NestedStruct local_nested = {{local_int, local_float, 'Y'}, local_uint};
    BitFieldStruct local_bitfield = {1, 3, 7, -25};
    
    // TYPE_USER_STRUCT usage
    BaseClass base_instance;
    DerivedClass derived_instance;
    MultipleInheritanceClass multiple_instance;
    
    base_instance.virtual_func();
    derived_instance.virtual_func();
    derived_instance.derived_only_func();
    multiple_instance.base1_func();
    multiple_instance.base2_func();
    
    // Use template instances
    global_template_int.set_data(local_int);
    global_template_double.set_data(local_double);
    
    // TYPE_UNION usage
    SimpleUnion local_union;
    local_union.i = 42;
    local_union.f = 3.14f;
    
    global_struct_with_union.int_member = 999;
    
    // TYPE_POINTER usage
    int* local_int_ptr = &local_int;
    int** local_int_ptr_ptr = &local_int_ptr;
    SimpleStruct* local_struct_ptr = &local_struct;
    BaseClass* local_base_ptr = &derived_instance;
    
    // Take addresses of everything
    void* addr1 = &global_int;
    void* addr2 = &global_simple_struct;
    void* addr3 = &global_template_int;
    void* addr4 = &global_simple_union;
    
    // TYPE_ARRAY usage
    int local_array[5] = {1, 2, 3, 4, 5};
    int local_2d_array[2][3] = {{1,2,3},{4,5,6}};
    SimpleStruct local_struct_array[2] = {{1,2.0,'A'},{3,4.0,'B'}};
    
    // Access array elements
    for (int i = 0; i < 5; i++) {
        local_array[i] *= 2;
    }
    
    // Variable Length Array simulation
    #ifdef __GNUC__
    if (mode == 1) {
        size_t vla_size = 10 + mode;
        int* vla = static_cast<int*>(alloca(vla_size * sizeof(int)));
        for (size_t i = 0; i < vla_size; i++) {
            vla[i] = static_cast<int>(i * i);
        }
    }
    #endif
    
    // TYPE_CALLBACK usage
    global_simple_callback = callback_func1;
    global_complex_callback = callback_func2;
    global_struct_with_callback.callback = callback_func1;
    
    if (global_simple_callback) {
        int result = global_simple_callback(10, 2.5);
        cout << "Callback result: " << result << endl;
    }
    
    // Use lambdas as callbacks
    template_with_callback(global_lambda, 42);
    template_with_callback(global_capturing_lambda, 100);
    
    // TYPE_LANG_STRUCT usage
    // Lambda with capture
    int capture_result = lambda_with_capture(5);
    cout << "Lambda with capture result: " << capture_result << endl;
    
    // initializer_list
    for (auto val : global_initializer_list) {
        cout << val << " ";
    }
    cout << endl;
    
    // Structured binding (C++17)
    auto [x, y] = global_point;
    cout << "Structured binding: x=" << x << ", y=" << y << endl;
    
    // Fold expression
    int fold_result = sum(1, 2, 3, 4, 5);
    cout << "Fold expression result: " << fold_result << endl;
    
    // Variadic template instantiation
    VariadicTemplate<int, double, char, bool> variadic_instance;
    
    // Alias template usage
    AliasTemplate<float> alias_instance(3.14f);
    
    // Coroutine (C++20)
    #ifdef __cpp_coroutines
    if (mode == 2) {
        dummy_coroutine();
    }
    #endif
    
    // Generate some observable output based on all types
    size_t hash = 0;
    hash ^= reinterpret_cast<size_t>(&global_int);
    hash ^= reinterpret_cast<size_t>(&global_simple_struct);
    hash ^= reinterpret_cast<size_t>(&global_template_int);
    hash ^= reinterpret_cast<size_t>(&global_simple_union);
    hash ^= reinterpret_cast<size_t>(&global_int_array);
    hash ^= reinterpret_cast<size_t>(global_simple_callback);
    hash ^= sizeof(SimpleStruct);
    hash ^= sizeof(DerivedClass);
    hash ^= sizeof(SimpleUnion);
    hash ^= sizeof(global_int_array);
    
    cout << "Final hash: " << hash << endl;
    cout << "Program completed successfully!" << endl;
    
    return 0;
}

#else // C version
// C-specific code for when compiled as C

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// Forward declarations for TYPE_UNDEFINED
struct UndefinedType1;
typedef struct UndefinedType1 UndefinedType1;

// TYPE_UNDEFINED
UndefinedType1* global_undefined_ptr1 = NULL;

// TYPE_SCALAR
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
float global_float = 3.14159f;
double global_double = 2.718281828459045;

// TYPE_STRING
const char* global_cstr = "Hello, World!";
char global_char_array[100] = "Character Array";

// TYPE_STRUCT
struct SimpleStruct {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    struct SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;
    signed int value : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];
};

// Instantiate structs
struct SimpleStruct global_simple_struct = {1, 2.0, 'A'};
struct NestedStruct global_nested_struct = {{10, 20.0, 'B'}, 30};
struct BitFieldStruct global_bitfield_struct = {1, 5, 10, -50};
struct PackedStruct global_packed_struct = {'X', 123, 456.789};

// TYPE_UNION
union SimpleUnion {
    int i;
    float f;
    double d;
    char c;
};

union ComplexUnion {
    int int_val;
    float float_val;
    struct SimpleStruct struct_val;
    void* ptr_val;
};

struct StructWithAnonymousUnion {
    int type;
    union {
        int int_member;
        float float_member;
        char char_member;
    };
    double regular_member;
};

// Instantiate unions
union SimpleUnion global_simple_union = {.i = 42};
union ComplexUnion global_complex_union;
struct StructWithAnonymousUnion global_struct_with_union = {1, {.int_member = 100}, 3.14};

// TYPE_POINTER
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
int* const global_int_const_ptr = &global_int;
int** global_int_ptr_ptr = &global_int_ptr;
struct SimpleStruct* global_struct_ptr = &global_simple_struct;
void* global_void_ptr = NULL;

// TYPE_ARRAY
int global_int_array[10] = {0,1,2,3,4,5,6,7,8,9};
int global_2d_array[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
struct SimpleStruct global_struct_array[5];
int* global_ptr_array[10];
const char* global_string_array[] = {"one", "two", "three", NULL};

// TYPE_CALLBACK
typedef int (*SimpleCallback)(int, double);
typedef void (*ComplexCallback)(const char*, void*);

struct StructWithCallback {
    SimpleCallback callback;
    void* user_data;
};

// Function pointer variables
SimpleCallback global_simple_callback = NULL;
struct StructWithCallback global_struct_with_callback = {NULL, NULL};

// Callback functions
int callback_func1(int x, double y) { return (int)(x * y); }
void callback_func2(const char* str, void* data) { printf("Callback: %s\n", str); }

// Main function for C
int main(int argc, char* argv[]) {
    int mode = (argc > 1) ? atoi(argv[1]) : 0;
    
    // Use all types
    if (global_undefined_ptr1) {
        printf("Undefined pointer: %p\n", (void*)global_undefined_ptr1);
    }
    
    // Scalars
    int local_int = global_int + mode;
    float local_float = global_float * mode;
    
    // Strings
    const char* local_cstr = "Local String";
    printf("%s\n", local_cstr);
    
    // Structs
    struct SimpleStruct local_struct = {local_int, global_double, 'Z'};
    struct NestedStruct local_nested = {{local_int, local_float, 'Y'}, global_uint};
    
    // Unions
    union SimpleUnion local_union;
    local_union.i = 42;
    local_union.f = 3.14f;
    
    // Pointers
    int* local_int_ptr = &local_int;
    struct SimpleStruct* local_struct_ptr = &local_struct;
    
    // Arrays
    int local_array[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        local_array[i] *= 2;
    }
    
    // Variable Length Array (C99)
    if (mode > 0) {
        int vla_size = 5 + mode;
        int vla[vla_size];
        for (int i = 0; i < vla_size; i++) {
            vla[i] = i * i;
        }
    }
    
    // Callbacks
    global_simple_callback = callback_func1;
    if (global_simple_callback) {
        int result = global_simple_callback(10, 2.5);
        printf("Callback result: %d\n", result);
    }
    
    // Generate observable output
    size_t hash = 0;
    hash ^= (size_t)&global_int;
    hash ^= (size_t)&global_simple_struct;
    hash ^= (size_t)&global_simple_union;
    hash ^= (size_t)&global_int_array;
    hash ^= sizeof(struct SimpleStruct);
    hash ^= sizeof(union SimpleUnion);
    
    printf("Final hash: %zu\n", hash);
    printf("Program completed successfully!\n");
    
    return 0;
}
#endif
