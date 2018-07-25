#!/usr/bin/env python
import sys, os, ctypes, struct

def phase1():

  max = 0;
  with open('.cams/next', "r") as file:
    max = int(file.read())
  print max

  def walker(hex, dirname, files):
    for f in files:
      src = "%s/%s" % (dirname, f)
      dest = ".cams/files/%s.%s" % (hex, f)
      print "moving %s to %s" % (src, dest)
      os.rename(src, dest)


  for x in range(1, max):
    hex = "%016d" % x 
    if not os.path.exists(".cams/%d/hex" % x):
      print "creating hex files %s" % hex
      with open(".cams/%d/hex" % x, "w+") as hexfile:
        hexfile.write(hex)
      with open(".cams/%d/time" % x, "w+") as hexfile:
        hexfile.write(hex)

    filespath =  ".cams/%d/files" % x
    if os.path.exists(filespath):
      os.path.walk(filespath, walker, hex)
        

def phase2():

  def update_cindex(id):
    results = {}
    path = ".cams/%d/cindex" % id
    with open(path, "rb") as file:
      with open("%s.new" % path, "w+") as newf:
        try:
          while(True):
            cid =  struct.unpack("h", file.read(2))[0];
            hex = "%016d" % cid;
            newf.write("%s.%s\n" % (hex, file.readline()[:-1]))
        except struct.error:
          pass
    os.rename("%s.new" % path, path)

  max = 0;
  with open('.cams/next', "r") as file:
    max = int(file.read())
  print max

  for x in range(1, max):
    update_cindex(x)


phase2()
