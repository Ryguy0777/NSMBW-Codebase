import sys
import os
import re

if len(sys.argv) < 2:
    print("Usage: python maptokamek.py <file_path> [output_path]")
    sys.exit(1)

file_path = sys.argv[1]
output_path = sys.argv[2] if len(sys.argv) > 2 else (
    os.path.splitext(file_path)[0] + ".txt"
)

with open(file_path, 'r') as f:
    content = f.read()

def extract_symbols_from_all_sections(content):
    # find sections
    section_pattern = re.compile(r"^\.[\w\.]+.*$", re.MULTILINE)

    sections = [(m.start(), m.group()) for m in section_pattern.finditer(content)]
    symbols = []
    
    for i, (start_idx, section_name) in enumerate(sections):
        end_idx = sections[i+1][0] if i+1 < len(sections) else len(content)
        section = content[start_idx:end_idx]
        lines = section.splitlines()[1:]  # skip the section header line
        for line in lines:
            parts = line.strip().split()
            if len(parts) < 5:
                continue
            address = parts[0].upper()
            symbol = parts[4]

            if symbol.startswith('@') or symbol.startswith("hash_"): # unknown name
                continue

            symbols.append(f"{symbol}=0x{address}")
    return symbols

all_symbols = extract_symbols_from_all_sections(content)

with open(output_path, 'w') as out:
    out.write('\n'.join(all_symbols))

print(f"Symbols written to {output_path}")
