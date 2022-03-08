#include "json_output.h"
#include "compilation_context.h"
#include <map>
#include <stdio.h>

using namespace std;

static map<string, string> content;

void escapeJSON(const char * input, string & output)
{
  output.clear();
  output.reserve(64);

  int i = 0;
  while (input[i])
  {
    switch (input[i])
    {
      case '"':
        output += "\\\"";
        break;
      case '/':
        output += "\\/";
        break;
      case '\b':
        output += "\\b";
        break;
      case '\f':
        output += "\\f";
        break;
      case '\n':
        output += "\\n";
        break;
      case '\r':
        output += "\\r";
        break;
      case '\t':
        output += "\\t";
        break;
      case '\\':
        output += "\\\\";
        break;
      default:
        output += input[i];
        break;
    }

    i++;
  }
}

static string escapedStr;
static char buf[256 * 1024];
static char tokstr[64];

static void stringify_token(Token & tok)
{
  const char * ptr = tokstr;
  const char * quote = "";
  if (tok.type == TK_INTEGER)
    snprintf(tokstr, 64, "%llu", (long long unsigned int)tok.u.i);
  else if (tok.type == TK_FLOAT)
    snprintf(tokstr, 64, "%g", tok.u.d);
  else if (tok.type == TK_STRING_LITERAL)
  {
    escapeJSON(tok.u.s, escapedStr);
    ptr = escapedStr.c_str();
    quote = "\"";
  }
  else
  {
    ptr = tok.u.s;
    quote = "\"";
  }

  snprintf(buf, sizeof(buf) - 1, "\"tt\":\"%s\",\"val\":%s%s%s,\"line\":%d,\"col\":%d",
    token_type_names[int(tok.type)], quote, ptr, quote, tok.line, tok.column);
}


bool get_tokens_as_string(Lexer & lexer, string & s)
{
  s.reserve(16384);
  s += "\"tokens\":[";

  bool first = true;
  for (Token & tok : lexer.tokens)
  {
    if (!first)
      s += ",";

    stringify_token(tok);
    s += "\n{";
    s += buf;
    s += "}";
    first = false;

    if (tok.type == TK_EOF)
      break;
  }

  s += "]";
  return true;
}


string get_node_as_string(Node * node)
{
  if (!node)
    return string("null");

  string res;
  res.reserve(128);
  snprintf(buf, sizeof(buf) - 1, "\n{\"nt\":\"%s\",", node_type_names[node->nodeType]);
  res += buf;
  stringify_token(node->tok);
  res += buf;

  if (!node->children.empty())
  {
    res += ",\"children\":[";
    for (size_t i = 0; i < node->children.size(); i++)
    {
      if (i > 0)
        res += ",";
      res += get_node_as_string(node->children[i]);
    }
    res += "]";
  }

  res += "}";
  return res;
}

static void append_content(const char * file_name, const string & s)
{
  auto it = content.find(file_name);
  if (it == content.end())
    content.insert(make_pair(file_name, string("{\n") + s));
  else
  {
    it->second += ",\n";
    it->second += s;
  }
}

bool tokens_to_json(const char * file_name, Lexer & lexer)
{
  string s;
  if (!get_tokens_as_string(lexer, s))
    return false;

  append_content(file_name, s);
  return true;
}


bool ast_to_json(const char * file_name, Node * node)
{
  string s("\"root\":");
  s += get_node_as_string(node);

  append_content(file_name, s);
  return true;
}


bool compiler_messages_to_json(const char * file_name)
{
  string s("\"messages\":[");
  bool first = true;
  char txt[2048] = { 0 };

  for (CompilerMessage & cm : CompilationContext::compilerMessages)
  {
    if (!first)
      s += ",";

    string escapedMsg;
    string escapedFile;
    escapeJSON(cm.message.c_str(), escapedMsg);
    escapeJSON(cm.fileName.c_str(), escapedFile);

    snprintf(txt, sizeof(txt) - 1,
      "\n{\"line\":%d,\"col\":%d,\"len\":4,\"file\":\"%s\",\"intId\":%d,\"textId\":\"%s\",\"message\":\"%s\",\"isError\":%s}",
      cm.line, cm.column, escapedFile.c_str(), cm.intId, cm.textId, escapedMsg.c_str(), cm.isError ? "true" : "false");

    s += txt;
    first = false;
  }
  s += "]";

  append_content(file_name, s);

  return true;
}

bool json_write_files()
{
  bool res = true;
  for (auto & it : content)
  {
    const char * fileName = it.first.c_str();
    if (*fileName)
    {
      FILE * f = fopen(fileName, "wt");
      if (!f)
      {
        CompilationContext::setErrorLevel(ERRORLEVEL_FATAL);
        fprintf(out_stream, "ERROR: json_write_files(): cannot write to file '%s'\n", fileName);
        res = false;
        continue;
      }

      fprintf(f, "%s\n}", it.second.c_str());
      fclose(f);
    }
    else
    {
      fprintf(out_stream, "%s\n}", it.second.c_str());
    }
  }

  return res;
}
