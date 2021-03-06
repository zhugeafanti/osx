#!/usr/bin/env python
#
# transform_sql.py -- create a header file with the appropriate SQL variables
# from an SQL file
#


import os
import re
import sys


def usage_and_exit(msg):
  if msg:
    sys.stderr.write('%s\n\n' % msg)
  sys.stderr.write(
    'USAGE: %s SQLITE_FILE [OUTPUT_FILE]\n'
    '  stdout will be used if OUTPUT_FILE is not provided.\n'
    % os.path.basename(sys.argv[0]))
  sys.stderr.flush()
  sys.exit(1)


def main(input, output, filename):
  input = input.read()

  var_name = re.sub('[-.]', '_', filename)

  output.write(
    '/* This file is automatically generated from %s.\n'
    ' * Do not edit this file -- edit the source and rerun gen-make.py */\n'
    '\n'
    % (filename,))

  output.write('#define %s \\\n' % var_name.upper())

  regex = re.compile(r'/\*.*?\*/', re.MULTILINE|re.DOTALL)
  input = regex.sub('', input)

  for line in input.split('\n'):
    line = line.replace('"', '\\"')

    if line.strip():
      # got something besides whitespace. write it out.
      output.write('  "' + line + '"\\\n')

  output.write('  ""\n')


if __name__ == '__main__':
  if len(sys.argv) < 2 or len(sys.argv) > 3:
    usage_and_exit('Incorrect number of arguments')

  # Note: we could use stdin, but then we'd have no var_name
  input_file = open(sys.argv[1], 'r')

  if len(sys.argv) > 2:
    output_file = open(sys.argv[2], 'w')
  else:
    output_file = sys.stdout

  main(input_file, output_file, os.path.basename(sys.argv[1]))
