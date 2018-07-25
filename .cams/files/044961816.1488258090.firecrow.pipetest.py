#!/usr/bin/env python
import time, sys, random

terms = ['Apple\n', 'Ban', 'nan', 'na\n', 'Car', 'rot\n', 'Dishwasher\n', 'Elephant\n' 'Franklin\n' 'Grock\nHello\nInteresting\n', 'jasmine']

for term in terms:
  sys.stdout.write(term)
  sys.stdout.flush()
  time.sleep(random.random()*1)
sys.exit(3);
