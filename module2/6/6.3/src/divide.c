#include "Operation.h"
#include <math.h>

const char *calc_name = "деление";
const char calc_symbol = '/';

double calc_func(double a, double b) {
    if (b == 0.0) {
        return NAN;
    }
    return a / b;
}