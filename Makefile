URL_BASE := https://raw.githubusercontent.com/osschar/cmssw/g4snitch-14.0-p3/SimG4Core/HelpfulWatchers

DATAFORMAT_SRCS := G4SnitchDataFormat.h G4S_LinkDef.h
DATAFORMAT_LIB  := libRootG4Snitch.so

all: ${DATAFORMAT_LIB}

dataformats: ${DATAFORMAT_LIB}

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

DATA_BASE :=  http://xrd-cache-1.t2.ucsd.edu/matevz/g4s-data
DATA_MANIFEST := data-manifest.txt

${DATA_MANIFEST}:
	echo Downloading ${DATA_MANIFEST}
	@curl -sS ${DATA_BASE}/ | perl -ne 'print "$$1\n" if m!<a href="([-\w\d\.]+)">!;' > ${DATA_MANIFEST}

get-data-root: ${DATA_MANIFEST}
	@grep .root ${DATA_MANIFEST} | xargs -i sh -c "echo Downloading {}; curl -sSO ${DATA_BASE}/{}"

get-data-json: ${DATA_MANIFEST}
	@grep .json ${DATA_MANIFEST} | xargs -i sh -c "echo Downloading {}; curl -sSO ${DATA_BASE}/{}"

clean-data:
	@echo Removing downloaded data
	@if [ -e ${DATA_MANIFEST} ]; then cat ${DATA_MANIFEST} | xargs rm -f; fi
	@echo Removing ${DATA_MANIFEST}
	@rm -f ${DATA_MANIFEST}
