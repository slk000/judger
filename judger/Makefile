HEADER_DIR := -I/opt/lampp/include
LIB_DIR := -L/opt/lampp/lib

CXXFLAGS := -O2 -w $(HEADER_DIR) $(LIB_DIR) -lmysqlclient

OBJS = ../db.o\
	../consts.o\
	judger.o

TARGET = judger

$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(CXXFLAGS)

all:$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)