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
  $(SRCDIR)/CDSVReader.cpp \
  $(SRCDIR)/CDSVWriter.cpp \
  $(SRCDIR)/CXMLReader.cpp  \
  $(SRCDIR)/CXMLWriter.cpp  \
  $(SRCDIR)/StringDataSource.cpp \
  $(SRCDIR)/StringDataSink.cpp \
  $(SRCDIR)/StringUtils.cpp

# test
TESTSRCS = \
  $(TESTDIR)/StringDataSourceTest.cpp \
  $(TESTDIR)/StringDataSinkTest.cpp \
  $(TESTDIR)/StringUtilsTest.cpp \
  $(TESTDIR)/testdsv.cpp \
  $(TESTDIR)/testxml.cpp

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
TESTOBJS = $(patsubst $(TESTDIR)/%.cpp, $(OBJDIR)/%.test.o, $(TESTSRCS))

TEST_EXECS = \
  $(BINDIR)/teststrdatasource \
  $(BINDIR)/teststrdatasink \
  $(BINDIR)/teststrutils \
  $(BINDIR)/testdsv \
  $(BINDIR)/testxml

.PHONY: all clean

all: $(TEST_EXECS)


$(BINDIR)/teststrdatasource: \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSourceTest.test.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ \
	  $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/teststrdatasink: \
  $(OBJDIR)/StringDataSink.o \
  $(OBJDIR)/StringDataSinkTest.test.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ \
	  $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/teststrutils: \
  $(OBJDIR)/StringUtils.o \
  $(OBJDIR)/StringUtilsTest.test.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ \
	  $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testdsv: \
  $(OBJDIR)/CDSVReader.o \
  $(OBJDIR)/CDSVWriter.o \
  $(OBJDIR)/testdsv.test.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ \
	  $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)

$(BINDIR)/testxml: \
  $(OBJDIR)/CXMLReader.o \
  $(OBJDIR)/CXMLWriter.o \
  $(OBJDIR)/testxml.test.o \
  $(OBJDIR)/StringDataSource.o \
  $(OBJDIR)/StringDataSink.o
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -o $@ $^ \
	  $(GTEST_LDFLAGS) $(EXPAT_LDFLAGS)


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -c $< -o $@


$(OBJDIR)/%.test.o: $(TESTDIR)/%.cpp
	@mkdir -p $(OBJDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -I$(GTEST_INC) -I$(TESTDIR) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

