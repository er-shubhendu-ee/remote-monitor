"""*
 * @file      build_c_cpp_properties.py
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

# build_c_cpp_properties.py
import os, json

# Root folder and workspace root
ROOT = os.path.abspath(os.path.dirname(__file__))
PARENT = os.path.abspath(os.path.join(ROOT, ".."))

# VSCode C/C++ configuration path
VSCODE_CONFIG_PATH = os.path.join(PARENT, ".vscode", "c_cpp_properties.json")

# Folders to skip during header search
EXCLUDE_FOLDERS = {"build", ".vscode", ".devcontainer", "docs", "webapp"}


def find_header_dirs(root):
    """Return sorted list of directories containing at least one .h file, excluding EXCLUDE_FOLDERS."""
    out = set()
    for d, _, files in os.walk(root):
        if any(x in d.split(os.sep) for x in EXCLUDE_FOLDERS):
            continue
        if any(f.endswith(".h") for f in files):
            out.add(d)
    return sorted(out)


def to_ws_path(path):
    """Convert absolute path to workspace-relative path for includePath (recursive)."""
    rel = path[len(PARENT):].replace(os.sep, "/")
    ws = f"${{workspaceFolder}}{rel}/**"
    return ws.replace("//", "/")


def to_ws_path_browse(path):
    """Convert absolute path to workspace-relative path for browse.path (folder only, no /**)."""
    rel = path[len(PARENT):].replace(os.sep, "/")
    ws = f"${{workspaceFolder}}{rel}"
    return ws.replace("//", "/")


def normalize_paths(paths):
    """Convert any backslashes to forward slashes."""
    return [p.replace("\\", "/") for p in paths]


def main():
    hdr_dirs = find_header_dirs(PARENT)

    # Prepare paths for includePath and browse.path separately
    include_paths = normalize_paths([to_ws_path(p) for p in hdr_dirs])
    browse_paths = normalize_paths([to_ws_path_browse(p) for p in hdr_dirs])

    print("Folders to be added:")
    for p in hdr_dirs:
        print(" ", p)

    # Load existing VSCode config
    with open(VSCODE_CONFIG_PATH, encoding="utf-8") as f:
        cfg = json.load(f)

    # Merge new paths with existing entries, avoid duplicates
    for c in cfg.get("configurations", []):
        if "includePath" in c:
            c["includePath"] = list(dict.fromkeys(normalize_paths(c["includePath"]) + include_paths))
        if "browse" in c and "path" in c["browse"]:
            c["browse"]["path"] = list(dict.fromkeys(normalize_paths(c["browse"]["path"]) + browse_paths))

    # Save updated config
    with open(VSCODE_CONFIG_PATH, "w", encoding="utf-8") as f:
        json.dump(cfg, f, indent=4)


if __name__ == "__main__":
    main()
