import xml.etree.ElementTree as ET
import numpy as np
import matplotlib.pyplot as plt
import math


class Particle:
	def __init__(self,pdgid,spin,px=0,py=0,pz=0,energy=0,mass=0):
		self.pdgid = pdgid
		self.spin = spin
		self.px = px
		self.py = py
		self.pz = pz
		self.energy = energy
		self.mass = mass

	def get_PT(self):
		return math.sqrt((self.px*self.px + self.py*self.py))

	def get_eta(self):
		abs_p = (self.px**2 + self.py**2 + self.pz**2)**0.5
		theta = math.acos(self.pz / abs_p)
		eta = -math.log(math.tan(0.5*theta))
		return eta 

   	def get_phi(self):
      		return math.atan2(self.py, self.px) + np.pi



class Event:
	def __init__(self,num_particles):
        	self.num_particles=num_particles
        	self.particles=[]

	def __addParticle__(self,particle):
        	self.particles.append(particle)

    	def getParticlesByIDs(self,idlist):
        	partlist=[]
        	for pdgid in idlist:
            		for p in self.particles:
                		if p.pdgid==pdgid:
                    			partlist.append(p)
        	return partlist

class LHEFData:
    	def __init__(self,version):
    	    self.version=version
    	    self.events=[]

    	def __addEvent__(self,event):
    	    self.events.append(event)

    	def getParticlesByIDs(self,idlist):
    	    partlist=[]
    	    for event in self.events:
    	        partlist.extend(event.getParticlesByIDs(idlist))
    	    return partlist


def readLHEF(name):
    	tree = ET.parse(name)
   	root=tree.getroot()
    	lhefdata=LHEFData(float(root.attrib['version']))
    	for i,child in enumerate(root):
        	if(child.tag=='event'):
            		lines=child.text.strip().split('\n')
            		event_header=lines[0].strip()
            		num_part=int(event_header.split()[0].strip())
            		e=Event(num_part)
            		for i in range(1,num_part+1):
                		part_data=lines[i].strip().split()
                		p=Particle(int(part_data[0]), float(part_data[12]), float(part_data[6]), float(part_data[7]), float(part_data[8]), float(part_data[9]), float(part_data[10]))
                		e.__addParticle__(p)
            		lhefdata.__addEvent__(e)

    	return lhefdata



def get_invMass(part1,part2):
	E1 = part1.energy
	E2 = part2.energy
	p1x = part1.px
	p2x = part2.px
	p1y = part1.py
	p2y = part2.py
	p1z = part1.pz
	p2z = part2.pz
	m2 = part1.mass**2 + part2.mass**2 + 2*(E1*E2 - p1x*p2x - p1y*p2y - p1z*p2z)
	print math.sqrt(m2)
	return math.sqrt(m2)



if __name__ == "__main__":
	data = readLHEF("test_SM/Events/run_01/unweighted_events.lhe")

	parts_em  = data.getParticlesByIDs([11])  # e-
	parts_ep  = data.getParticlesByIDs([-11])  # e+
	parts_mum = data.getParticlesByIDs([13])  # mu-
	parts_mup = data.getParticlesByIDs([-13])   # mu+


	axes = plt.gca()
	#axes.set_xlim([700,2300])
	axes.set_ylim([0,150])
	invMass = []
	for i in range(len(parts_em)):
		invMass.append(get_invMass(parts_em[i], parts_ep[i]))
	for i in range(len(parts_mum)):
		invMass.append(get_invMass(parts_mum[i], parts_mup[i]))
	plt.hist(invMass, 50, fill=False, histtype='step')
	#plt.yscale('log')
	plt.show()

	#print "anti-top ID: ", [p.pdgid for p in parts_tbar[0:10]]
	#print "b ID: ", [p.pdgid for p in parts_b[0:10]]
	#print "mup ID: ", [p.pdgid for p in parts_mup[0:10]]


	#f = open("test_SM.txt","a")

	#for ev in range(len(parts_Wp)):
	#	#print "mu+ px: ", parts_mup[ev].px
    	#	line = ""
    	#	line = "--------\n"
    	#	line += "{} {} {} {}\n".format(parts_mum[ev].px, parts_mum[ev].py, parts_mum[ev].pz, parts_mum[ev].energy)
    	#	line += "{} {} {} {}\n".format(parts_bbar[ev].px, parts_bbar[ev].py, parts_bbar[ev].pz, parts_bbar[ev].energy)
    	#	line += "{} {} {} {}\n".format(parts_mup[ev].px, parts_mup[ev].py, parts_mup[ev].pz, parts_mup[ev].energy)
  	#  	line += "{} {} {} {}\n".format(parts_b[ev].px, parts_b[ev].py, parts_b[ev].pz, parts_b[ev].energy)
    	#	line += "{} {} {} {}\n".format(parts_nup[ev].px, parts_nup[ev].py, parts_nup[ev].pz, parts_nup[ev].energy)
   	# 	line += "{} {} {} {}\n".format(parts_num[ev].px, parts_num[ev].py, parts_num[ev].pz, parts_num[ev].energy)
    	#	f.write(line)

	#f.close()
