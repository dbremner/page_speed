/*
 * This file contains a C++ port of jsmin.c. The copyright notice
 * below is the copyright notice from jsmin.c.
 */

/* jsmin.c
   2008-08-03

Copyright (c) 2002 Douglas Crockford  (www.crockford.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "third_party/jsmin/cpp/jsmin.h"

#include <stdlib.h>
#include <stdio.h>

#include "base/logging.h"

/* isAlphanum -- return true if the character is a letter, digit, underscore,
        dollar sign, or non-ASCII character.
*/

namespace {

int
isAlphanum(int c)
{
    return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') || c == '_' || c == '$' || c == '\\' ||
        c > 126);
}

class StringConsumer {
 public:
  void push_back(int character) {
    output_.push_back(character);
  }

  void append(const std::string& str) {
    output_.append(str);
  }

  std::string output_;
};

class SizeConsumer {
 public:
  SizeConsumer() : size_(0) {
  }

  void push_back(int character) {
    ++size_;
  }

  void append(const std::string& str) {
    size_ += str.size();
  }

  int size_;
};

/**
 * jsmin::Minifier is a C++ port of jsmin.c. Minifier uses member
 * variables instead of the static globals used by jsmin.c. Minifier
 * also uses strings to read input and write output, where jsmin.c
 * uses stdin/stdout.
 */
template<typename OutputConsumer>
class Minifier {
 public:

  /**
   * Construct a new Minifier instance that minifies the specified
   * JavaScript input.
   */
  explicit Minifier(const std::string* input);
  virtual ~Minifier();

  /**
   * @return a pointer to an OutputConsumer instance if minification
   * was successful, NULL otherwise.
   */
  OutputConsumer* GetOutput();

 private:
  void WriteCccIfAny();
  // The various methods from jsmin.c, ported to this class.
  int get(bool translate);
  int peek();
  int next();
  void action(int d);
  void jsmin();

  // action 1 - Output A. Copy B to A. Get the next B.
  void AdvanceAndOutputA();

  // action 2 - Copy B to A. Get the next B. (Delete A).
  void AdvanceAndDeleteA();

  // action 3 - Get the next B. (Delete B).
  void DeleteB();

  // Data members from jsmin.c, ported to this class.
  int theA;
  int theB;
  int theLookahead;

  // Our custom data members, not from jsmin.c
  const std::string *input_;
  int input_index_;
  OutputConsumer output_consumer_;
  std::string ccc_contents_; // the last Conditional Compilation Comment
  bool error_;
};


template<typename OutputConsumer>
Minifier<OutputConsumer>::Minifier(const std::string *input)
  : theA(-1),
    theB(-1),
    theLookahead(EOF),
    input_(input),
    input_index_(0),
    error_(false) {
}

template<typename OutputConsumer>
Minifier<OutputConsumer>::~Minifier() {}

template<typename OutputConsumer>
OutputConsumer*
Minifier<OutputConsumer>::GetOutput()
{
    jsmin();
    if (!error_) {
        return &output_consumer_;
    }
    return NULL;
}

// If there's currently a Conditional Compilation Comment in the buffer, write
// it to the output.
template<typename OutputConsumer>
void Minifier<OutputConsumer>::WriteCccIfAny() {
  if (!ccc_contents_.empty()) {
    output_consumer_.append("/*");
    output_consumer_.append(ccc_contents_);
    output_consumer_.append("*/");
    ccc_contents_.clear();
  }
}


/* get -- return the next character from the input. Watch out for lookahead. If
        the character is a control character, translate it to a space or
        linefeed.
*/
template<typename OutputConsumer>
int
Minifier<OutputConsumer>::get(bool translate)
{
    if (theLookahead == EOF) {
        if (input_index_ < input_->length()) {
          int c = (0xff & input_->at(input_index_++));
          if (translate) {
            if (c >= ' ' || c == '\n' || c == EOF) {
                return c;
            }
            if (c == '\r') {
                return '\n';
            }
            return ' ';
          } else {
            return c;
          }
        } else {
            return EOF;
        }
    } else {
      int c = theLookahead;
      theLookahead = EOF;
      return c;
    }
}


/* peek -- get the next character without getting it.
*/

template<typename OutputConsumer>
int
Minifier<OutputConsumer>::peek()
{
    theLookahead = get(true);
    return theLookahead;
}

