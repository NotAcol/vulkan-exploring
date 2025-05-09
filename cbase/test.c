#include "base_include.h"
#include "base_include.c"
#include <stdio.h>

int main(void) {
    m3_f32 A = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    m3_f32 B = {1, 0, 0, 0, 1, 0, 0, 0, 1};

    A = MulM3F32(A, B);
    printf("%f %f %f %f %f %f %f %f %f\n", A.A11, A.A12, A.A13, A.A21, A.A22, A.A23, A.A31, A.A32, A.A33);

    return 0;
}
