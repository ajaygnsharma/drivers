import re

s = 'DRO: min = 1.2, max = 3.4, origin = 5.6';
expList = re.findall("\d+.\d+", s);
min,max,orig = expList;
print(expList);
print(min);
print(max);
print(orig);

