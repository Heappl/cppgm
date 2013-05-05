#ifndef PPTOKENIZER_HPP
#define PPTOKENIZER_HPP

#include "IPPTokenStream.h"
#include <memory>

// Translation features you need to implement:
// - utf8 decoder
// - utf8 encoder
// - universal-character-name decoder
// - trigraphs
// - line splicing
// - newline at eof
// - comment striping (can be part of whitespace-sequence)
// Tokenizer

class PPTokenizer
{
    struct PPTokenizerImpl;

	std::shared_ptr<IPPTokenStream> output;
    std::shared_ptr<PPTokenizerImpl> impl;

public:
	PPTokenizer(std::shared_ptr<IPPTokenStream> output);
	void process(uint64_t c);
};



#endif //PPTOKENIZER_HPP

