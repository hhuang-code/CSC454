# @ Author: Hao Huang
# @ Usage: In a command-line terminal, type: /u/cs254/bin/mcs combinations.cs, and then type: /u/cs254/bin/mono combinations.exe 2 5

import sys

# Print the list
# Param: res - the list to be printed
def printer(res):
	length = len(res)
	for i in range(length):
		print(res[i], ' ', end='')
	print()

"""
	Calculate C(k, n) combinations on an integer from 1 to n.
	Param: vec - an integer list [1, 2, ..., n]
	Param: res - a list to store the result and it will be re-written every time when a new result is generated.
	Param: k - input variable
	Param: n - input variable
	Param: spos - a counter during iteration
	Param: res_idx - an index on the res to keep the position of the result
"""
def combination(vec, res, k, n, spos, res_idx):
	if k == 0:
		yield res
		return
	epos = n - k
	for i in range(spos, epos + 1):	#left close, right open	
		res[res_idx] = vec[i]
		for p in combination(vec, res, k - 1, n, i + 1, res_idx + 1):
			yield p

# Program entry
if len(sys.argv) != 3:
	sys.stderr.write("usage: %s k n\n" % sys.argv[0])
	sys.exit(1)

vec = []
for i in range(int(sys.argv[2])):
	vec.append(i + 1)

res = list(range(int(sys.argv[1])))
for s in combination(vec, res, int(sys.argv[1]), int(sys.argv[2]), 0, 0):	# Iteration
	printer(s)


