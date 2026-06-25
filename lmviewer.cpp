
#include "common.hh"
#include <cfloat>
#include <cstdlib>
#include <cstring>

static void printUsage(const char *prog) {
  printf(
      "Usage: %s <file.lm> [options]\n\n"
      "Options:\n"
      "  --header    Print only the LM header\n"
      "  --summary   Print header + statistics (no individual coincidences)\n"
      "  -n N        Print only the first N coincidences\n"
      "  --help      Show this help message\n",
      prog);
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  // Parse arguments
  const char *filename = argv[1];
  bool headerOnly = false;
  bool summaryOnly = false;
  long maxCoins = -1; // -1 means all

  for (int i = 2; i < argc; ++i) {
    if (strcmp(argv[i], "--header") == 0) {
      headerOnly = true;
    } else if (strcmp(argv[i], "--summary") == 0) {
      summaryOnly = true;
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      maxCoins = atol(argv[++i]);
      if (maxCoins <= 0) {
        printf("Error: -n requires a positive integer\n");
        return 1;
      }
    } else if (strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    } else {
      printf("Unknown option: %s\n", argv[i]);
      printUsage(argv[0]);
      return 1;
    }
  }

  // Open file
  FILE *fin = fopen(filename, "rb");
  if (fin == nullptr) {
    printf("Error: Unable to open file '%s'\n", filename);
    return 1;
  }

  // Get file size
  fseek(fin, 0, SEEK_END);
  long fileSize = ftell(fin);
  fseek(fin, 0, SEEK_SET);

  printf("═══════════════════════════════════════════\n");
  printf("  LM Viewer  —  List Mode File Inspector\n");
  printf("═══════════════════════════════════════════\n\n");
  printf("File          : %s\n", filename);
  printf("File size     : %ld bytes (%.2f KB)\n\n", fileSize,
         static_cast<double>(fileSize) / 1024.0);

  // Read header
  LMHeader header;
  if (fread(&header, sizeof(LMHeader), 1, fin) != 1) {
    printf("Error: File is too small to contain a valid LM header\n");
    printf("  Expected header size: %lu bytes\n",
           static_cast<unsigned long>(sizeof(LMHeader)));
    printf("  File size:            %ld bytes\n", fileSize);
    fclose(fin);
    return 1;
  }

  printf("───────────── LM Header ─────────────────\n");
  header.print();

  // Calculate expected coincidences
  long dataSize = fileSize - static_cast<long>(sizeof(LMHeader));
  long expectedCoins = dataSize / static_cast<long>(sizeof(coincidence));
  long remainder = dataSize % static_cast<long>(sizeof(coincidence));

  printf("\n───────────── File Layout ──────────────────\n");
  printf("Header size        : %lu bytes\n",
         static_cast<unsigned long>(sizeof(LMHeader)));
  printf("Coincidence size   : %lu bytes\n",
         static_cast<unsigned long>(sizeof(coincidence)));
  printf("Data region        : %ld bytes\n", dataSize);
  printf("Expected coincid.  : %ld\n", expectedCoins);
  if (remainder != 0) {
    printf("WARNING: %ld trailing bytes do not form a complete coincidence!\n",
           remainder);
  }

  if (headerOnly) {
    fclose(fin);
    return 0;
  }

  // Read all coincidences and compute statistics
  long count = 0;
  double minE1 = DBL_MAX, maxE1 = -DBL_MAX, sumE1 = 0.0;
  double minE2 = DBL_MAX, maxE2 = -DBL_MAX, sumE2 = 0.0;
  double minTime = DBL_MAX, maxTime = -DBL_MAX;

  // Determine how many to print
  long printLimit = (maxCoins > 0) ? maxCoins : expectedCoins;
  bool printAll = !summaryOnly;

  if (printAll) {
    printf("\n───────────── Coincidences ─────────────────\n");
    printf("%8s │ %10s │ %10s │ %5s %5s │ %5s %5s │ %4s │ %4s │ %15s\n",
           "Index", "Energy1", "Energy2", "X1", "Y1", "X2", "Y2", "Pair",
           "Gate", "Time");
    printf("─────────┼────────────┼────────────┼"
           "─────────────┼─────────────┼──────┼──────┼─────────────────\n");
  }

  coincidence c;
  while (fread(&c, sizeof(coincidence), 1, fin) == 1) {

    // Accumulate statistics
    sumE1 += c.energy1;
    sumE2 += c.energy2;
    if (c.energy1 < minE1)
      minE1 = c.energy1;
    if (c.energy1 > maxE1)
      maxE1 = c.energy1;
    if (c.energy2 < minE2)
      minE2 = c.energy2;
    if (c.energy2 > maxE2)
      maxE2 = c.energy2;
    if (c.time < minTime)
      minTime = c.time;
    if (c.time > maxTime)
      maxTime = c.time;

    // Print individual coincidence
    if (printAll && count < printLimit) {
      printf(
          "%8ld │ %10.3f │ %10.3f │ %5u %5u │ %5u %5u │ %4u │ %4u │ %15.9E\n",
          count, c.energy1, c.energy2, c.xPosition1, c.yPosition1, c.xPosition2,
          c.yPosition2, c.pair, c.gate_flag, c.time);
    } else if (printAll && count == printLimit) {
      printf("  ... (%ld more coincidences not shown) ...\n",
             expectedCoins - printLimit);
    }

    ++count;
  }

  // Print summary
  printf("\n───────────── Summary ──────────────────────\n");
  printf("Total coincidences : %ld\n", count);

  if (count > 0) {
    printf("\nEnergy 1 (keV):\n");
    printf("  Min : %10.3f\n", minE1);
    printf("  Max : %10.3f\n", maxE1);
    printf("  Mean: %10.3f\n", sumE1 / count);

    printf("\nEnergy 2 (keV):\n");
    printf("  Min : %10.3f\n", minE2);
    printf("  Max : %10.3f\n", maxE2);
    printf("  Mean: %10.3f\n", sumE2 / count);

    printf("\nTime range:\n");
    printf("  First: %15.9E s\n", minTime);
    printf("  Last : %15.9E s\n", maxTime);
    printf("  Span : %15.9E s\n", maxTime - minTime);
  } else {
    printf("  (no coincidences found)\n");
  }

  printf("\n═══════════════════════════════════════════\n");

  fclose(fin);
  return 0;
}
