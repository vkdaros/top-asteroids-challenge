CXX     = g++
RM      = rm -f
CPPFLAG = -Wall -pedantic -std=c++11
LDFLAG  = -Wall

BOT = vkdrone
SRC = main.cpp VKDrone.cpp BotBase.cpp bot_interface.cpp
OBJ = $(subst .cpp,.o,$(SRC))

all: $(OBJ)
	$(CXX) $(LDFLAG) $(OBJ) -o $(BOT)

main.o: main.cpp VKDrone.h bot_interface.h
	$(CXX) $(CPPFLAG) -c $<

VKDrone.o: VKDrone.cpp VKDrone.h BotBase.h
	$(CXX) $(CPPFLAG) -c $<

BotBase.o: BotBase.cpp BotBase.h bot_interface.h
	$(CXX) $(CPPFLAG) -c $<

bot_interface.o: bot_interface.cpp bot_interface.h
	$(CXX) $(CPPFLAG) -c $<

clean:
	$(RM) $(OBJ)