/* next -- get the next character, excluding comments. peek() is used to see
        if a '/' is followed by a '/' or '*'.
*/
// If next() sees a comment, it'll skip over it and return ' '.  If the comment
// it sees is a Conditional Compilation Comment, it'll store it in the
// ccc_contents_ buffer.  CCC's should be sent to the output, but we can't send
// them right away in next() because that might cause reordering.  Instead, we
// call WriteCccIfAny() at certain times to flush the ccc_contents_ buffer to
// the output.  [-mdsteele]
template<typename OutputConsumer>
int
Minifier<OutputConsumer>::next()
{
    int c = get(true);
    if  (c == '/') {
        switch (peek()) {
        case '/':
            for (;;) {
                c = get(true);
                if (c <= '\n') {
                    return c;
                }
            }
        case '*':
            get(true);  // get the star we just peeked
            for (;;) {
                const int c2 = get(false);
                switch (c2) {
                case '*':
                    if (peek() == '/') {
                      get(true);  // get the slash we just peeked
                      // If this looks like an IE conditional compilation
                      // comment, leave it in.
                      if (!(ccc_contents_.size() >= 2 &&
                            ccc_contents_[0] == '@' &&
                            ccc_contents_[ccc_contents_.size() - 1] == '@')) {
                        ccc_contents_.clear();
                      }
                      return ' ';
                    } else {
                      ccc_contents_.push_back(static_cast<char>(c2));
                    }
                    break;
                case EOF:
                    LOG(WARNING) << "Error: JSMIN Unterminated comment.";
                    error_ = true;
                    return EOF;
                default:
                    ccc_contents_.push_back(static_cast<char>(c2));
                }
            }
        default:
            return c;
        }
    }
    return c;
}

template<typename OutputConsumer>
void Minifier<OutputConsumer>::AdvanceAndOutputA() {
  output_consumer_.push_back(theA);
  AdvanceAndDeleteA();
}

template<typename OutputConsumer>
void Minifier<OutputConsumer>::AdvanceAndDeleteA() {
  theA = theB;
  WriteCccIfAny();
  if (theA == '\'' || theA == '"') {
    for (;;) {
      output_consumer_.push_back(theA);
      theA = get(false);
      if (theA == theB) {
        break;
      }
      if (theA == '\\') {
        output_consumer_.push_back(theA);
        theA = get(false);
      }
      if (theA == EOF) {
        LOG(WARNING) << "Error: JSMIN unterminated string literal.";
        error_ = true;
        return;
      }
    }
  }
  DeleteB();
}

template<typename OutputConsumer>
void Minifier<OutputConsumer>::DeleteB() {
  theB = next();
  if (error_) {
    return;
  }
  if (theB == '/' && (theA == '(' || theA == ',' || theA == '=' ||
                      theA == ':' || theA == '[' || theA == '!' ||
                      theA == '&' || theA == '|' || theA == '?' ||
                      theA == '{' || theA == '}' || theA == ';' ||
                      theA == '\n')) {
    output_consumer_.push_back(theA);
    WriteCccIfAny();
    output_consumer_.push_back(theB);
    for (;;) {
      theA = get(true);
      if (theA == '/') {
        break;
      }
      if (theA =='\\') {
        output_consumer_.push_back(theA);
        theA = get(true);
      }
      if (theA == EOF) {
        LOG(WARNING) << "Error: JSMIN unterminated "
                     << "Regular Expression literal.\n";
        error_ = true;
        return;
      }
      output_consumer_.push_back(theA);
    }
    theB = next();
  }
}

/* jsmin -- Copy the input to the output, deleting the characters which are
        insignificant to JavaScript. Comments will be removed. Tabs will be
        replaced with spaces. Carriage returns will be replaced with linefeeds.
        Most spaces and linefeeds will be removed.
*/

template<typename OutputConsumer>
void
Minifier<OutputConsumer>::jsmin()
{
    DeleteB();
    AdvanceAndDeleteA();
    if (error_) {
        return;
    }
    while (theA != EOF) {
        switch (theA) {
        case ' ':
            if (isAlphanum(theB)) {
                AdvanceAndOutputA();
            } else {
                AdvanceAndDeleteA();
            }
            break;
        case '\n':
            switch (theB) {
            case '{':
            case '[':
            case '(':
            case '+':
            case '-':
                AdvanceAndOutputA();
                break;
            case ' ':
                DeleteB();
                break;
            default:
                if (isAlphanum(theB)) {
                    AdvanceAndOutputA();
                } else {
                    AdvanceAndDeleteA();
                }
            }
            break;
        default:
            switch (theB) {
            case ' ':
                if (isAlphanum(theA)) {
                    AdvanceAndOutputA();
                    break;
                }
                DeleteB();
                break;
            case '\n':
                switch (theA) {
                case '}':
                case ']':
                case ')':
                case '+':
                case '-':
                case '"':
                case '\'':
                    AdvanceAndOutputA();
                    break;
                default:
                    if (isAlphanum(theA)) {
                        AdvanceAndOutputA();
                    } else {
                        DeleteB();
                    }
                }
                break;
            default:
                AdvanceAndOutputA();
                break;
            }
        }
    }
}

}  // namespace

namespace jsmin {

bool MinifyJs(const std::string& input, std::string* out) {
  Minifier<StringConsumer> minifier(&input);
  StringConsumer* output = minifier.GetOutput();
  if (output) {
    *out = output->output_;
    return true;
  } else {
    return false;
  }
}

bool GetMinifiedJsSize(const std::string& input, int* minimized_size) {
  Minifier<SizeConsumer> minifier(&input);
  SizeConsumer* output = minifier.GetOutput();
  if (output) {
    *minimized_size = output->size_;
    return true;
  } else {
    return false;
  }
}

}  // namespace jsmin
