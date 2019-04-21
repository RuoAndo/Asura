TARGET	= asura
OBJS	= asura.o 

CC		= g++
CFLAGS		= -O2
CXXFLAGS 	+= -fpermissive 
CXXFLAGS 	+= -std=c++11
LDFLAGS		= 
INCLUDES	= -I./
LIBS		= -lm -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 

$(OBJS): timer.h 

.c.o:
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDES) -c $< 

clean:
	rm -rf $(TARGET) $(OBJS) *~

