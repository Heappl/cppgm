# cppgm starter makefile

apps = \
	pptoken \
	posttoken

units = \
	Helpers \
	PPTokenizer \
	RegexRule \
	StandardData \
	StateMachine \
	PostTokenAnalyser

all: $(apps)

CXXFLAGS = -MD -g -O3 -std=gnu++11

clean:
	-rm $(apps) *.o *.d

$(apps): %: %.o $(units:=.o)
	g++ -g -O2 -std=gnu++11 $^ -o $@

-include $(units:=.d) $(apps:=.d)

