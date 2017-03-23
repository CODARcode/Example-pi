/**
 * Calculate approximations of pi using different methods.
 *
 * Usage: pi method n
 *
 * Depending on the method, n will be number of iterations or the number
 * of grid points.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/**
 * Compute the partial sum with nterms terms of the power series for
 *      atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
 * Note that this converges for |x| < 1 and x = 1.
 */
long double atan_pseries(long double x, long int nterms) {
    assert(x <= 1 || x > -1);
    long double total = x;
    int plus_minus = -1;
    long double xpower = powl(x, 3);
    long double denom = 3;
    long double x2 = x*x;
    for (long int i=1; i<nterms; i++) {
        total += plus_minus * xpower / denom;
        denom += 2;
        xpower *= x2;
        plus_minus *= -1;
    }
    return total;
}


/**
 * Compute pi using simple Monte Carlo integration of a circle with
 * radius 1.
 */
long double pi_monte_carlo_integration(long int ntrials) {
    double x, y;
    long double count = 0;
    for (int i=0; i<ntrials; i++) {
        // generate random numbers between -1 and 1
        x = drand48() * 2 - 1;
        y = drand48() * 2 - 1;
        if ((x*x) + (y*y) < 1) {
            ++count;
        }
    }
    return 4 * (count / ntrials);
}


/**
 * Compute pi using trapezoid rule integration of a quarter circle with
 * radius 1 centered at the origin, i.e. y = sqrt(1-x^2) on [0,1].
 */
long double pi_trap_integration(long double n) {
    long double delta = 1.0L / n;
    long double x, y;
    long double total = 0;

    // sum internal values, weight is 1
    for (x=delta; x<1; x+=delta) {
        total += sqrt(1 - x*x);
    }
    // add weighted sum of values from boundaries at x=0 (1/2 * 1)
    // and x=1 (1/2 * 0)
    total += 0.5;
    return 4 * delta * total;
}


long double pi_atan_pseries(long int nterms) {
    return (16 * atan_pseries(1.0L/5, nterms)
           - 4 * atan_pseries(1.0L/239, nterms));
}


void print_usage(FILE *out, char *prog_name) {
    fprintf(out, "Usage: %s method iterations\n", prog_name);
}


int main(int argc, char **argv) {
    if (argc != 3) {
        print_usage(stderr, argv[0]);
        exit(1);
    }

    char *method = argv[1];
    long int niters = atol(argv[2]);

    // Typically 8 bytes / 128 bits on gcc x86_64
    //printf("sizeof(long double) = %zd\n", sizeof(long double));

    // make random methods deterministic
    srand48(2895720909174927L);

    long double pi_approx;
    if (strcmp(method, "mc") == 0) {
        pi_approx = pi_monte_carlo_integration(niters);
    } else if (strcmp(method, "trap") == 0) {
        pi_approx = pi_trap_integration(niters);
    } else if (strcmp(method, "atan") == 0) {
        pi_approx = pi_atan_pseries(niters);
    } else {
        print_usage(stderr, argv[0]);
        exit(2);
    }
    printf("%.36Lf\n", pi_approx);
}
