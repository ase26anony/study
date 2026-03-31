#pragma omp task depend(in: x)      // Would print: depend(update(in): x)
#pragma omp task depend(out: y)     // Would print: depend(update(out): y)
#pragma omp task depend(destroy: z) // Would print: depend(destroy: z)
