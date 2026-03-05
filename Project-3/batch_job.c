#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Busy-wait CPU burn for about N seconds (no sleep/usleep).
int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <seconds>\n", argv[0]);
    return 1;
  }

  int seconds = atoi(argv[1]);
  if (seconds <= 0) seconds = 1;

  clock_t start = clock();
  double elapsed = 0.0;

  // Prevent compiler optimizing loop away
  volatile unsigned long long sink = 0;

  while (elapsed < (double)seconds) {
    // garbage computations to consume CPU
    for (int i = 0; i < 500000; i++) {
      sink += (unsigned long long)i * 3ULL + 7ULL;
      sink ^= (sink << 1);
    }

    elapsed = (double)(clock() - start) / (double)CLOCKS_PER_SEC;
  }

  // No stdout output for true batch mode, but OK for early dev.
  // printf("batch_job done (%d seconds)\n", seconds);
  return 0;
}