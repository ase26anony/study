// main.cpp - C++ part targeting explicit, mutable, location, segment attributes
#include <iostream>
#include <thread>
#include <atomic>

// Variables in custom sections for DW_AT_segment
__attribute__((section(".mysection"))) int custom_section_var = 42;
__attribute__((section(".anothersection"))) volatile int another_section_var = 100;

// Thread-local for location lists
thread_local int tls_var = 123;

// Class with explicit constructor for DW_AT_explicit
class ExplicitClass {
public:
    explicit ExplicitClass(int x) : value(x) {}
    int getValue() const { return value; }
private:
    int value;
};

// Class with mutable member for DW_AT_mutable
class MutableClass {
public:
    MutableClass() : counter(0) {}
    void increment() const { counter++; }  // Can modify mutable member
    int getCounter() const { return counter; }
private:
    mutable int counter;  // DW_AT_mutable
};

// Template class to force full debug info
template<typename T>
class Container {
public:
    explicit Container(const T& val) : data(val) {}
    T get() const { return data; }
private:
    T data;
};

// Function with prototyped attribute
void prototyped_function(int x, double y, const char* z);

// Function with register variable (hints at location)
void use_register_var() {
    register int reg_var asm("ebx") = 42;
    asm volatile("" : : "r"(reg_var));  // Force use in assembly
}

// Complex inheritance for detailed type info
class Base {
public:
    virtual ~Base() = default;
    virtual void foo() = 0;
};

class Derived : public Base {
public:
    explicit Derived(int val) : value(val) {}
    void foo() override { std::cout << "Derived: " << value << std::endl; }
private:
    int value;
};

// Use inline assembly to force location info
void asm_location_test() {
    int local_var = 999;
    asm volatile(
        "movl %0, %%eax\n\t"
        : : "m"(local_var) : "%eax"
    );
}

// Fortran declarations
extern "C" {
    void fortran_subroutine_(char* str, int* len, int str_len);
    void fortran_character_test_();
    int fortran_function_(int* x);
}

// Ada declarations (if available)
#ifdef __GNUC__
extern "C" {
    void ada_test_procedure();
    int ada_test_function();
}
#endif

int main() {
    // Use explicit constructor
    ExplicitClass explicit_obj(42);
    std::cout << "ExplicitClass value: " << explicit_obj.getValue() << std::endl;
    
    // Use mutable member
    MutableClass mutable_obj;
    mutable_obj.increment();
    std::cout << "MutableClass counter: " << mutable_obj.getCounter() << std::endl;
    
    // Use template with explicit constructor
    Container<int> container(100);
    std::cout << "Container value: " << container.get() << std::endl;
    
    // Use thread-local variable
    tls_var = 456;
    std::cout << "TLS variable: " << tls_var << std::endl;
    
    // Use custom section variables
    custom_section_var = 999;
    another_section_var = 888;
    std::cout << "Custom section vars: " << custom_section_var << ", " 
              << another_section_var << std::endl;
    
    // Use register variable function
    use_register_var();
    
    // Use assembly location test
    asm_location_test();
    
    // Create derived class
    Derived derived(123);
    derived.foo();
    
    // Call Fortran code
    char fortran_str[] = "Hello Fortran";
    int str_len = sizeof(fortran_str) - 1;
    fortran_subroutine_(fortran_str, &str_len, str_len);
    
    // Call Fortran character test
    fortran_character_test_();
    
    int x = 42;
    int result = fortran_function_(&x);
    std::cout << "Fortran function result: " << result << std::endl;
    
    // Try to call Ada if available
    #ifdef __GNUC__
    ada_test_procedure();
    int ada_result = ada_test_function();
    std::cout << "Ada function result: " << ada_result << std::endl;
    #endif
    
    // Prevent dead code elimination
    volatile int keep_alive = 0;
    keep_alive += explicit_obj.getValue();
    keep_alive += mutable_obj.getCounter();
    keep_alive += container.get();
    
    return keep_alive > 0 ? 0 : 1;
}

// Implement prototyped function
void prototyped_function(int x, double y, const char* z) {
    std::cout << "Prototyped: " << x << ", " << y << ", " << z << std::endl;
}
