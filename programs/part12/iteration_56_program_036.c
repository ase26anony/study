#pragma omp task depend(inout: x)      // Task A
{ /* modifies x */ }

#pragma omp task depend(update: x)     // Task B - depends on Task A
{ /* potentially updates x on device/host */ }

#pragma omp task depend(in: x)         // Task C - depends on Task B
{ /* reads x */ }
