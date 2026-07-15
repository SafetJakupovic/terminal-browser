#pragma once
#include "dom.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

struct Style {
  string color = "black";
  string bgColor = "white";
  string fontWeight = "normal";
  string display = "block";
  int fontSize = 16;
  int marginTop = 0;
  int marginBottom = 0;
};

// npr. h1 { color: blue, font-size: 24px }
struct CSSRule {
  string selector;                // h1
  map<string, string> properties; // color -> blue
};

// parsira css string i vraca listu pravila
vector<CSSRule> parseCSS(const string &css);

// parsira inline style string i vraca mapu osobina
map<string, string> parseInlineStyle(const string &style);

// primijeni css pravila na cijelo dom stablo
// svaki node dobija svoj style objekat
void applyStyles(Node *node, const vector<CSSRule> &rules);

// pretvori css string vrijednost u style objekat
Style computeStyle(const map<string, string> &properties);
