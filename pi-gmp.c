/**
 * Calculate approximations of pi using different methods and gmp
 * arbitrary precision floating point calculations.
 *
 * Usage: pi method mantissa_bits num_iterations
 *
 * Depending on the method, n will be number of iterations or the number
 * of grid points.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <gmp.h>


/**
 * Compute the partial sum with nterms terms of the power series for
 *      atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
 * Note that this converges for |x| < 1 and x = 1. Result is stored in y.
 */
void atan_pseries(mpf_t y, mpf_t x, long int nterms) {
    mpf_t xpower, x2, denom, tmp;

    mpf_init(xpower);
    mpf_init(denom);
    mpf_init(x2);
    mpf_init(tmp);

    bool is_minus = true;
    mpf_set(y, x);
    mpf_pow_ui(xpower, x, 3);
    mpf_pow_ui(x2, x, 2);
    mpf_set_ui(denom, 3);

    for (long int i=1; i<nterms; i++) {
        // y += plus_minus * xpower / denom;
        mpf_div(tmp, xpower, denom);
        if (is_minus)
            mpf_neg(tmp, tmp);
        mpf_add(y, y, tmp);

        mpf_add_ui(denom, denom, 2);
        mpf_mul(xpower, xpower, x2);
        is_minus = !is_minus;
    }

    mpf_clear(xpower);
    mpf_clear(denom);
    mpf_clear(x2);
    mpf_clear(tmp);
}


/**
 * Compute pi using Machin's formula:
 *   pi/4 = 4 arctan(1/5) - arctan(1/239)
 * and partial sums of the Taylor series for arctan.
 */
void pi_atan_pseries(mpf_t pi, long int nterms) {
    mpf_t a, b, tmp;

    mpf_init(tmp);
    // a = 1 / 5
    mpf_init_set_ui(a, 1);
    mpf_div_ui(a, a, 5);
    // b = 1 / 239
    mpf_init_set_ui(b, 1);
    mpf_div_ui(b, b, 239);

    // 16 * atan_pseries(1.0L/5, nterms)
    atan_pseries(tmp, a, nterms);
    mpf_mul_ui(tmp, tmp, 16);
    mpf_set(pi, tmp);

    // - 4 * atan_pseries(1.0L/239, nterms));
    atan_pseries(tmp, b, nterms);
    mpf_mul_ui(tmp, tmp, 4);
    mpf_sub(pi, pi, tmp);

    mpf_clear(a);
    mpf_clear(b);
    mpf_clear(tmp);
}


void print_usage(FILE *out, char *prog_name) {
    fprintf(out, "Usage: %s method iterations\n", prog_name);
}


int main(int argc, char **argv) {
    if (argc != 4) {
        print_usage(stderr, argv[0]);
        exit(1);
    }


    char *method = argv[1];
    long int prec_bits = atol(argv[2]);
    long int niters = atol(argv[3]);

    mpf_set_default_prec(prec_bits);

    mpf_t pi_approx;
    mpf_init_set_ui(pi_approx, 0);

    if (strcmp(method, "mc") == 0) {
        //pi_approx = pi_monte_carlo_integration(niters);
    } else if (strcmp(method, "trap") == 0) {
        //pi_approx = pi_trap_integration(niters);
    } else if (strcmp(method, "atan") == 0) {
        pi_atan_pseries(pi_approx, niters);
    } else {
        print_usage(stderr, argv[0]);
        exit(2);
    }
    gmp_printf("%.Ff\n", pi_approx);

    mpf_clear(pi_approx);
}
