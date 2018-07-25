#!/usr/bin/env python
import os, re, errno

with open('.cams/current', 'r') as current:
    cid = current.read()    
    print 'checking out %s...' % cid
    with open('.cams/%s/cindex' % cid) as cindex:
        for f in cindex.readlines():
            fullname = f.strip()
            fname=re.sub('[^:]*:', '', fullname)
            dname = os.path.dirname(fname)
            if dname:
                try:
                    os.makedirs(dname)
                except Exception as e:
                    if e.errno == errno.EEXIST and os.path.isdir(dname):
                        pass
                    else:
                        raise
            fullname2 = re.sub(':%s' % fname, '.%s' % fname, fullname)
            fullname2 = re.sub('\/', '+', fullname2)
            src=open('.cams/files/%s' % fullname2, 'r')
            dest=open(fname, 'w+')
            dest.write(src.read())
            src.close()
            dest.close()
