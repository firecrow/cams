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
            newf.write("%s%s\n" % (hex, file.readline()[:-1]))
        except struct.error:
          pass
    os.rename("%s.new" % path, path)

  max = 0;
  with open('.cams/next', "r") as file:
    max = int(file.read())
  print max

  for x in range(1, max):
    update_cindex(x)


def phase3():
  #remove extra dot in generated cindexs

  def remove_period(id):
    results = {}
    path = ".cams/%d/cindex" % id
    with open(path, "rb") as file:
      with open("%s.new" % path, "w+") as newf:
        while(True):
          hex =  file.read(16)
          if len(hex) == 0:
            break
          fname = file.readline()[:-1]
          if fname[0] == '.':
            fname = fname[1:]
          newf.write("%s%s\n" % (hex, fname))
    os.rename("%s.new" % path, path)

  max = 0;
  with open('.cams/next', "r") as file:
    max = int(file.read())
  print max

  for x in range(1, max):
    remove_period(x)

def phase4():

  min = 190
  max = 200

  for x in range(min, max+1):
    hex = "%016d" % x 
    print "creating hex files %s" % hex
    with open(".cams/%d/hex" % x, "w+") as hexfile:
      hexfile.write(hex)
    with open(".cams/%d/time" % x, "w+") as hexfile:
      hexfile.write(hex)

def remove_nl_from_message():
  def get_message(x):
    with open(".cams/%d/message" % x, "r") as file:
      content = file.read()
      if content[-1] == "\n":
        content = content[:-1]
    print content
    with open(".cams/%d/message" % x, "w+") as file:
      file.write(content)

  with open('.cams/next', "r") as file:
    max = int(file.read())

  for x in range(1, max):
    get_message(x)

def cindex_color_sep():

  def update_w_colon(path):
    content = ""
    with open(path, "r") as cindex:
      for line in cindex.readlines():
        content += line[:16] + ':' + line[16:]
    newpath = path + '.new'
    with open(newpath, "w+") as file:
      file.write(content)
    os.rename(newpath, path)

  with open('.cams/next', "r") as file:
    max = int(file.read())

  for x in range(1, max):
    update_w_colon(".cams/%d/cindex" % x)
    
if __name__ == "__main__":
  #remove_nl_from_message()
  cindex_color_sep()

