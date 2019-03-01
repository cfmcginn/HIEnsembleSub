#std-c++11
CXX = g++
CXXFLAGS = -Wall -O3 -Wextra -Wno-unused-local-typedefs -Wno-deprecated-declarations -std=c++11
ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

ifndef ENSEMBLESUBPATH
$(error ENSEMBLESUBPATH is not set at all. Do 'source setEnv.sh')
endif

ROOT=`root-config --cflags --glibs`
FASTJET=`$(FASTJETPATH)/bin/fastjet-config --cxxflags --libs`
INCLUDES=-I/usr/include -I$(PWD) -I$(FASTJETPATH)

MKDIR_BIN = mkdir -p $(PWD)/bin
MKDIR_PDFDIR = mkdir -p $(PWD)/pdfDir
MKDIR_OUTPUT = mkdir -p $(PWD)/output

all: mkdirBin mkdirOutput mkdirPdfdir bin/createEnsembleMaps.exe bin/hydjetToHFCent.exe bin/quickMedianCheck.exe bin/testID.exe bin/createEnsembleMapHistID.exe bin/compareOldNewMapID.exe bin/testEnsembleMaps.exe bin/sumCounts.exe

mkdirBin:
	$(MKDIR_BIN)

mkdirOutput:
	$(MKDIR_OUTPUT)

mkdirPdfdir:
	$(MKDIR_PDFDIR)


bin/createEnsembleMaps.exe: src/createEnsembleMaps.C
	$(CXX) $(CXXFLAGS) src/createEnsembleMaps.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/createEnsembleMaps.exe

bin/hydjetToHFCent.exe: src/hydjetToHFCent.C
	$(CXX) $(CXXFLAGS) src/hydjetToHFCent.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/hydjetToHFCent.exe

bin/quickMedianCheck.exe: src/quickMedianCheck.C
	$(CXX) $(CXXFLAGS) src/quickMedianCheck.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/quickMedianCheck.exe

bin/testID.exe: src/testID.C
	$(CXX) $(CXXFLAGS) src/testID.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/testID.exe

bin/createEnsembleMapHistID.exe: src/createEnsembleMapHistID.C
	$(CXX) $(CXXFLAGS) src/createEnsembleMapHistID.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/createEnsembleMapHistID.exe

bin/compareOldNewMapID.exe: src/compareOldNewMapID.C
	$(CXX) $(CXXFLAGS) src/compareOldNewMapID.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/compareOldNewMapID.exe

bin/testEnsembleMaps.exe: src/testEnsembleMaps.C
	$(CXX) $(CXXFLAGS) -g src/testEnsembleMaps.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/testEnsembleMaps.exe

bin/sumCounts.exe: src/sumCounts.C
	$(CXX) $(CXXFLAGS) -g src/sumCounts.C $(ROOT) $(FASTJET) $(INCLUDES) -lm -lpthread -lX11 -o bin/sumCounts.exe

clean:
	rm *~ || true
	rm *# || true
	rm include/*~ || true
	rm include/#*# || true
	rm src/*~ || true
	rm src/#*# || true
	rm bash/*~ || true
	rm bash/#*# || true
	rm bin/*.exe || true
	rmdir bin || true
