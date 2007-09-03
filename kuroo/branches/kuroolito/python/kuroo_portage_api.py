#!/usr/bin/python

import os,sys
import portage

def installedPackages():
	vartree = portage.db[portage.root]["vartree"]
	print vartree.dbapi.cp_all()
	
def installedPackagesVersion():
	vartree = portage.db[portage.root]["vartree"]
	t = []
	for cpv in vartree.dbapi.cpv_all():
		mysplit = portage.catpkgsplit(cpv)
		mypkg = "/".join(mysplit[:3])
		if mysplit[3] != "r0":
			mypkg = mypkg + "-" + mysplit[3]
		print mypkg
		#t.append( mypkg )
	#print t

def portagePackages():
	porttree = portage.db[portage.root]["porttree"]
	return porttree.dbapi.cp_all()

def allPortagePackages():
	vartree = portage.db[portage.root]["vartree"]
	porttree = portage.db[portage.root]["porttree"]
	t = []
	t = porttree.dbapi.cp_all()
	t += vartree.dbapi.cp_all()
	return t

def allPackagesVersion():
	porttree = portage.db[portage.root]["porttree"]
	t = []
	for x in porttree.dbapi.cp_all():
		t.extend( porttree.dbapi.cp_list(x) )
	print t

def getAllPackageData(db, keys):
	rval = {}
	cplist = db.cp_all()
	cplist.sort()
	for cp in cplist:
		for cpv in db.cp_list(cp):
			rval[cpv] = db.aux_get(cpv, keys)
	return rval

def allPackagesData():
	porttree = portage.db[portage.root]["porttree"]
	testdata = {}
	testdata = getAllPackageData(porttree.dbapi, ["DESCRIPTION", "HOMEPAGE", "LICENSE", "KEYWORDS", "IUSE", "SLOT"])
	t = []
	for cpv in testdata.keys():
		mysplit = portage.catpkgsplit(cpv)
		mypkg = "/".join(mysplit[:3])
		if mysplit[3] != "r0":
			mypkg = mypkg + "-" + mysplit[3]
		t.append( mypkg )
		for v in testdata[cpv]:
			t.append( v )
	print t

installedPackagesVersion()
#allPackagesVersion()
#allPackagesData()
