#ifndef PPTOKENIZER_HPP
#define PPTOKENIZER_HPP

#include "IPPTokenStream.h"

// Translation features you need to implement:
// - utf8 decoder
// - utf8 encoder
// - universal-character-name decoder
// - trigraphs
// - line splicing
// - newline at eof
// - comment striping (can be part of whitespace-sequence)
// Tokenizer

struct PPTokenizer
{
	IPPTokenStream& output;

	PPTokenizer(IPPTokenStream& output);
	void process(int c);
};



#endif //PPTOKENIZER_HPP

