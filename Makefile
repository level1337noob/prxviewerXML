

CXX := g++
CXXFLAGS += -O3 -std=c++20 -fomit-frame-pointer -march=native

OBJS += main.o
prxviewer: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $*.o -c $<
