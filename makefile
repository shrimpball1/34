CXX        = g++
CXXFLAGS   = -std=c++17

GTEST_DIR  = /mnt/f/googletest/build/lib
GTEST_INC  = /mnt/f/googletest/googletest/include

INC_DIRS   = -I$(GTEST_INC) -Iinclude

LDLIBS     = -L$(GTEST_DIR) -lgtest -lgtest_main -lpthread

TARGET     = StringUtilsTest

SRCS       = src/StringUtils.cpp \
             testsrc/StringUtilsTest.cpp

all: $(TARGET)

$(TARGET): $(SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

clean:
	rm -f $(TARGET) src/*.o testsrc/*.o
