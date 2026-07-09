#include "dom.h"
#include "http.h"
#include "tokenizer.h"
#include <iostream>

using namespace std;

int main() {
  string url;
  cout << "Unesite URL: ";
  cin >> url;

  // skini html
  string html = httpGet(url);

  // tokenizuj
  vector<Token> tokens = tokenize(html);

  /*for (const Token &t : tokens) {
    switch (t.type) {
    case TokenType::OPEN_TAG:
      cout << "[OPEN] <" << t.tagName << ">";
      // ispisi atribute ako postoje
      for (const auto &attr : t.attributes)
        cout << " " << attr.first << "=\"" << attr.second << "\"";
      cout << "\n";
      break;
    case TokenType::CLOSE_TAG:
      cout << "[CLOSE] </" << t.tagName << ">\n";
      break;
    case TokenType::TEXT:
      cout << "[TEXT] " << t.text << "\n";
      break;
    case TokenType::SELF_CLOSING:
      cout << "[SELF] </" << t.tagName << "/>\n";
      break;
    case TokenType::COMMENT:
      cout << "[COMMENT]\n";
      break;
    case TokenType::DOCTYPE:
      cout << "[DOCTYPE]\n";
      break;
    }
  }

  cout << "\nUkupno tokena: " << tokens.size() << "\n";*/

  unique_ptr<Node> dom = buildDOM(tokens);

  printDOM(dom.get());

  return 0;
}
