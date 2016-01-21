#define N 624
#define R 31
#define M 397
#define W 32
#define A 0x9908B0DF
#define U 11
#define D 0xFFFFFFFF
#define S 7
#define B 0x9D2C5680
#define T 15
#define C 0xEFC60000
#define L 18
#define F 1812433253

unsigned MT [N];
unsigned idx = N + 1;
const unsigned lower_mask =  0x80000000;
const unsigned upper_mask = ~0x80000000;

void seed (unsigned seed) {
  idx = N;
  MT[0] = seed;
  int i;
  for (i=1; i < N; i++) {
    MT[i] = F * (MT[i-1] ^ (MT[i-1] >> (W-2))) + i;
  }
}

void twist () {
  int i;
  for (i=0; i < N - 1; i++) {
    unsigned x = (MT[i] & upper_mask) + (MT[(i+1)%N] & lower_mask);
    unsigned xA = x >> 1;
    if (x & 1) {
      xA ^= A;
    }
    MT[i] = MT[(i + M) % N] ^ xA;
  }
  idx = 0;
}

unsigned rand () {
  if (idx >= N) {
    if (idx > N) {
      seed(5489);
    }
    twist();
  }

  unsigned y = MT[idx];
  y ^= ((y >> U) & D);
  y ^= ((y << S) & B);
  y ^= ((y << T) & C);
  y ^= (y >> L);

  idx++;
  return y;
}
