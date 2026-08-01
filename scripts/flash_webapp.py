"""*
 * @file      flash_webapp.py
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief     
 * @details   Distributed globally for free under the MIT License terms.
 * 
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *"""
# file: flash_webapp.py
import os
import csv
import subprocess
import sys
import shutil

ESP_AT_PORT = "COM6"

# ======================
# Workspace & default paths
# ======================
WORKSPACE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

WEBAPP_PARTITION_NAME = "fatfs"  # Partition name in CSV
PARTITION_TABLE_FILE = os.path.join(WORKSPACE_DIR, "partitions.csv")
WEBAPP_DIR = os.path.join(WORKSPACE_DIR, "webapp")
IMAGE_FILE = os.path.join(WORKSPACE_DIR, "webapp_fatfs.bin")

# ======================
# Helpers
# ======================
def parse_size(size_str):
    """Convert size string like '1M' or '65536' to integer bytes."""
    size_str = size_str.upper()
    if size_str.endswith("K"):
        return int(size_str[:-1]) * 1024
    elif size_str.endswith("M"):
        return int(size_str[:-1]) * 1024 * 1024
    else:
        return int(size_str)

def find_partition_offset(partition_csv, partition_name):
    """Parse CSV partition table and return offset and size (in bytes) for the given partition."""
    if not os.path.isfile(partition_csv):
        raise FileNotFoundError(f"Partition table '{partition_csv}' does not exist")
    
    with open(partition_csv, newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or row[0].startswith("#"):
                continue
            name, _type, subtype, offset, size, *flags = [r.strip() for r in row]
            if name == partition_name:
                return int(offset, 0), parse_size(size)
    
    raise ValueError(f"Partition '{partition_name}' not found in {partition_csv}")

def generate_fatfs_image(input_dir, output_bin, partition_size_bytes):
    """Generate FATFS binary image from a folder using fatfsgen.py."""
    if not os.path.exists(input_dir):
        raise FileNotFoundError(f"Webapp directory '{input_dir}' does not exist")

    print(f"[+] Generating FATFS image from {input_dir} (max {partition_size_bytes} bytes)")

    print("IDF_PATH =", os.environ.get("IDF_PATH"))
    print("IDF_TOOLS_PATH =", os.environ.get("IDF_TOOLS_PATH"))
    print("IDF_PYTHON_ENV_PATH =", os.environ.get("IDF_PYTHON_ENV_PATH"))

    idf_path = os.environ.get("IDF_PATH", "")
    
    fatfsgen_py = os.path.join(idf_path, "components", "fatfs", "wl_fatfsgen.py")

    print(f"[+] IDF_PATH      : {idf_path}")
    print(f"[+] fatfsgen.py   : {fatfsgen_py}")

    if not os.path.isfile(fatfsgen_py):
        raise FileNotFoundError(f"Cannot find fatfsgen.py at '{fatfsgen_py}'")

    try:
        subprocess.run([
            sys.executable,
            fatfsgen_py,
            input_dir,
            "--output_file", output_bin,
            "--partition_size", str(partition_size_bytes),
            "--long_name_support"
        ], check=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"fatfsgen.py failed with return code {e.returncode}")

    print(f"[+] FATFS image saved to {output_bin}")

def flash_image_esptool(port, address, bin_file):
    """Flash a binary image to the ESP32 using the current Python environment."""

    print(f"[+] Flashing {bin_file} at 0x{address:X} via {port}")
    print(f"[+] Python      : {sys.executable}")

    cmd = [
        sys.executable,
        "-m",
        "esptool",
        "--chip", "esp32",
        "--port", port,
        "--baud", "460800",
        "write_flash",
        f"0x{address:X}",
        bin_file,
    ]

    print("[+] Command:")
    print("    " + " ".join(cmd))

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"esptool failed with return code {e.returncode}")


# ======================
# Main
# ======================
def main():
    try:
        # Step 1: Parse partition table
        offset, size_bytes = find_partition_offset(PARTITION_TABLE_FILE, WEBAPP_PARTITION_NAME)
        print(f"[+] Found partition '{WEBAPP_PARTITION_NAME}' at offset 0x{offset:X}, size {size_bytes} bytes")
    except (ValueError, FileNotFoundError) as e:
        print(f"[!] Error: {e}")
        sys.exit(1)

    try:
        # Step 2: Generate FATFS image
        generate_fatfs_image(WEBAPP_DIR, IMAGE_FILE, size_bytes)
    except (FileNotFoundError, RuntimeError) as e:
        print(f"[!] Error: {e}")
        sys.exit(1)

    try:
        # Step 3: Flash image
        serial_port = ESP_AT_PORT   # Replace with your actual port
        flash_image_esptool(serial_port, offset, IMAGE_FILE)
    except RuntimeError as e:
        print(f"[!] Error: {e}")
        sys.exit(1)

    print("[+] Webapp FATFS image flashed successfully!")

if __name__ == "__main__":
    main()
