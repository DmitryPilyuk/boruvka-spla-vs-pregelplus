CXX = mpic++
CPPFLAGS += -I$(HADOOP_HOME)/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -I../deps/PregelPlus -I../deps/PregelPlus/system -Wno-deprecated -O2
LDFLAGS += -L$(HADOOP_HOME)/lib/native -lhdfs

TARGETS = boruvka.out boruvka_reqresp.out

.PHONY: all clean

all: $(TARGETS)

%.out: %.cpp
	$(CXX) $(CPPFLAGS) $< -o $@ $(LDFLAGS) 

clean:
	@echo "Cleaning up..."
	-rm -f $(TARGETS)
