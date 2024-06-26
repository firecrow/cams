#!/usr/bin/env python
import sys, os, re

def phase1():
  max = 0;
  with open('.cams/next', "r") as file:
    max = int(file.read())
  print max

  for x in range(1, max):
    name = "%09d.%010d.firecrow" % (x, x)
    print name 
    os.rename('.cams/%d' % x, name)

def gen_mapping():
  hex_to_cid = {}

  def filter_cmd(x):
    return True if x.find('00') != -1 else False

  commits = sorted(filter(filter_cmd, os.listdir(".cams")))
  for c in commits:
    with open(".cams/%s/hex" % c, "r") as hexf:
      hex = hexf.read()
      hex_to_cid[hex] = c
  return hex_to_cid, commits


def phase2():
  hex_to_cid, commits = gen_mapping()
  ### rename files 
  #for x in sorted(os.listdir(".cams/files")):
  #  hex = x.split('.')[0]
  #  newname = x.replace(hex, hex_to_cid[hex])
  #  #os.rename(".cams/files/%s" % x, ".cams/files/%s" % newname)

  ### update each cindex
  for x in commits:
    with open(".cams/%s/cindex" % x, "r") as cindex:
      with open(".cams/%s/cindex.new" % x, "w+") as newfile:
        for line in cindex.readlines():
          if line.strip():
            hex = line.split(':')[0]
            newline = line.replace(hex, hex_to_cid[hex])
            newfile.write(newline)
            print "%s -> %s" % (line, newline)
    src = ".cams/%s/cindex.new" % x
    dest = ".cams/%s/cindex" % x
    os.rename(src, dest)

def phase3():
  # create prior records
  hex_to_cid, commits = gen_mapping()
  for c in commits:
    millis = int(c.split('.')[0]) 
    parent = "%09d.%010d.firecrow" % (millis-1, millis-1)
    with open(".cams/%s/parent" % c, "w+") as parentfile:
      parentfile.write(parent)

def phase4():
  def is_cid(cid):
    return re.search('[0-9]{9,10}.[0-9]{10}.[a-zA-Z0-9]+', cid)
  cids = filter(is_cid, os.listdir('.cams'))

  millis_to_cid = {}

  for cid in cids:
    millis = re.sub('\..*', '', cid)
    if millis_to_cid.get(millis):
      print "CONFLICT %s" % cid
      break
    else:
      with open('.cams/millis/%s' % millis, "w+") as file:
        file.write(cid)

  print len(cids)


if __name__ == "__main__":
  #phase1()
  #phase2()
  #phase3()
  phase4()
