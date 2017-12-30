"""
	#Cyclic 4
	R.<x1,x2,x3,x4> = QQ[]
	polys = [x1+x2+x3+x4,x1*x2+x2*x3+x3*x4+x4*x1,x1*x2*x3+x2*x3*x4+x3*x4*x1+x4*x1*x2]
	TropicalPrevariety(polys)
	#Should be equivalent (up to homogenization) to:
	R.ideal(polys).groebner_fan().tropical_intersection()

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
import os, inspect
# CHANGE THE LINE BELOW! Also, Sage has to be run in the same directory as the prevariety program.
pathToPrevariety = "/home/jeff/DynamicPrevariety"

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
    # Accept either an ideal or a list of polynomials.
	if type(polys) is sage.rings.polynomial.multi_polynomial_ideal.MPolynomialIdeal:
		polys = polys.gens()

	support = [[[Integer(j) for j in i] for i in poly.exponents()] for poly in polys]

	inputfile = open('inputfile', 'w')
	for poly in support:
		inputfile.write("%s\n" % poly)
	inputfile.close()
	call([pathToPrevariety + "/dynamicprevariety", pathToPrevariety + "/inputfile", "-t", str(ProcessCount)])
	
	return ParseOutput(pathToPrevariety + "/output.txt")
