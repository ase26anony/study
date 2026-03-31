// main.cpp - C++ part targeting explicit, mutable, location, segment attributes
#include <iostream>
#include <thread>

// Force location and segment attributes
__attribute__((section("mysection"))) static int section_var = 42;
thread_local int tls_var = 100;
register int reg_var asm ("r12") = 0;  // May force complex location info

// Classes for explicit and mutable attributes
class Base {
public:
    explicit Base(int x) : x(x) {}
    virtual ~Base() {}
private:
    int x;
};

class WithMutable {
public:
    WithMutable() : counter(0) {}
    void increment() const { counter++; }  // mutable can be modified in const method
private:
    mutable int counter;
};

// Template to force type elaboration
template<typename T>
class Container {
    T value;
public:
    explicit Container(T v) : value(v) {}
    T get() const { return value; }
};

// Complex inheritance
class Derived : public Base {
public:
    explicit Derived(int x, int y) : Base(x), y(y), mut(WithMutable()) {}
    
    void use_mutable() const {
        mut.increment();
    }
    
private:
    int y;
    WithMutable mut;
};

// Function with prototyped attribute (DW_AT_prototyped)
int prototyped_func(int a, int b, int c);

int prototyped_func(int a, int b, int c) {
    return a + b + c;
}

// Use inline assembly to force location lists
void use_asm() {
    int local_var = 42;
    asm volatile (
        "mov %0, %%eax\n"
        : 
        : "r" (local_var)
        : "%eax"
    );
}

// Declare Fortran function
extern "C" {
    void fortran_sub_(char* str, int* len, int strlen);
    void fortran_use_character_types_();
}

// Declare Ada function if available
extern "C" {
    void ada_test_();
}

int main() {
    // Use explicit constructor
    Base b1(42);
    Container<int> c1(100);
    Derived d1(10, 20);
    
    // Use mutable member
    d1.use_mutable();
    
    // Use variables with special attributes
    std::cout << "Section var: " << section_var << std::endl;
    std::cout << "TLS var: " << tls_var << std::endl;
    
    // Force use of prototyped function
    std::cout << "Prototyped func: " << prototyped_func(1, 2, 3) << std::endl;
    
    // Use assembly
    use_asm();
    
    // Call Fortran code
    char str[] = "Hello";
    int len = 5;
    fortran_sub_(str, &len, sizeof(str));
    
    // Call Fortran character type test
    fortran_use_character_types_();
    
    // Try to call Ada if available
    // ada_test_();
    
    // Prevent dead code elimination
    volatile int dummy = 0;
    dummy += reinterpret_cast<long>(&b1);
    dummy += reinterpret_cast<long>(&c1);
    dummy += reinterpret_cast<long>(&d1);
    
    return 0;
}
