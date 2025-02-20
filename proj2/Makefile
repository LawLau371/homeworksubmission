CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/usr/include/gtest -pthread
OBJDIR = obj
BINDIR = bin
SOURCES = StringUtils.cpp StringUtilsTest.cpp
OBJECTS = $(SOURCES:%.cpp=$(OBJDIR)/%.o)
EXEC = $(BINDIR)/teststrutils

GTEST_LIBS = -lgtest -lgtest_main -pthread

$(shell mkdir -p $(OBJDIR) $(BINDIR))

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $(EXEC) $(GTEST_LIBS)

test: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
