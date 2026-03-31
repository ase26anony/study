#pragma omp task depend(in: x)      // Would print "update(in)"
#pragma omp task depend(out: y)     // Would print "update(out)"
#pragma omp task depend(inout: z)   // Would print "update(inout)"
