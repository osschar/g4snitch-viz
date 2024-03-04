URL_BASE:=https://raw.githubusercontent.com/osschar/cmssw/g4snitch-14.0-p3/SimG4Core/HelpfulWatchers

DATAFORMAT_SRCS := G4SnitchDataFormat.h G4S_LinkDef.h
DATAFORMAT_LIB  := libRootG4Snitch.so

all: test

${DATAFORMAT_SRCS} &:
	curl -O ${URL_BASE}/interface/G4SnitchDataFormat.h
	curl -O ${URL_BASE}/g4s-test/G4S_LinkDef.h

${DATAFORMAT_LIB} : ${DATAFORMAT_SRCS}
	rootcling -f RootG4Snitch.cc ${DATAFORMAT_SRCS}
	c++ -I${ROOTSYS}/include -I. RootG4Snitch.cc -fPIC -shared -o $@

test: ${DATAFORMAT_LIB}
	root.exe loader.C reve_draw.C

clean:
	rm -f ${DATAFORMAT_LIB} RootG4Snitch_rdict.pcm RootG4Snitch.cc

distclean: clean
	rm -f ${DATAFORMAT_SRCS}
