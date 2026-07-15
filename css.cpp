#include "css.h"
#include "dom.h"
#include <algorithm>
#include <cctype>
#include <iostream>

using namespace std;

static string trim(const string &s) {
  size_t start = s.find_first_not_of(" \t\n\r");
  size_t end = s.find_last_not_of(" \t\n\r");
  if (start == string::npos)
    return "";
  return s.substr(start, end - start + 1);
}

// parsira red u mapu
map<string, string> parseProperties(const string &block) {
  map<string, string> props;
  size_t i = 0;

  while (i < block.size()) {
    string name = "";
    while (i < block.size() && block[i] != ':' && block[i] != '}')
      name += block[i++];
    name = trim(name);

    if (name.empty() || i >= block.size())
      break;

    i++; // preskace se :

    string value = "";
    while (i < block.size() && block[i] != ';' && block[i] != '}')
      value += block[i++];
    value = trim(value);

    if (!name.empty() && !value.empty())
      props[name] = value;

    if (i < block.size())
      i++;
  }
  return props;
}

// parsira inline
map<string, string> parseInlineStyle(const string &style) {
  return parseProperties(style);
}

// parsira cijeli css string
vector<CSSRule> parseCSS(const string &css) {
  vector<CSSRule> rules;
  size_t i = 0;

  while (i < css.size()) {
    // preskakanje razmaka i komentara
    while (i < css.size() && isspace(css[i]))
      i++;
    if (i >= css.size())
      break;

    // preskakanje /* */ komentara
    if (i + 1 < css.size() && css[i] == '/' && css[i + 1] == '*') {
      i += 2;
      while (i + 1 < css.size() && !(css[i] == '*' && css[i + 1] == '/'))
        i++;
      i += 2;
      continue;
    }

    // citanje selektora do {
    string selector = "";
    while (i < css.size() && css[i] != '{')
      selector += css[i++];
    selector = trim(selector);
    if (selector.empty() || i >= css.size())
      break;

    i++; // {

    // citanje tijela do {
    string body = "";
    while (i < css.size() && css[i] != '}')
      body += css[i++];
    if (i < css.size())
      i++; // }

    // selektor moze imati vise selektroa odvojenih zarezom
    // h1, h2, h3 {color : blue}
    // razdvajanje i pravljenje zasebnih
    size_t commaPos = 0;
    string remaining = selector;
    while (true) {
      commaPos = remaining.find(',');
      string single = trim(
          commaPos == string::npos ? remaining : remaining.substr(0, commaPos));
      if (!single.empty()) {
        CSSRule rule;
        rule.selector = single;
        rule.properties = parseProperties(body);
        rules.push_back(rule);
      }

      if (commaPos == string::npos)
        break;
      remaining = remaining.substr(commaPos + 1);
    }
  }
  return rules;
}

// provjeri da li selektor odgovara ovom cvoru
// za sad samo tag h1, klasa .title i id #main
static bool selectorMatches(const string &selector, const Node *node) {
  if (node->type != NodeType::ELEMENT)
    return false;

  if (selector.empty())
    return false;

  if (selector[0] == '.') {
    // klasa
    string className = selector.substr(1);
    auto it = node->attributes.find("class");
    if (it == node->attributes.end())
      return false;
    // class moze imati vise klasa, title bold, provjeriti svaku
    string classes = it->second;
    size_t pos = 0;
    while (pos < classes.size()) {
      size_t space = classes.find(' ', pos);
      string cls = classes.substr(pos, space == string::npos ? string::npos
                                                             : space - pos);
      if (cls == className)
        return true;
      if (space == string::npos)
        break;
      pos = space + 1;
    }
    return false;
  } else if (selector[0] == '#') {
    // id
    string id = selector.substr(1);
    auto it = node->attributes.find("id");
    return it != node->attributes.end() && it->second == id;
  } else {
    // tag
    return node->tagName == selector;
  }
}

