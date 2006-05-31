import portage

class PortageInterface:
	def __init__(self):
		self.vartree = portage.db[portage.root]["vartree"]
		self.porttree = portage.db[portage.root]["porttree"]
		
	def installedPackages(self):
		return self.vartree.dbapi.cp_all()
		
	def installedPackagesVersion(self):
		t = []
		for cpv in self.vartree.dbapi.cpv_all():
			mysplit = portage.catpkgsplit(cpv)
			mypkg = "/".join(mysplit[:3])
			if mysplit[3] != "r0":
				mypkg = mypkg + "-" + mysplit[3]
			t.append( mypkg )
		return t
	
	def portagePackages(self):
		return self.porttree.dbapi.cp_all()

	def allPortagePackages(self):
		t = []
		t = self.porttree.dbapi.cp_all()
		t += self.vartree.dbapi.cp_all()
		return t

	def allPackagesVersion(self):
		t = []
		for x in self.porttree.dbapi.cp_all():
			t.extend( self.porttree.dbapi.cp_list(x) )
		return t

	def getAllPackageData(self, db, keys):
		rval = {}
		cplist = db.cp_all()
		cplist.sort()
		for cp in cplist:
			for cpv in db.cp_list(cp):
				rval[cpv] = db.aux_get(cpv, keys)
		return rval
	
	def allPackagesData(self):
		testdata = {}
		testdata = self.getAllPackageData(self.porttree.dbapi, ["DESCRIPTION", "HOMEPAGE", "LICENSE", "KEYWORDS", "IUSE", "SLOT"])
		t = []
		for cpv in testdata.keys():
			mysplit = portage.catpkgsplit(cpv)
			mypkg = "/".join(mysplit[:3])
			if mysplit[3] != "r0":
				mypkg = mypkg + "-" + mysplit[3]
			t.append( mypkg )
			for v in testdata[cpv]:
				t.append( v )
		return t
