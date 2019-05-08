APP = strategy
HPP = UCT.hpp
OBJECTS = Judge.o Strategy.o
OUTPUTS = outputs/*

CXXC = g++
CXXFLAGS = -std=c++11 -O2 -arch i386 -m32
LINKFLAGS = -arch i386 -m32 -dynamiclib

DEL = rm -rf

default:
	make clean -s
	make $(APP) -s

run:
	make $(APP) -s
	../Compete/bin/Compete ../Testcases/$(CASE).dylib ./strategy.dylib outputs/result.txt 1

$(APP): $(OBJECTS) $(HPP) Makefile
	echo 'Linking: $(APP)' && \
	$(CXXC) $(LINKFLAGS) $(OBJECTS) -o $(APP).dylib

%.o: %.cpp %.h Makefile
	echo 'Compiling components: $*.o' && \
	$(CXXC) $(CXXFLAGS) -c $*.cpp -o $*.o

clean:
	echo 'Cleaning all files ...' 
	make -s clean_object
	make -s clean_output

clean_object:
	echo 'Cleaning objects ...'
	-$(DEL) *.o
	-$(DEL) $(APP).dylib

clean_output:
	echo 'Cleaning outputs ...'
	-$(DEL) $(OUTPUTS)	