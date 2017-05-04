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
 *
 * Implements special case where x = 1/b, which is the common form of
 * terms used for Machin-like pi formula. Uses str for b to allow
 * higher precision.
 */
void atan_pseries(mpf_t y, char *b_str, unsigned long nterms) {
    mpf_t b, term, minus_b2, denom, tmp;

    mpf_init_set_str(b, b_str, 10);
    mpf_init(term);
    mpf_init(denom);
    mpf_init(minus_b2);
    mpf_init(tmp);

    // y = x = 1/b
    mpf_set_ui(y, 1);
    mpf_div(y, y, b);

    // term = x = 1/b
    mpf_set(term, y);

    mpf_pow_ui(minus_b2, b, 2);
    mpf_neg(minus_b2, minus_b2);
    mpf_set_ui(denom, 1);

    for (long int i=1; i<nterms; i++) {
        // denom += 2, i.e. denom = (2i + 1)
        mpf_add_ui(denom, denom, 2);
        // term /= -b^2
        mpf_div(term, term, minus_b2);
        mpf_div(tmp, term, denom);
        mpf_add(y, y, tmp);
    }

    mpf_clear(b);
    mpf_clear(term);
    mpf_clear(minus_b2);
    mpf_clear(denom);
    mpf_clear(tmp);
}


/**
 * Compute pi using Machin's formula:
 *   pi/4 = 4 arctan(1/5) - arctan(1/239)
 * and partial sums of the Taylor series for arctan.
 */
void pi_atan_pseries(mpf_t pi, unsigned long nterms) {
    mpf_t tmp;
    mpf_init(tmp);

    // 4 * atan(1/5)
    atan_pseries(pi, "5", nterms);
    mpf_mul_ui(pi, pi, 4);

    // - atan(1/239)
    atan_pseries(tmp, "239", nterms);
    mpf_sub(pi, pi, tmp);

    mpf_mul_ui(pi, pi, 4);

    mpf_clear(tmp);
}


/**
 * Compute pi using Machin-like formula:
 *   pi/4 = 183 arctan(1/239) + 32 arctan(1/1023) - 68 arctan (1/5832)
 *          + 12 arctan(1/110443) - 12 arctan(1/4841182)
 *          - 100 arctan(1/6826318)
 * and partial sums of the Taylor series for arctan.
 *
 * NB: this converges faster than atan above, but takes longer per iteration.
 */
void pi_atan2_pseries(mpf_t pi, unsigned long nterms) {
    mpf_t tmp;
    mpf_init(tmp);

    atan_pseries(pi, "239", nterms);
    mpf_mul_ui(pi, pi, 183);

    atan_pseries(tmp, "1023", nterms);
    mpf_mul_ui(tmp, tmp, 32);
    mpf_add(pi, pi, tmp);

    atan_pseries(tmp, "5832", nterms);
    mpf_mul_ui(tmp, tmp, 68);
    mpf_sub(pi, pi, tmp);

    atan_pseries(tmp, "110443", nterms);
    mpf_mul_ui(tmp, tmp, 12);
    mpf_add(pi, pi, tmp);

    atan_pseries(tmp, "4841182", nterms);
    mpf_mul_ui(tmp, tmp, 12);
    mpf_sub(pi, pi, tmp);

    atan_pseries(tmp, "6826318", nterms);
    mpf_mul_ui(tmp, tmp, 100);
    mpf_sub(pi, pi, tmp);

    mpf_mul_ui(pi, pi, 4);

    mpf_clear(tmp);
}


/**
 * Compute pi using simple Monte Carlo integration of a circle with
 * radius 1.
 */
void pi_monte_carlo_integration(mpf_t pi, unsigned long int ntrials) {
    gmp_randstate_t rstate;
    mpf_t x, y;
    mpf_t count;

    mpf_init(x);
    mpf_init(y);
    mpf_init_set_ui(count, 0);

    gmp_randinit_mt(rstate);
    gmp_randseed_ui(rstate, 2895720909174927L);

    for (unsigned long i=0; i<ntrials; i++) {
        // generate random numbers between -1 and 1, and square them
        mpf_urandomb(x, rstate, mpf_get_default_prec());
        mpf_mul_ui(x, x, 2);
        mpf_sub_ui(x, x, 1);
        mpf_mul(x, x, x);

        mpf_urandomb(y, rstate, mpf_get_default_prec());
        mpf_mul_ui(y, y, 2);
        mpf_sub_ui(y, y, 1);
        mpf_mul(y, y, y);

        // now x = squared distance from origin
        mpf_add(x, x, y);

        if (mpf_cmp_ui(x, 1uL) < 0) {
            mpf_add_ui(count, count, 1);
        }
    }
    mpf_div_ui(count, count, ntrials);
    mpf_mul_ui(pi, count, 4);
}


/**
 * Compute pi using trapezoid rule integration of a quarter circle with
 * radius 1 centered at the origin, i.e. y = sqrt(1-x^2) on [0,1].
 */
void pi_trap_integration(mpf_t pi, unsigned long n) {
    gmp_randstate_t rstate;
    mpf_t delta, x, y;

    gmp_randinit_mt(rstate);
    gmp_randseed_ui(rstate, 2895720909174927L);

    mpf_init_set_ui(delta, 1);
    mpf_div_ui(delta, delta, n);
    mpf_set_ui(pi, 0);
    mpf_init_set(x, delta);
    mpf_init(y);

    // sum internal values, weight is 1
    for (int i=1; i<n; i++) {
        // y = sqrt(1 - x^2)
        mpf_mul(y, x, x);
        mpf_neg(y, y);
        mpf_add_ui(y, y, 1);
        mpf_sqrt(y, y);

        // pi += sqrt(1 - x^2)
        mpf_add(pi, pi, y);

        mpf_add(x, x, delta);
    }
    // add weighted sum of values from boundaries at x=0 (1/2 * 1)
    // and x=1 (1/2 * 0)
    // pi += 0.5
    mpf_set_d(y, 0.5);
    mpf_add(pi, pi, y);

    // pi = pi * delta * 4
    mpf_mul(pi, pi, delta);
    mpf_mul_ui(pi, pi, 4);
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
        pi_monte_carlo_integration(pi_approx, niters);
    } else if (strcmp(method, "trap") == 0) {
        pi_trap_integration(pi_approx, niters);
    } else if (strcmp(method, "atan") == 0) {
        pi_atan_pseries(pi_approx, niters);
    } else if (strcmp(method, "atan2") == 0) {
        pi_atan2_pseries(pi_approx, niters);
    } else {
        print_usage(stderr, argv[0]);
        exit(2);
    }
    gmp_printf("%.Ff\n", pi_approx);

    mpf_clear(pi_approx);
}
