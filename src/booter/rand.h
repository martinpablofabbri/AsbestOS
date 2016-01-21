#ifndef RAND_H
#define RAND_H

/**
 * Simple implementation of a PRNG.
 * We will be using the Mersenne Twister
 * See https://en.wikipedia.org/wiki/Mersenne_Twister
 */

void seed (unsigned seed);

unsigned rand ();

#endif
