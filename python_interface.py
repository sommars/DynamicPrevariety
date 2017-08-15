"""
	#Cyclic 4
	R.<x1,x2,x3,x4> = QQ[]
	polys = [x1+x2+x3+x4,x1*x2+x2*x3+x3*x4+x4*x1,x1*x2*x3+x2*x3*x4+x3*x4*x1+x4*x1*x2]
	TropicalPrevariety(polys)
	#Should be equivalent (up to homogenization) to:
	R.ideal(polys).groebner_fan().tropical_intersection().rays()

	#Reduced cyclic 8
	R.<y_1,y_2,y_3,y_4,y_5,y_6,y_7> = QQ[]
	polys = [1 + y_1 + y_2 + y_3 + y_4 + y_5 + y_6 + y_7,y_1 + y_1*y_2 + y_2*y_3
	+ y_3*y_4 + y_4*y_5 + y_5*y_6 + y_6*y_7 + y_7,y_1*y_2 + y_1*y_2*y_3
	+ y_2*y_3*y_4 + y_3*y_4*y_5 + y_4*y_5*y_6 + y_5*y_6*y_7
	+ y_6*y_7 + y_7*y_1,y_1*y_2*y_3 + y_1*y_2*y_3*y_4 + y_2*y_3*y_4*y_5 
	+ y_3*y_4*y_5*y_6 + y_4*y_5*y_6*y_7 + y_5*y_6*y_7 + y_6*y_7*y_1
	+ y_7*y_1*y_2,y_1*y_2*y_3*y_4 + y_1*y_2*y_3*y_4*y_5 + y_2*y_3*y_4*y_5*y_6
	+ y_3*y_4*y_5*y_6*y_7 + y_4*y_5*y_6*y_7 + y_5*y_6*y_7*y_1 + y_6*y_7*y_1*y_2
	+ y_7*y_1*y_2*y_3,y_1*y_2*y_3*y_4*y_5 + y_1*y_2*y_3*y_4*y_5*y_6
	+ y_2*y_3*y_4*y_5*y_6*y_7 + y_3*y_4*y_5*y_6*y_7 + y_4*y_5*y_6*y_7*y_1
	+ y_5*y_6*y_7*y_1*y_2 + y_6*y_7*y_1*y_2*y_3
	+ y_7*y_1*y_2*y_3*y_4,y_1*y_2*y_3*y_4*y_5*y_6 + y_1*y_2*y_3*y_4*y_5*y_6*y_7
	+ y_2*y_3*y_4*y_5*y_6*y_7+ y_3*y_4*y_5*y_6*y_7*y_1 + y_4*y_5*y_6*y_7*y_1*y_2
	+ y_5*y_6*y_7*y_1*y_2*y_3+ y_6*y_7*y_1*y_2*y_3*y_4 + y_7*y_1*y_2*y_3*y_4*y_5]
	TropicalPrevariety(polys)
"""

from subprocess import Popen, PIPE, call
#The below should work for generic machines
import os, inspect
pathToPrevariety = os.path.dirname(inspect.stack()[0][1])

def ParseOutput(FileName):
  with open(FileName,"r") as OutputFile:
  	s = OutputFile.read()
	IndexToRayMap = {}
	ConesList = []
	Rays = []
	for l in s.splitlines():
		if "vector" in l: # We are finished
			break
		if ":" in l: # it is a ray
			(Index,Ray) = l.split(":")
			Ray = eval(Ray.replace("{","[").replace("}","]"))
			Rays.append(Ray)
			IndexToRayMap[eval(Index.replace(":",""))] = Ray
			continue
		if "{" in l: # it is a cone
			ConesList.append(Cone([IndexToRayMap[i] for i in eval(l.replace("{","[").replace("}","]"))]))
			continue
	Rays.sort()
	return (ConesList, Rays)

def TropicalPrevariety(polys, ProcessCount = 1):
	support = str([[[Integer(j) for j in i] for i in poly.exponents()] for poly in polys]).replace("], ", "]").replace(" ","")
	if ProcessCount < 10:
	   support = '0' + str(ProcessCount) + support
	else: 
	   support = str(ProcessCount) + support
	
	call([pathToPrevariety + "/prevariety", pathToPrevariety + "/examples/cyclic/cyclic8"])
	
	#if len(ans) > 0 and ans[0] != '[':
	#	raise Exception("Internal error in tropical_prevariety")
	return ParseOutput(pathToPrevariety + "/output.txt")
