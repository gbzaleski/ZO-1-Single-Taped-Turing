all: tm_interpreter tm_translator

tm_interpreter: tm_interpreter.cpp turing_machine.cpp turing_machine.h tm_convert.cpp tm_convert.h
	g++ -Wall -Wshadow $(filter %.cpp,$^) -o $@

tm_translator: tm_translator.cpp turing_machine.cpp turing_machine.h tm_convert.cpp tm_convert.h
	g++ -Wall -Wshadow $(filter %.cpp,$^) -o $@

clean:
	rm -rf tm_translator tm_interpreter *~
