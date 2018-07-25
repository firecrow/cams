#!/usr/bin/env python
import os, sys, md5
from subprocess import Popen, PIPE

camscwd = os.getcwd()
stg_path = camscwd + '/testing/stage'
fixture_path = camscwd + '/testing/fixtures/'
debug = True;

# handle working directory
def cmd(*args, **kwargs):
  cwd = kwargs["cwd"] if kwargs.get("cwd") else stg_path  
  shell = kwargs["shell"] if kwargs.get("shell") else False  
  p = Popen(args, stderr=PIPE, stdout=PIPE, cwd=cwd, shell=False)
  
  p.wait()
  if debug:
    sys.stderr.write(p.stderr.read())
  return p

def strcmp(a, b):
  return md5.new(a).hexdigest() == md5.new(b).hexdigest()

assert cmd("rm", "-Rf", "testing/stage", cwd=camscwd).returncode == 0, "rm existing"
assert cmd("mkdir", "testing/stage", cwd=camscwd).returncode == 0, "make new stage dir"
sys.stdout.write(".")

#c = cmd("../../cams", "help")
#assert c.returncode == 1, "usage"
#with open("testing/fixtures/usage", "r") as file:
#  assert strcmp(file.read(), c.stdout.read()), "usage should have expected output"
#sys.stdout.write(".")

assert cmd("../../cams", "init").returncode == 0, "init"
sys.stdout.write(".")
assert cmd("cp", "1/x", "../stage", cwd=fixture_path).returncode == 0, "set up fixture files"
assert cmd("../../cams", "add", "x").returncode == 0, "add first file"
sys.stdout.write(".")

assert cmd("diff", ".cams/stage/files/x", "x").returncode == 0, "there should be no diff between the file and the added file"
sys.stdout.write(".")

c = cmd("../../cams", "commit", "adding test file x")
assert c.returncode == 0, "committing file"
sys.stdout.write(".")

c = cmd("../../cams", "list")
assert c.returncode == 0, "list should succeed"
with open("testing/fixtures/list.1", "r") as file:
  print "new '%s'" % c.stdout.read()
  print "fxtr'%s'" %file.read()
  #assert strcmp(file.read(), c.stdout.read()), "list 1 should produce expected output"
sys.stdout.write(".")

assert cmd("cp", "2/x", "../stage", cwd=fixture_path).returncode == 0, "set up fixture files 2"
assert cmd("../../cams", "add", "x").returncode == 0, "add first file"
assert cmd("cp", "2/y", "../stage", cwd=fixture_path).returncode == 0, "set up fixture files 2"
assert cmd("../../cams", "add", "y").returncode == 0, "add first file"
sys.stdout.write(".")

assert cmd("../../cams", "commit", "adding test file x and y from 2").returncode == 0, "committing file"
sys.stdout.write(".")

c = cmd("../../cams", "list")
assert c.returncode == 0, "list should succeed"
with open("testing/fixtures/list.2", "r") as file:
  assert strcmp(file.read(), c.stdout.read()), "list 2 should have expected output"
sys.stdout.write(".")

#c = cmd("../../cams", "diff", "000000001.0000000001.testuser", "000000002.0000000002.testuser")
#assert c.returncode == 0, "diff should succeed"
#sys.stdout.write(".")
#
#with open("testing/fixtures/diff.1.2", "r") as file:
#  assert strcmp(file.read(), c.stdout.read()), "diff should output expected content"
#sys.stdout.write(".")

c = cmd("../../cams", "rm", "x")
assert c.returncode == 0, "rm should succeed"
sys.stdout.write(".")

with open("testing/fixtures/rm.3.x", "r") as file:
  with open("testing/stage/.cams/stage/remove", "r") as rmfile:
    assert strcmp(file.read(), rmfile.read()), "rm should output expected content"
sys.stdout.write(".")

with open("testing/fixtures/cindex.2", "r") as file:
  with open("testing/stage/.cams/000000002.0000000002.testuser/cindex") as cindex:
    assert strcmp(file.read(), cindex.read()), "cindex should contain two commits and two files"
sys.stdout.write(".")

c = cmd("../../cams", "commit", "removing test file x")
assert c.returncode == 0, "committing removed file"
sys.stdout.write(".")

with open("testing/fixtures/cindex.3", "r") as file:
  with open("testing/stage/.cams/000000002.0000000003.testuser/cindex", "r") as cindex:
    assert strcmp(file.read(), cindex.read()), "new cindex should not include x anymore"


print ""

"""
[all user facing commands]
x help 
x init
x add
x commit
x list
x diff
- rm
- checkout
- show
- reindex
- cindex
- reset
- push
"""

sys.exit(0)
