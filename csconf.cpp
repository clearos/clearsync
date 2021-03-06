// ClearSync: system synchronization daemon.
// Copyright (C) 2011-2012 ClearFoundation <http://www.clearfoundation.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <expat.h>
#include <regex.h>
#include <pwd.h>
#include <grp.h>

#include <clearsync/csexception.h>
#include <clearsync/csconf.h>
#include <clearsync/cslog.h>
#include <clearsync/csutil.h>

csXmlTag::csXmlTag(const char *name, const char **attr)
    : name(name), text(""), data(NULL)
{
    for (int i = 0; attr[i]; i += 2)
        param[attr[i]] = attr[i + 1];
}

bool csXmlTag::ParamExists(const string &key)
{
    map<string, string>::iterator i;
    i = param.find(key);
    return (bool)(i != param.end());
}

string csXmlTag::GetParamValue(const string &key)
{
    map<string, string>::iterator i;
    i = param.find(key);
    if (i == param.end())
        throw csXmlKeyNotFound(key.c_str());
    return i->second;
}

bool csXmlTag::operator==(const char *tag)
{
    if (!strcasecmp(tag, name.c_str())) return true;
    return false;
}

bool csXmlTag::operator!=(const char *tag)
{
    if (!strcasecmp(tag, name.c_str())) return false;
    return true;
}

static void csXmlElementOpen(
    void *data, const char *element, const char **attr)
{
    csXmlParser *csp = static_cast<csXmlParser *>(data);

    csXmlTag *tag = new csXmlTag(element, attr);
#ifdef _CS_DEBUG
    csLog::Log(csLog::Debug, "Element open: %s", tag->GetName().c_str());
#endif
    csp->ParseElementOpen(tag);
    csp->stack.push_back(tag);
}

static void csXmlElementClose(void *data, const char *element)
{
    csXmlParser *csp = static_cast<csXmlParser *>(data);

    csXmlTag *tag = csp->stack.back();
#ifdef _CS_DEBUG
    csLog::Log(csLog::Debug, "Element close: %s", tag->GetName().c_str());
#endif
#if 0
    string text = tag->GetText();
    if (text.size()) {
        csLog::Log(csLog::Debug, "Text[%d]:", text.size());
        //csHexDump(stderr, text.c_str(), text.size());
    }
#endif
    csp->stack.pop_back();
    csp->ParseElementClose(tag);
    delete tag;
}

static void csXmlText(void *data, const char *txt, int length)
{
    if (length == 0) return;

    csXmlParser *csp = static_cast<csXmlParser *>(data);

    csXmlTag *tag = csp->stack.back();
    string text = tag->GetText();
    for (int i = 0; i < length; i++) {
        if (txt[i] == '\n' || txt[i] == '\r' ||
            !isprint(txt[i])) continue;
        text.append(1, txt[i]);
    }
    tag->SetText(text);
}

csXmlParser::csXmlParser(void)
    : p(NULL), conf(NULL), fh(NULL), buffer(NULL)
{
    Reset();
}

csXmlParser::~csXmlParser()
{
    Reset();
    if (p != NULL) XML_ParserFree(p);
    if (buffer != NULL) delete [] buffer;
}

void csXmlParser::Reset(void)
{
    if (p != NULL) XML_ParserFree(p);
    p = XML_ParserCreate(NULL);
    XML_SetUserData(p, (void *)this);
    XML_SetElementHandler(p, csXmlElementOpen, csXmlElementClose);
    XML_SetCharacterDataHandler(p, csXmlText);

    if (buffer != NULL) delete [] buffer;
    page_size = ::csGetPageSize();
    buffer = new uint8_t[page_size];

    for (vector<csXmlTag *>::iterator i = stack.begin();
        i != stack.end(); i++) delete (*i);
}

void csXmlParser::Parse(const char *filename)
{
    Reset();

    if (conf == NULL)
        throw csException(EINVAL, "Configuration not set.");

    if (filename == NULL) filename = conf->GetFilename();
    if (!(fh = fopen(filename, "r")))
        throw csException(errno, filename);

    for (;;) {
        size_t length;
        length = fread(buffer, 1, page_size, fh);
        if (ferror(fh))
            throw csException(errno, filename);
        int done = feof(fh);

        if (!XML_Parse(p, (const char *)buffer, length, done))
            ParseError(XML_ErrorString(XML_GetErrorCode(p)));
        if (done) break;
    }

    fclose(fh);
    fh = NULL;
}

void csXmlParser::ParseError(const string &what)
{
    throw csXmlParseException(
        what.c_str(),
        XML_GetCurrentLineNumber(p),
        XML_GetCurrentColumnNumber(p),
        buffer[XML_GetCurrentByteIndex(p)]);
}

csConf::csConf(const char *filename, csXmlParser *parser,
    int argc, char *argv[])
    : filename(filename), parser(parser), argc(argc), argv(argv)
{
}

csConf::~csConf()
{
    if (parser != NULL) delete parser;
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
