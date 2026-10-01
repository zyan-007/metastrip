# metastrip

![version](https://img.shields.io/badge/version-1.0.0-brightgreen)
![language](https://img.shields.io/badge/language-C-blue)
![license](https://img.shields.io/badge/license-MIT-lightgrey)
![platform](https://img.shields.io/badge/platform-JPEG%20only-orange)

**metastrip** is a command-line tool, written in C from scratch, for inspecting and removing the metadata segments hidden inside JPEG files — EXIF, XMP, ICC color profiles, comments, and raw APP0–APP15 marker segments — without needing a GUI or a heavyweight library.

---

## See it in action

**Inspecting a JPEG's metadata:**

![metastrip showing metadata of a JPEG file](assets/main-function.gif)

**Stripping metadata (whole file, a single target, and a custom output path):**

![metastrip stripping metadata from a JPEG file](assets/demo-strip.gif)

---

## Features

- View **every** metadata segment in a JPEG at once, or filter down to exactly what you want
- **Strip** metadata out entirely — the whole file, or just the target(s) you choose — and write the result to a new file, never touching your original
- Target segments either by **JPEG marker number** (`app0` … `app15`, `com`) or by **content identifier** (`exif`, `xmp`, `icc`, `iptc`) — metastrip reads the actual payload signature to tell apart, for example, the two different things that can live inside an `APP1` segment (EXIF vs. XMP)
- Detect **trailing data** appended after the end of the actual image stream
- Combine multiple targets in a single command (`app0,com,exif`)
- Input validation that catches invalid targets, duplicate targets, and malformed combinations *before* ever opening your file
- **Automatic, collision-free output naming** when stripping — or pin down an exact output path yourself with `-o`/`--output`
- Refuses to ever overwrite an existing file, whether the output name was generated automatically or given explicitly
- Zero external dependencies — pure C, standard library only

---

## Prerequisites

You need a **C compiler** installed. That's it — no other libraries or dependencies are required.

- **Windows:** [MinGW-w64](https://www.mingw-w64.org/) (provides `gcc`), or the compiler bundled with [MSYS2](https://www.msys2.org/)
- **Linux:** `gcc` (usually already installed, or `sudo apt install build-essential` / your distro's equivalent)
- **macOS:** Xcode Command Line Tools (`xcode-select --install`), which provides `clang`/`gcc`

Check you have one available:

```bash
gcc --version
```

---

## Installation

### Windows: download the prebuilt binary

Grab `metastrip-<version>-windows-x64.zip` from the [Releases](https://github.com/zyan-007/metastrip/releases) page, extract it, and you'll have `metastrip.exe` ready to run — no compiler needed. Skip to [Running it from anywhere](#running-it-from-anywhere-without-recompiling-every-time) to add it to your `PATH`.

### Build from source (Windows, Linux, macOS)

#### 1. Clone the repository

```bash
git clone git@github.com:zyan-007/metastrip.git
cd metastrip
```

#### 2. Compile it

```bash
gcc main.c -o metastrip
```

On Windows this produces `metastrip.exe`; on Linux/macOS it produces `metastrip` (no extension). This step needs to be repeated any time `main.c` changes — see below for how to avoid retyping the full path every time you run it.

#### 3. Run it

From inside the project folder:

```bash
# Windows
.\metastrip.exe show all photo.jpg

# Linux / macOS
./metastrip show all photo.jpg
```

---

## Running it from anywhere (without recompiling every time)

Once you've compiled the binary, you don't need to rebuild it again just to use it from a different folder — you only need the compiler when the *source code* changes. To be able to type `metastrip` from **any** directory, add the folder containing the compiled binary to your system's `PATH`.

### Windows — temporary (current terminal session only)

Good for quickly trying it out without touching your permanent settings:

```powershell
$env:PATH += ";C:\path\to\metastrip"
```

This only lasts until you close that terminal window.

### Windows — permanent

1. Press `Win`, search **"Environment Variables"**, open **"Edit the system environment variables"**
2. Click **Environment Variables…**
3. Under **User variables** (or **System variables** to make it available to all users), select `Path` → **Edit** → **New**
4. Add the full folder path, e.g. `C:\path\to\metastrip`
5. Click OK on every dialog, then **open a new terminal window** for the change to take effect

### Linux / macOS — temporary (current shell session only)

```bash
export PATH="$PATH:/path/to/metastrip"
```

### Linux / macOS — permanent

Add the same `export` line above to your shell's config file (`~/.bashrc`, `~/.zshrc`, etc.), then reload it:

```bash
source ~/.bashrc   # or ~/.zshrc, depending on your shell
```

Once set up, you can run `metastrip <command>` from any directory, on any file, without `cd`-ing into the project folder or recompiling.

---

## Usage

```
metastrip show <file>
metastrip show <target> <file>
metastrip strip <file>
metastrip strip <target> <file>
metastrip strip <file> -o|--output <output-file>
metastrip strip <target> <file> -o|--output <output-file>
metastrip -h | --help
metastrip -v | --version
```

- `metastrip show <file>` — shows **all** metadata segments found in the file (equivalent to explicitly passing `all` as the target)
- `metastrip show <target> <file>` — shows only the segment(s) matching `<target>`
- `metastrip strip <file>` — writes a copy of the file with **all** metadata removed (equivalent to `strip all <file>`)
- `metastrip strip <target> <file>` — writes a copy with only the matching segment(s) removed; everything else is preserved
- `metastrip strip ... -o <output-file>` / `--output <output-file>` — write the result to a specific path instead of an auto-generated one
- `<target>` can be a **comma-separated list** of multiple targets, e.g. `app0,com,exif`
- `show` only ever reads the file. `strip` never modifies the input file — it always writes to a separate output file.

### Targets

| Target             | Matches                                                                 |
|--------------------|--------------------------------------------------------------------------|
| `all`              | Every segment in the file (cannot be combined with any other target)     |
| `app0` … `app15`   | A specific APP marker segment, by its raw marker number                  |
| `com`              | The comment (COM) segment                                                |
| `trailing`         | Any data found appended after the end of the actual image stream         |
| `exif`             | The EXIF data segment, identified by its payload signature (not just its marker number) |
| `xmp`              | The XMP data segment, identified by its payload signature                |
| `icc`              | The embedded ICC color profile, identified by its payload signature      |
| `iptc`             | IPTC metadata, identified by its payload signature                       |

> **Note:** EXIF, XMP, the ICC profile, and IPTC data are matched by reading the actual bytes at the start of the payload — not just by which `APPn` slot they happen to sit in. This matters because a single marker number can hold more than one kind of data (for example, `APP1` is used by both EXIF and XMP, and `APP2` can hold more than just an ICC profile), so metastrip peeks at the payload itself to tell them apart correctly.

### Validation rules

- `all` cannot be combined with any other target — `show all,app0` is rejected
- The same target cannot be repeated — `show app0,app0` is rejected
- An unrecognized target, or an out-of-range app number (e.g. `app16`), is rejected before the file is even opened
- Requesting a broader target automatically covers the more specific targets nested inside it, so you won't get contradictory or duplicate output — e.g. `app1` covers `exif`/`xmp`, `app2` covers `icc`, `app13` covers `iptc`

### Examples

```bash
# Show everything found in the file
metastrip show photo.jpg

# Show only EXIF data
metastrip show exif photo.jpg

# Show a specific APP segment by number
metastrip show app3 photo.jpg

# Show multiple targets at once
metastrip show app0,com,trailing photo.jpg

# Show EXIF, XMP, and the ICC profile together
metastrip show exif,xmp,icc photo.jpg

# Redirect output to a file
metastrip show exif photo.jpg > metadata.txt

# Strip everything, let metastrip name the output file
metastrip strip photo.jpg

# Strip only the EXIF data, leave everything else intact
metastrip strip exif photo.jpg

# Strip multiple targets at once
metastrip strip exif,xmp,icc photo.jpg

# Strip everything and choose the output path yourself
metastrip strip photo.jpg -o clean.jpg

# Strip a specific target with a custom output path
metastrip strip exif photo.jpg --output clean.jpg
```

---

## Automatic output filename generation

`strip` never modifies the file you give it — it always writes the result to a separate output file, and it will **never overwrite an existing file**, whether that output file's name was generated automatically or given explicitly with `-o`/`--output`.

### When no `-o`/`--output` is given

metastrip builds the output name itself as `stripped_<original-name>`:

```
metastrip strip photo.jpg          ->  stripped_photo.jpg
```

If `stripped_photo.jpg` already exists, it tries `stripped_1_photo.jpg`, then `stripped_2_photo.jpg`, and so on, counting up until it finds a name that isn't taken. This search is capped — if every number up to the limit is already in use (999 numbered attempts on top of the base name), metastrip gives up rather than looping forever:

```
Max file generation limit hit, please delete some existing file or use -o to generate name
```

At that point, delete some old `stripped_*` files or pass `-o`/`--output` with an explicit name.

### When `-o`/`--output` is given

- If the name you give has **no extension**, metastrip appends the same extension as the input file (`-o clean` on `photo.jpg` → `clean.jpg`).
- If the name you give **does have an extension**, it must match the input file's extension exactly, or the command is rejected before anything is written:
  ```
  !! Extensions provided for input and output file are both incompatible, please provide same extension !!
  ```
- Either way, if the resulting filename already exists, metastrip refuses to touch it:
  ```
  !! The Output File Provided, Overwriting any existing file is forbidden !!
  ```

---

## Current limitations

- **JPEG files only.** metastrip currently only understands the JPEG file structure (`FF D8` header, `FFxx` marker segments). Other image formats (PNG, TIFF, HEIC, etc.) are not supported.
- EXIF, XMP, ICC, and IPTC data shown by `show` are displayed as **raw payload bytes** (printable characters shown as-is, everything else shown as `.`) — metastrip does not yet decode the internal structure of EXIF (its TIFF/IFD tag tree) or parse XML from XMP into individual fields.
- `strip` removes whole metadata segments; it does not selectively edit or redact fields within a segment.

---

## License

Released under the [MIT License](LICENSE).
