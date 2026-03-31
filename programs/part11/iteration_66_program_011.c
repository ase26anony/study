omp_depend_t obj;
// ... initialize or use obj with depend clauses ...

#pragma omp depobj(obj) destroy
// obj should not be used after this point
