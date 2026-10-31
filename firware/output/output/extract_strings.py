import re

with open("uni_app_release_update.bin", "rb") as f:
    data = f.read()

# Extract all ASCII strings (printable characters 4+ in a row)
strings = re.findall(rb"[ -~]{4,}", data)

with open("ascii_strings.txt", "w", encoding="utf-8") as out:
    for s in strings:
        out.write(s.decode("utf-8", errors="replace") + "\n")

print("Saved to ascii_strings.txt")
