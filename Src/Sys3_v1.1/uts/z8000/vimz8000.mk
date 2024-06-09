INCRT = $(ROOT)/usr/include

all: machine system drivers icpstuff vpmicpstuff

machine:
	cd ml; make -f vimml.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

system:
	cd os; make -f vimos.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

drivers:
	cd io; make -f vimio.mk "FRC=$(FRC)" "INCRT=$(INCRT)"

icpstuff:
#	cd icp; make -f icp.mk clean
#	cd icp; make -f icp.mk icp "FRC=$(FRC)" "INCRT=$(INCRT)"
#	cd icp; make -f icp.mk clean

vpmicpstuff:
#	cd icp; make -f vpmicp.mk clean
#	cd icp; make -f vpmicp.mk vpm0 "FRC=$(FRC)" "INCRT=$(INCRT)"
#	cd icp; make -f vpmicp.mk clean

clean:
	cd ml; make -f vimml.mk clean "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd os; make -f vimos.mk clean "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd io; make -f vimio.mk clean "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd icp; make -f icp.mk clean "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd icp; make -f vpmicp.mk clean "INCRT=$(INCRT)" "ROOT=$(ROOT)"

clobber:
	cd ml; make -f vimml.mk clobber "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd os; make -f vimos.mk clobber "INCRT=$(INCRT)" "ROOT=$(ROOT)"
	cd io; make -f vimio.mk clobber "INCRT=$(INCRT)" "ROOT=$(ROOT)"
#	cd icp; make -f icp.mk clobber "INCRT=$(INCRT)" "ROOT=$(ROOT)"
#	cd icp; make -f vpmicp.mk clobber "INCRT=$(INCRT)" "ROOT=$(ROOT)"

print:
	cd /$(ROOT)/usr/include; lnum sys/*.h *.h icp/*.h > /dev/lp
	cd /$(ROOT)/usr/src/uts/z8000/ml; make -f vimml.mk print "ROOT=$(ROOT)"
	cd /$(ROOT)/usr/src/uts/z8000/os; make -f vimos.mk print "ROOT=$(ROOT)"
	cd /$(ROOT)/usr/src/uts/z8000/io; make -f vimio.mk print "ROOT=$(ROOT)"
	cd /$(ROOT)/usr/src/uts/z8000/cf; make -f vimcf.mk print "ROOT=$(ROOT)"
	cd /$(ROOT)/usr/src/uts/z8000/icp; make -f icp.mk print "ROOT=$(ROOT)"
	cd icp; make -f vpmicp.mk print "ROOT=$(ROOT)"

cxref:
	cxref ml/*.s os/*.c io/*.c cf/*.[cs] icp/*/*.[cs]\
		icp/vpmicp/*/*.[cs] h/*.h \ > cxref.$(REL)

FRC:
