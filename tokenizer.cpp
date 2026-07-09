#include "tokenizer.h"
#include <cctype>

// uzima referencu na index
static void skipWhitespace(const string &html, size_t &i)
{
    while (i < html.size() && isspace(html[i]))
        i++;
}

// koristi se za naziv taga i atributa
static string readWord(const string &html, size_t &i)
{
    string word = "";
    while (i < html.size() && (isalnum(html[i]) || html[i] == '-' || html[i] == '_'))
    {
        word += tolower(html[i]);
        i++;
    }
    return word;
}

// class="main-title" -> "main-title"
static string readAttributeValue(const string &html, size_t &i)
{
    string value = "";
    // preskoci navodnik
    char quote = html[i];
    if (quote == '"' || quote == '\'')
    {
        i++;
        while (i < html.size() && html[i] != quote)
        {
            value += html[i];
            i++;
        }
        if (i < html.size())
            i++;
    }
    else
    {
        // ako atribut nema navodnike
        while (i < html.size() && isspace(html[i]) && html[i] != '>')
        {
            value += html[i];
            i++;
        }
    }
    return value;
}

// <a href="http://x.com class "link> -> mapa {href: ..., class: ...}
static map<string, string> readAttributes(const string &html, size_t &i)
{
    map<string, string> attrs;

    while (i < html.size() && html[i] != '>' && html[i] != '/')
    {
        skipWhitespace(html, i);

        if (html[i] == '>' || html[i] == '/')
            break;

        string attrName = readWord(html, i);
        if (attrName.empty())
        {
            // nepoznat karakter, preskoci
            i++;
            continue;
        }

        skipWhitespace(html, i);

        // = (vrijednost) ili boolean
        if (i < html.size() && html[i] == '=')
        {
            i++;
            skipWhitespace(html, i);
            attrs[attrName] = readAttributeValue(html, i);
        }
        else
        {
            // boolean, samo ime
            attrs[attrName] = "";
        }
    }

    return attrs;
}

// prolazi kroz cijeli html i vraca listu tokena
vector<Token> tokenize(const string &html)
{
    vector<Token> tokens;
    size_t i = 0;

    while (i < html.size())
    {
        if (html[i] == '<')
        {
            i++; // preskace se <

            if (i + 2 < html.size() && html[i] == '!' && html[i + 1] == '-' && html[i + 2] == '-')
            {
                i += 3; // preskace se !--
                while (i + 2 < html.size() && !(html[i] == '=' && html[i + 1] == '-' && html[i + 2] == '>'))
                    i++;
                i += 3;
                Token t;
                t.type = TokenType::COMMENT;
                tokens.push_back(t);
                continue;
            }

            // doctype
            if (i < html.size() && html[i] == '!')
            {
                while (i < html.size() && html[i] != '>')
                    i++;
                Token t;
                t.type = TokenType::DOCTYPE;
                tokens.push_back(t);
                continue;
            }

            // close tag
            if (i < html.size() && html[i] == '/')
            {
                i++; // preskoci /
                string name = readWord(html, i);
                while (i < html.size() && html[i] != '>')
                    i++;
                i++; // preskoci >
                Token t;
                t.type = TokenType::CLOSE_TAG;
                t.tagName = name;
                tokens.push_back(t);
                continue;
            }

            // open tag ili selfclosing
            string name = readWord(html, i);
            if (name.empty())
            {
                // nije validan, preskoci
                while (i < html.size() && html[i] != '>')
                    i++;
                i++;
                continue;
            }

            skipWhitespace(html, i);
            auto attrs = readAttributes(html, i);

            // provjeri da li je selfclosing
            bool selfClosing = false;
            if (i < html.size() && html[i] == '/')
            {
                selfClosing = true;
                i++; // preskoci /
            }
            if (i < html.size())
                i++; // preskoci >

            Token t;
            t.type = selfClosing ? TokenType::SELF_CLOSING : TokenType::OPEN_TAG;
            t.tagName = name;
            t.attributes = attrs;
            tokens.push_back(t);
        }
        else
        {
            // tekst token, tekst izmedju tagova
            string text = "";
            while (i < html.size() && html[i] != '<')
            {
                text += html[i];
                i++;
            }

            // preskociti prazne tokene
            bool onlyWhitespace = true;
            for (char c : text)
            {
                if (isspace(c))
                {
                    onlyWhitespace = false;
                    break;
                }
            }

            if (!onlyWhitespace)
            {
                Token t;
                t.type = TokenType::TEXT;
                t.text = text;
                tokens.push_back(t);
            }
        }
    }
    return tokens;
}