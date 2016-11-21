TARGET = equihash
SRC = *.cpp
HEADER = *.h 
CRYPTOPPDIR = ../cryptopp563/
INC = -I. -I$(CRYPTOPPDIR) 
LDFLAGS = -L$(CRYPTOPPDIR) -lcryptopp -lpthread


$(TARGET): $(SRC) $(HEADER)
	g++ -std=c++11 -o $@ $< $(INC) $(LDFLAGS)

clean:
	rm -f $(TARGET) *~ 
