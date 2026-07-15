#include "css.h"
#include "dom.h"
#include "http.h"
#include "tokenizer.h"
#include <iostream>

using namespace std;

string extractCSS(const Node *node) {
  if (!node)
    return "";
  string css = "";

  if (node->type == NodeType::ELEMENT && node->tagName == "style") {
    for (const auto &child : node->children) {
      if (child->type == NodeType::TEXT)
        css += child->text;
    }
  }

  for (const auto &child : node->children)
    css += extractCSS(child.get());

  return css;
}

int main() {
  string url;
  cout << "Unesite URL: ";
  cin >> url;

  string html = httpGet(url);
  vector<Token> tokens = tokenize(html);
  auto dom = buildDOM(tokens);

  string cssText = extractCSS(dom.get());
  auto cssRules = parseCSS(cssText);

  cout << "Pronadjeno CSS pravila: " << cssRules.size() << "\n";
  for (const auto &rule : cssRules) {
    cout << " " << rule.selector << " { ";
    for (const auto &prop : rule.properties)
      cout << prop.first << ": " << prop.second << "; ";
    cout << "}\n";
  }

  applyStyles(dom.get(), cssRules);

  printDOM(dom.get());

  return 0;
}
