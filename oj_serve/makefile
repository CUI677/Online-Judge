CXX := g++
TARGET := oj_server
SRCS := oj_server.cc
OBJS := $(SRCS:.cc=.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS := -I/usr/include/jsoncpp
CXXFLAGS := -std=c++11 -pthread -MMD -MP
LDLIBS := -ljsoncpp -pthread

$(TARGET): $(OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS)
