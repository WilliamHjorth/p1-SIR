import re
from pathlib import Path
p = Path('stochastic_replicates.txt')
if not p.exists():
    print('stochastic_replicates.txt not found; aborting fixed script generation')
    raise SystemExit(1)
text = p.read_text()
num = text.count('# Replicate')
print('Found', num, 'replicates in stochastic_replicates.txt')
old_path = Path('plot_replicates.gnu')
if not old_path.exists():
    print('plot_replicates.gnu not found; aborting')
    raise SystemExit(1)
old = old_path.read_text()

# Pattern for the specific for-plot usage we produced
pattern = re.compile(r"for \[i=0:(\d+)\] '([^']+)' index i using 1:(\d+) with lines ls (\d+) title '([^']*)'")
new = old

for m in list(pattern.finditer(old)):
    file = m.group(2)
    col = int(m.group(3))
    ls = m.group(4)
    label = m.group(5)
    parts = []
    for rep in range(num):
        if rep == 0:
            parts.append("'%s' index %d using 1:%d with lines ls %s title '%s'" % (file, rep, col, ls, label))
        else:
            parts.append("'%s' index %d using 1:%d with lines ls %s title ''" % (file, rep, col, ls))
    replacement = ", \\\n".join(parts)
    new = new.replace(m.group(0), replacement)

# Remove stray 'replot ' occurrences introduced by generator
new = new.replace('\nreplot ', '\n')

out_path = Path('plot_replicates_fixed.gnu')
out_path.write_text(new)
print('Wrote', out_path)
