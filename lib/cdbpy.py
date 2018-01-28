# This file was created automatically by SWIG 1.3.29.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _cdbpy
import new
new_instancemethod = new.instancemethod
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


dbgeterror = _cdbpy.dbgeterror
dbcreate = _cdbpy.dbcreate
dbopen = _cdbpy.dbopen
dbclose = _cdbpy.dbclose
dbnumrecs = _cdbpy.dbnumrecs
dbnumfields = _cdbpy.dbnumfields
dbseq = _cdbpy.dbseq
dbiseof = _cdbpy.dbiseof
dbisbof = _cdbpy.dbisbof
dbshow = _cdbpy.dbshow
dbflen = _cdbpy.dbflen
dbdeclen = _cdbpy.dbdeclen
dbsetint = _cdbpy.dbsetint
dbsetnum = _cdbpy.dbsetnum
dbsetlog = _cdbpy.dbsetlog
dbsetchar = _cdbpy.dbsetchar
dbsetdate = _cdbpy.dbsetdate
dbsettime = _cdbpy.dbsettime
dbadd = _cdbpy.dbadd
dbretrieve = _cdbpy.dbretrieve
dbupdate = _cdbpy.dbupdate
dbdel = _cdbpy.dbdel
dbundel = _cdbpy.dbundel
dbisdeleted = _cdbpy.dbisdeleted
dbfldname = _cdbpy.dbfldname
dbfldtype = _cdbpy.dbfldtype
dbfldlen = _cdbpy.dbfldlen
dbflddec = _cdbpy.dbflddec
dbcurrent = _cdbpy.dbcurrent
dbgo = _cdbpy.dbgo
dbnext = _cdbpy.dbnext
dbnextindex = _cdbpy.dbnextindex
dbprev = _cdbpy.dbprev
dbprevindex = _cdbpy.dbprevindex
dbhead = _cdbpy.dbhead
dbheadindex = _cdbpy.dbheadindex
dbtail = _cdbpy.dbtail
dbtailindex = _cdbpy.dbtailindex
dbsearchindex = _cdbpy.dbsearchindex
dbsearchexact = _cdbpy.dbsearchexact
dbpack = _cdbpy.dbpack
dbreindex = _cdbpy.dbreindex
dbexit = _cdbpy.dbexit
md5sum = _cdbpy.md5sum
sha1sum = _cdbpy.sha1sum
rmd160sum = _cdbpy.rmd160sum
dbtime = _cdbpy.dbtime
dbnummidx = _cdbpy.dbnummidx
dbismidx = _cdbpy.dbismidx
dbmidxname = _cdbpy.dbmidxname
dbmidxblksz = _cdbpy.dbmidxblksz
dbmidxnumfldnames = _cdbpy.dbmidxnumfldnames
dbmidxfldname = _cdbpy.dbmidxfldname
dbisindexed = _cdbpy.dbisindexed
dbidxblksz = _cdbpy.dbidxblksz
dbshowinfo = _cdbpy.dbshowinfo
dbteststring = _cdbpy.dbteststring
dbtestupperstring = _cdbpy.dbtestupperstring
dbtestmixedstring = _cdbpy.dbtestmixedstring
dbtestnumber = _cdbpy.dbtestnumber
dbtestnumstring = _cdbpy.dbtestnumstring
dbencrypt = _cdbpy.dbencrypt
dbdecrypt = _cdbpy.dbdecrypt
crc32sum = _cdbpy.crc32sum
bcnumadd = _cdbpy.bcnumadd
bcnumsub = _cdbpy.bcnumsub
bcnumcompare = _cdbpy.bcnumcompare
bcnummultiply = _cdbpy.bcnummultiply
bcnumdivide = _cdbpy.bcnumdivide
bcnumraise = _cdbpy.bcnumraise
bcnumiszero = _cdbpy.bcnumiszero
bcnumisnearzero = _cdbpy.bcnumisnearzero
bcnumisneg = _cdbpy.bcnumisneg
bcnumuninit = _cdbpy.bcnumuninit


