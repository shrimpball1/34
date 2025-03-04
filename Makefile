CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# googletest lcoal path
GTEST_DIR = /mnt/f/googletest
GTEST_INC = $(GTEST_DIR)/googletest/include
GTEST_LIB = $(GTEST_DIR)/build/lib

GTEST_LDFLAGS = -L$(GTEST_LIB) -lgtest -lgtest_main -pthread
EXPAT_LDFLAGS = -lexpat

# dic
SRCDIR = src
TESTDIR = testsrc
INCDIR = include
OBJDIR = obj
BINDIR = bin

# src
SRCS = \
  $(SRCDIR)/DSVReader.cpp \
  $(SRCDIR)/DSVWriter.cpp \
  $(SRCDIR)/XMLReader.cpp  \
  $(SRCDIR)/XMLWriter.cpp  \
  $(SRCDIR)/StringDataSource.cpp \
  $(SRCDIR)/StringDataSink.cpp \
  $(SRCDIR)/StringUtils.cpp \
  $(SRCDIR)/CSVBusSystem.cpp \
  $(SRCDIR)/OpenStreetMap.cpp

# test
TESTSRCS = \
  $(TESTDIR)/StringDataSourceTest.cpp \
  $(TESTDIR)/StringDataSinkTest.cpp \
  $(TESTDIR)/StringUtilsTest.cpp \
  $(TESTDIR)/testdsv.cpp \
  $(TESTDIR)/testxml.cpp \
  $(TESTDIR)/CSVBusSystemTest.cpp \
  $(TESTDIR)/OpenStreetMapTest.cpp

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
TESTOBJS = $(patsubst $(TESTDIR)/%.cpp, $(OBJDIR)/%.o, $(TESTSRCS))

TEST_EXECS = \
  $(BINDIR)/teststrdatasource \
  $(BINDIR)/teststrdatasink \
  $(BINDIR)/teststrutils \
  $(BINDIR)/testdsv \
  $(BINDIR)/testxml \
  $(BINDIR)/testcsvbs \
  $(BINDIR)/testosm

.PHONY: all clean

all: $(TEST_EXECS)


$(BINDIR)/teststrdatasource: \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSourceTest.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/teststrdatasink: \
  $(OBJDIR)/StringDataSink.o \
  $(OBJDIR)/StringDataSinkTest.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/teststrutils: \
  $(OBJDIR)/StringUtils.o \
  $(OBJDIR)/StringUtilsTest.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testdsv: \
  $(OBJDIR)/DSVReader.o \
  $(OBJDIR)/DSVWriter.o \
  $(OBJDIR)/testdsv.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testxml: \
  $(OBJDIR)/XMLReader.o \
  $(OBJDIR)/XMLWriter.o \
  $(OBJDIR)/testxml.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testcsvbs: \
  $(OBJDIR)/CSVBusSystem.o \
  $(OBJDIR)/CSVBusSystemTest.o \
  $(OBJDIR)/DSVReader.o \
  $(OBJDIR)/DSVWriter.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o \
  $(OBJDIR)/StringUtils.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testosm: \
  $(OBJDIR)/OpenStreetMap.o \
  $(OBJDIR)/OpenStreetMapTest.o \
  $(OBJDIR)/XMLReader.o \
  $(OBJDIR)/XMLWriter.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o \
  $(OBJDIR)/StringUtils.o
	  $(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

# https://www.cmcrossroads.com/article/basics-vpath-and-vpath
vpath %.cpp $(SRCDIR) $(TESTDIR)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
