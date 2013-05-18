# cppgm starter makefile

apps = \
	pptoken \
	posttoken \
    ctrlexpr

units = \
	Helpers \
	PPTokenizer \
	RegexRule \
	StandardData \
	StateMachine \
	StringLiteralsPostTokenProcessor \
	PostTokenAnalyser \
    ControlExpressionEvaluator 

all: $(apps)

CXXFLAGS = -MD -g -O3 -std=gnu++11

clean:
	-rm $(apps) *.o *.d

$(apps): %: %.o $(units:=.o)
	g++ -g -O2 -std=gnu++11 $^ -o $@

-include $(units:=.d) $(apps:=.d)

