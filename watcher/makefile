HEADER_DIR := -I/opt/lampp/include
LIB_DIR := -L/opt/lampp/lib

CXXFLAGS := -O2 -w $(HEADER_DIR) $(LIB_DIR) -lmysqlclient

OBJS = daemon.o\
	func.o\
	watcher.o

TARGET = watcher

$(TARGET):$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(CXXFLAGS)

all:$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)