#! /usr/bin/env python
# tdg.py
# A transaction data generator, to generate transaction data (SAP BPC application)
# that can be used for testing (hopefully life can get easier!).
#
# Copyright (c) 2014 - Qianqian Zhu (zhuqianqian.299@gmail.com)

import sys
import random
import argparse
from datetime import datetime

parser = argparse.ArgumentParser(prog = 'tdg', description = 'A BPC transaction data generator tool')
parser.add_argument('dimfile', metavar = '<master_data_file>', type = argparse.FileType('rb'), 
	help = 'Master Data file')
parser.add_argument('--version', action='version', version = '%(prog)s: A BPC transaction data generator tool. version 1.3.3')
parser.add_argument('-v', '--verbose', action = 'store_true', help = 'Display verbose information during execution')
parser.add_argument('-s', '--separator', metavar = '<separator>', default = ',',
	help = 'Master data member separator in source file')
parser.add_argument('-o', '--output', metavar = '<output_file>', default = 'data.txt',
    help = 'Output file, default to data.txt if not set')
parser.add_argument('-d', '--delimiter', metavar = '<delimiter>', default = ',',
    help = 'Delimiter for the output file')
parser.add_argument('-i', '--min', type = int, metavar = '<minimum>', default = '-999999',
	help = 'The minimum value you want to generate')
parser.add_argument('-x', '--max', type = int, metavar = '<maximum>', default = '999999',
	help = 'The maximum value you want to generate')
parser.add_argument('-c', '--count', type = int, metavar = '<records_count', default = '0',
	help = 'The total number of records you want to get')

args = parser.parse_args()

try:
	if args.verbose:
		print("Reading master data...")
	file_content = args.dimfile.read()
except IOError:
	print("Error to read file.")
	quit()
finally:	
	args.dimfile.close()

try:
	if args.verbose:
		print("Creating output file...")
	output_file = open(args.output, "wb")
except IOError:
	print("Error to write file.")
	quit()

dim_names = []
dim_members = []
member_count = []
member_index = []
total = 1
if args.verbose:
	print("Parsing master data file...")
lines = file_content.split('\n')
n = len(lines) - 1
while n >= 0:
	if not lines[n]:
		del (lines[n])
	elif lines[n][-1] == '\r':
		lines[n] = lines[n][:-1]
	line = lines[n].split(':')
	dim_names.insert(0, line[0])
	member = line[1].split(args.separator)
	if not member[-1]:
		del (member[-1])
	dim_members.insert(0, member)
	member_count.insert(0, len(member))
	if args.verbose:
		print("    Resolved dimension: " + line[0] + ", " + str(len(member)) + " members found.")
	total = total * member_count[0]
	member_index.append(0)
	n = n - 1

# Make the return at the line begin, so no extra empty line is in target
i = 0
while i < len(dim_members[0]):
	dim_members[0][i] = '\n' + dim_members[0][i]
	i = i + 1

if args.verbose:
	print("Can get a maximum of " + str(total) + " transaction data records.")
if args.count > 0 and args.count < total:
	total = args.count
	if args.verbose:
		print("Will generate only " + str(total) + " records as specified.")

time_begin = datetime.now()
i = 0
m = len(dim_names)
temp_list = dim_names
temp_list.append("SIGNEDDATA")
header_entry = args.delimiter.join(temp_list)
output_file.write(header_entry)
if args.verbose:
	print("Generating transaction data records, value ranging from "+ str(args.min) + " to " + str(args.max))
while i < total:
	temp_list = []
	j = 0
	while j < m:
		temp_list.append(dim_members[j][member_index[j]])
		j = j + 1
	temp_list.append(str(random.randint(args.min,args.max)))
	data_entry = args.delimiter.join(temp_list)
	output_file.write(data_entry)
	j = m - 1
	while j >= 0:
		if(member_index[j] + 1 >= member_count[j]):
			member_index[j] = 0
		else:
			member_index[j] = member_index[j] + 1
			break
		j = j - 1
	i = i + 1

output_file.close()
time_end = datetime.now()
time_delta = time_end - time_begin
print("Total records generated: "+str(total)+", costing "+str(time_delta.total_seconds()) + " seconds.")
