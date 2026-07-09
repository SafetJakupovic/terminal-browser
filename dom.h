#pragma once
#include "tokenizer.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

enum class NodeType {
  DOCUMENT, // korijen stabla, cijeli dokument
  ELEMENT,  // html tag
  TEXT      // obican tekst
};

// jedan cvor u DOM stablu
struct Node {
  NodeType type;
  string tagName;
  string text;
  map<string, string> attributes;

  // node posjeduje svoje djecu, kad umre node umiru i djeca
  vector<unique_ptr<Node>> children;

  // nije unique_ptr jer roditelj posjeduje dijete, ne obrnuto
  Node *parent = nullptr;

  static unique_ptr<Node> makeElement(const string &tag,
                                      const map<string, string> &attrs);
  static unique_ptr<Node> makeText(const string &text);
  static unique_ptr<Node> makeDocument();
};

unique_ptr<Node> buildDOM(const vector<Token> &tokens);

void printDOM(const Node *node, int depth = 0);
