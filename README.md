# lmViewer

**List Mode PET Coincidence Viewer** — a lightweight command-line tool for inspecting binary List Mode (LM) files produced by Bruker PET scanners (or compatible PenRed simulation pipelines).

---

## Overview

`lmviewer` reads a binary `.lm` file, decodes its header and coincidence records, and prints a formatted summary to the terminal. It is useful for quickly validating, debugging, or exploring List Mode data without loading a full reconstruction suite.

---

## File Format

A `.lm` file consists of:

| Section | Size | Description |
|---------|------|-------------|
| `LMHeader` | fixed (packed) | Acquisition metadata |
| `coincidence[]` | N × sizeof(coincidence) | Array of coincidence records |

### `LMHeader` fields

| Field | Type | Description |
|-------|------|-------------|
| `identifier` | `char[16]` | File identifier string |
| `rawCounts` | `double` | Total raw count estimate |
| `acqTime` | `double` | Acquisition time (s) |
| `activity` | `double` | Source activity (μCi) |
| `isotope` | `char[16]` | Isotope name |
| `detectorSizeX/Y` | `double` | Crystal face dimensions (mm) |
| `startTime` | `double` | Acquisition start (s) |
| `measurementTime` | `double` | Duration (s) |
| `moduleNumber` | `int` | Number of detector modules |
| `ringNumber` | `int` | Number of detector rings |
| `ringDistance` | `double` | Axial ring pitch (mm) |
| `detectorDistance` | `double` | Detector-to-center distance (mm) |
| `isotopeHalfLife` | `double` | Half-life (s) |
| `weight` | `float` | Phantom/subject weight |
| `maxTemp` | `float` | Maximum temperature (°C) |
| `percentLoss` | `float` | Count-rate loss (%) |
| `version` | `uint8_t[2]` | Software version |
| `calibrationID` | `uint8_t` | Calibration identifier |
| `gatePeriod` | `double` | Gating period (s) |
| `DOILayer` | `short` | Depth-of-interaction layer |
| `method` | `short` | Coincidence method index |
| `StudyID` | `short` | Study identifier |

### `coincidence` record fields

| Field | Type | Description |
|-------|------|-------------|
| `time` | `double` | Timestamp (s) |
| `energy1` | `float` | Photon 1 energy (keV) |
| `energy2` | `float` | Photon 2 energy (keV) |
| `amount` | `float` | Statistical weight |
| `xPosition1/2` | `unsigned short` | Crystal X index for each photon |
| `yPosition1/2` | `unsigned short` | Crystal Y index for each photon |
| `pair` | `unsigned short` | Module pair index |
| `gate_flag` | `unsigned short` | Gating flag |

---

## Building

The tool requires a C++17-capable compiler. No external dependencies are needed.

```bash
# With g++
g++ -std=c++17 -O2 -o lmviewer lmviewer.cpp

# With clang++
clang++ -std=c++17 -O2 -o lmviewer lmviewer.cpp
```

---

## Usage

```
Usage: lmviewer <file.lm> [options]

Options:
  --header    Print only the LM header
  --summary   Print header + statistics (no individual coincidences)
  -n N        Print only the first N coincidences
  --help      Show this help message
```

### Examples

```bash
# Print header + all coincidences + summary
./lmviewer scan.lm

# Print only the header and file layout
./lmviewer scan.lm --header

# Print header, statistics, but no individual events
./lmviewer scan.lm --summary

# Inspect only the first 100 coincidences
./lmviewer scan.lm -n 100
```

---

## Output

The tool produces three sections:

1. **LM Header** — all metadata fields decoded from the binary header.
2. **File Layout** — header size, record size, expected coincidence count, and a warning if the file has trailing bytes.
3. **Coincidences** — a formatted table of individual events (suppressed with `--summary`).
4. **Summary** — min/max/mean energies for both photons and the time range of the acquisition.

Example summary output:

```
───────────── Summary ──────────────────────
Total coincidences : 1482305

Energy 1 (keV):
  Min :    410.234
  Max :    589.761
  Mean:    511.003

Energy 2 (keV):
  Min :    412.118
  Max :    588.445
  Mean:    510.997

Time range:
  First:  1.000000000E-03 s
  Last :  5.999999800E+01 s
  Span :  5.999899800E+01 s
```

---

## Related Components (`common.hh`)

`common.hh` is a shared header used across the broader post-processing pipeline. It defines:

- **`single`** — a single photon detection event read from PenRed simulation output, with optional energy blurring.
- **`coincidence` / `LMHeader`** — the binary format structures described above.
- **`coincidenceCastor`** — compact coincidence format for CASToR reconstruction.
- **Enumerations** for coincidence methods, output formats, projection methods, attenuation, and normalization.
- **Helper functions**: `castorCrystalIndex`, `castorLOR`, `project`, `toLocal`, `attenuationFactorWithAir`, and 3D linear algebra utilities.

---

## License

Distributed under the GNU General Public License. See [LICENSE](LICENSE) for details.
