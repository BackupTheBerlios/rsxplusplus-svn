import subprocess as sp
import os
import sys

TEMPLATE = "client/versiontemplate.h"
TARGET = "client/version.h"

def generateDefaultFile():
	try:
		versiontemplate = open(TEMPLATE,'r').read()
		versiontemplate = versiontemplate.replace("$WCREV$", "0")
		if not os.path.exists(TARGET):
			open(TARGET,'w').write(versiontemplate)
			print "Generated default version.h This will break update function!"
	except:
		print "Error during making default version.h header!"

try:
	p = sp.Popen("subwcrev.exe . %s %s" % (TEMPLATE, TARGET), stdout=sp.PIPE)
	result = p.communicate()[0]
	if result.find("is not a working copy") > 0:
		generateDefaultFile()
	else:
		print result
except:
	generateDefaultFile()