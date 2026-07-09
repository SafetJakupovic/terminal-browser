#include "dom.h"
#include "tokenizer.h"
#include <iostream>
#include <memory>

using namespace std;

unique_ptr<Node> Node::makeElement(const string &tag,
                                   const map<string, string> &attrs) {
  auto node = make_unique<Node>();
  node->type = NodeType::ELEMENT;
  node->tagName = tag;
  node->attributes = attrs;
  return node;
}

unique_ptr<Node> Node::makeText(const string &text) {
  auto node = make_unique<Node>();
  node->type = NodeType::TEXT;
  node->text = text;
  return node;
}

unique_ptr<Node> Node::makeDocument() {
  auto node = make_unique<Node>();
  node->type = NodeType::DOCUMENT;
  node->tagName = "#document";
  return node;
}

static bool isSelfClosingTag(const string &tag) {
  static const vector<string> selfClosing = {
      "area",  "base", "br",   "col",   "embed",  "hr",    "img",
      "input", "link", "meta", "param", "source", "track", "wbr"};
  for (const auto &t : selfClosing)
    if (t == tag)
      return true;
  return false;
}

unique_ptr<Node> buildDOM(const vector<Token> &tokens) {
  auto document = Node::makeDocument();

  // stek pokazivaca na cvorove
  // na vrhu je uvijek trenutni aktivni roditelj
  vector<Node *> stack;
  stack.push_back(document.get()); // vraca pokazivac iz unique_ptr

  for (const Token &token : tokens) {
    // trenutni roditelj je uvijek vrh steka
    Node *current = stack.back();

    if (token.type == TokenType::OPEN_TAG) {
      auto newNode = Node::makeElement(token.tagName, token.attributes);
      newNode->parent = current;

      Node *newNodePtr = newNode.get();
      current->children.push_back(std::move(newNode));

      if (!isSelfClosingTag(token.tagName))
        stack.push_back(
            newNodePtr); // postaje novi aktivni roditelj ako nije self closing
    } else if (token.type == TokenType::CLOSE_TAG) {
      // zatvara najblizi otvoreni tag
      for (int i = stack.size() - 1; i >= 1; i--) {
        if (stack[i]->tagName == token.tagName) {
          stack.erase(stack.begin() + i, stack.end());
          break;
        }
      }
    } else if (token.type == TokenType::SELF_CLOSING) {
      auto newNode = Node::makeElement(token.tagName, token.attributes);
      newNode->parent = current;
      current->children.push_back(std::move(newNode));
    } else if (token.type == TokenType::TEXT) {
      auto textNode = Node::makeText(token.text);
      textNode->parent = current;
      current->children.push_back(std::move(textNode));
    }
    // DOCTYPE i COMMENT kasnije
  }

  return document;
}

void printDOM(const Node *node, int depth) {
  if (!node)
    return;

  string indent(depth * 2, ' ');

  if (node->type == NodeType::DOCUMENT) {
    cout << indent << "[DOCUMENT]\n";
  } else if (node->type == NodeType::ELEMENT) {
    cout << indent << "<" << node->tagName;
    for (const auto &attr : node->attributes)
      cout << " " << attr.first << "=\"" << attr.second << "\"";
    cout << ">\n";
  } else if (node->type == NodeType::TEXT) {
    string display = node->text;
    if (display.size() > 50)
      display = display.substr(0, 50) + "...";
    cout << indent << "\"" << display << "\"\n";
  }

  for (const auto &child : node->children)
    printDOM(child.get(), depth + 1);
}
