# lmViewer

**List Mode PET Coincidence Viewer** — a lightweight CLI tool for inspecting binary List Mode (`.lm`) files from PET scanners.

`lmviewer` decodes the file header and coincidence records, printing a formatted summary to the terminal. Useful for quickly validating or exploring LM data without a full reconstruction suite.

---

## Building

Requires a C++17-capable compiler, no external dependencies.

```bash
g++ -std=c++17 -O2 -o lmviewer lmviewer.cpp
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

---

## File Format

A `.lm` file is a packed binary: an `LMHeader` struct followed by an array of `coincidence` records.

**`LMHeader`** — acquisition metadata (identifier, isotope, activity, timing, detector geometry, gating, version, etc.)

**`coincidence`** — per-event record: `time` (s), `energy1/2` (keV), `xPosition1/2`, `yPosition1/2`, `pair`, `gate_flag`.

---

## Output Sections

1. **Header** — all metadata fields decoded from the binary header.
2. **File Layout** — record sizes, expected coincidence count, trailing-byte warnings.
3. **Coincidences** — formatted table of individual events (omitted with `--summary`).
4. **Summary** — min/max/mean energies for both photons and the acquisition time span.

---

## License

GNU General Public License. See [LICENSE](LICENSE).
