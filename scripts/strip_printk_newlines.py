"""
Remove trailing \\n (or \\r\\n) from the first string literal in every
macro-form  printk("...\n")  call.  Leaves (printk)(...) raw calls untouched.
"""
import sys

def strip_newline(text):
    result = []
    i = 0
    while i < len(text):
        # Match macro-form: printk( but NOT (printk)(
        if text[i:i+7] == 'printk(' and (i == 0 or text[i-1] != '('):
            j = i + 7
            # skip whitespace to opening quote
            while j < len(text) and text[j] in ' \t\n':
                j += 1
            if j < len(text) and text[j] == '"':
                # scan to closing quote, respecting backslash escapes
                k = j + 1
                while k < len(text):
                    ch = text[k]
                    if ch == '\\':
                        k += 2
                        continue
                    if ch == '"':
                        break
                    k += 1
                seg = text[j+1:k]
                if seg.endswith('\\r\\n'):
                    seg = seg[:-4]
                    result.append(text[i:j+1] + seg + text[k:k+1])
                    i = k + 1
                    continue
                elif seg.endswith('\\n'):
                    seg = seg[:-2]
                    result.append(text[i:j+1] + seg + text[k:k+1])
                    i = k + 1
                    continue
        result.append(text[i])
        i += 1
    return ''.join(result)

for path in sys.argv[1:]:
    text = open(path, encoding='utf-8').read()
    new_text = strip_newline(text)
    if new_text != text:
        open(path, 'w', encoding='utf-8').write(new_text)
        print(f"updated  {path.split('/')[-1]}")
    else:
        print(f"no change {path.split('/')[-1]}")