Style computeStyle(const map<string, string> &props) {
  Style style;

  for (const auto &prop : props) {
    const string &name = prop.first;
    const string &value = prop.second;

    if (name == "color")
      style.color = value;
    else if (name == "background-color")
      style.bgColor = value;
    else if (name == "font-weight")
      style.fontWeight = value;
    else if (name == "display")
      style.display = value;
    else if (name == "font-size") {
      // 16px -> 16
      try {
        style.fontSize = stoi(value);
      } catch (...) {
      }
    } else if (name == "margin-top") {
      try {
        style.marginTop = stoi(value);
      } catch (...) {
      }
    } else if (name == "margin-bottom") {
      try {
        style.marginBottom = stoi(value);
      } catch (...) {
      }
    }
  }
  return style;
}

// defaultni stilovi po tagu, za browser ugradjeni
static Style defaultStyle(const string &tag) {
  Style s;
  if (tag == "h1") {
    s.fontSize = 32;
    s.fontWeight = "bold";
    s.marginTop = 8;
    s.marginBottom = 8;
  } else if (tag == "h2") {
    s.fontSize = 24;
    s.fontWeight = "bold";
    s.marginTop = 6;
    s.marginBottom = 6;
  } else if (tag == "h3") {
    s.fontSize = 18;
    s.fontWeight = "bold";
    s.marginTop = 4;
    s.marginBottom = 4;
  } else if (tag == "p") {
    s.marginTop = 8;
    s.marginBottom = 8;
  } else if (tag == "a") {
    s.color = "blue";
    s.display = "inline";
  } else if (tag == "strong" || tag == "b") {
    s.fontWeight = "bold";
    s.display = "inline";
  } else if (tag == "span")
    s.display = "inline";
  else if (tag == "li") {
    s.marginTop = 2;
    s.marginBottom = 2;
  } else if (tag == "head" || tag == "script" || tag == "style")
    s.display = "none";
  return s;
}

void applyStyles(Node *node, const vector<CSSRule> &rules) {
  if (!node)
    return;

  if (node->type == NodeType::ELEMENT) {
    // pocinje se sa defaultnim stilom za ovaj tag
    Style style = defaultStyle(node->tagName);

    // primjenjuju se css pravila koja odgovaraju
    for (const auto &rule : rules) {
      if (selectorMatches(rule.selector, node)) {
        Style ruleStyle = computeStyle(rule.properties);
        if (!rule.properties.count("color") == 0)
          style.color = ruleStyle.color;
        if (rule.properties.count("background-color"))
          style.bgColor = ruleStyle.bgColor;
        if (rule.properties.count("font-weight"))
          style.fontWeight = ruleStyle.fontWeight;
        if (rule.properties.count("font-size"))
          style.fontSize = ruleStyle.fontSize;
        if (rule.properties.count("display"))
          style.display = ruleStyle.display;
        if (rule.properties.count("margin-top"))
          style.marginTop = ruleStyle.marginTop;
        if (rule.properties.count("margin-bottom"))
          style.marginBottom = ruleStyle.marginBottom;
      }
    }

    // inline style ima najveci prioritet
    auto inlineIt = node->attributes.find("style");
    if (inlineIt != node->attributes.end()) {
      auto inlineProps = parseInlineStyle(inlineIt->second);
      Style inlineStyle = computeStyle(inlineProps);
      if (inlineProps.count("color"))
        style.color = inlineStyle.color;
      if (inlineProps.count("background-color"))
        style.bgColor = inlineStyle.bgColor;
      if (inlineProps.count("font-weight"))
        style.fontWeight = inlineStyle.fontWeight;
      if (inlineProps.count("font-size"))
        style.fontSize = inlineStyle.fontSize;
      if (inlineProps.count("display"))
        style.display = inlineStyle.display;
    }

    // cuvanje stila u atributima cvora (privremeno)
    // kasnije cu style direktno staviti u node struct
    node->attributes["_color"] = style.color;
    node->attributes["_fontSize"] = to_string(style.fontSize);
    node->attributes["_fontWeight"] = style.fontWeight;
    node->attributes["_display"] = style.display;
  }

  // rekruzivno primijeniti na svu djecu
  for (auto &child : node->children) {
    applyStyles(child.get(), rules);
  }
}
