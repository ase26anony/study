IF (GOMP_USE_SIMT() != 0) THEN
    GOTO lab1
ELSE
    GOTO lab2

lab1:
    #pragma omp for _SIMT_
    ... (SIMT version)
    GOTO lab3

lab2:
    #pragma omp for
    ... (regular version)
    (fall through to lab3)

lab3:
    ... (continue after loop)
