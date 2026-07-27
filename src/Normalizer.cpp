#include "../include/Normalizer.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

// To_lower()
void Normalizer::To_lower(string &link)
{
    for (char &c : link)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c - 'A' + 'a';
        }
    }
}

// read()
string Normalizer::read(string page)
{
    ifstream file(page);

    if (!file.is_open())
    {
        cout << "File not found: " << page << endl;
        return "";
    }

    string txt;

    while (getline(file, txt))
    {
        // Strip trailing comma / carriage-return (Windows line endings)
        // / stray spaces so entries match cleanly at lookup time.
        while (!txt.empty() &&
               (txt.back() == ',' || txt.back() == '\r' || txt.back() == ' '))
        {
            txt.pop_back();
        }

        if (txt.empty())
            continue;

        // Store everything lowercase since normalize() always compares
        // against lowercased URLs - otherwise case differences would
        // silently break the blocklist checks.
        To_lower(txt);

        if (page == DOMAIN_FILE)
        {
            ignoreDomain.insert(txt);
        }
        else
        {
            ignoreExtension.insert(txt);
        }
    }

    file.close();
    return txt;
}

// Normalizer()
Normalizer::Normalizer()
{
    read(DOMAIN_FILE);
    read(EXTENSION_FILE);
}

// removeFragment()
void Normalizer::removeFragment(string &source)
{
    size_t pos = source.find('#');
    if (pos == string::npos)
        return;
    source = source.substr(0, pos);
}

// normalizePath()
void Normalizer::normalizePath(string &source)
{
    // Remember whether the original path ended in '/' (a directory-style
    // URL) so we can preserve that instead of always forcing a trailing
    // slash onto every segment - including files like "/index.html".
    bool trailingSlash = !source.empty() && source.back() == '/';

    DynamicArray<string> content;
    string token;
    stringstream ss(source);
    while (getline(ss, token, '/'))
    {
        if (token.empty() || token == ".")
            continue;

        if (token == "..")
        {
            if (content.size() != 0)
                content.pop_back();
            continue;
        }
        content.push_back(token);
    }

    if (content.size() != 0)
    {
        string lastSegment = content[content.size() - 1];
        size_t pos = lastSegment.rfind('.');
        if (pos != string::npos)
        {
            string extension = lastSegment.substr(pos);
            if (ignoreExtension.exists(extension))
            {
                source = "";
                return;
            }
        }
    }

    string result = "/";
    for (int i = 0; i < content.size(); i++)
    {
        result += content[i];
        if (i != content.size() - 1 || trailingSlash)
            result += '/';
    }
    source = result;
}

// isrelative()
bool Normalizer::isrelative(string &source)
{
    size_t schemePos = source.find("://");
    if (schemePos == string::npos)
    {
        return true;
    }
    return false;
}

// normalize()
void Normalizer::normalize(string &source)
{
    
    removeFragment(source);
    if (source.empty())
        return;

    // Pull the query string out BEFORE lowercasing, since query values
    // can be case sensitive (e.g. "?token=AbC123") and must be kept
    // exactly as given. It's re-attached at the very end.
    string query;
    size_t queryPos = source.find('?');
    if (queryPos != string::npos)
    {
        query = source.substr(queryPos);
        source = source.substr(0, queryPos);
    }

    To_lower(source);

    // Relative URL
    if (isrelative(source))
    {
        relativeURL(source);
        if (source.empty())
            return;
    }

    size_t schemePos = source.find("://");
    if (schemePos == string::npos)
    {
        // Not resolvable to an absolute URL (bad link / no seed link) -
        // bail out instead of doing substr() with an out-of-range
        // position further down, which would throw.
        source = "";
        return;
    }

    string scheme = source.substr(0, schemePos);
    string remaining = source.substr(schemePos + 3);

    string authority;
    string path;

    size_t slashPos = remaining.find('/');

    if (slashPos == string::npos)
    {
        authority = remaining;
        path = "/";
    }
    else
    {
        authority = remaining.substr(0, slashPos);
        path = remaining.substr(slashPos);
    }

    // Remove default port
    size_t colonPos = authority.find(':');
    if (colonPos != string::npos)
    {
        string port = authority.substr(colonPos + 1);
        string host = authority.substr(0, colonPos);

        if ((port == "80" && scheme == "http") ||
            (port == "443" && scheme == "https"))
        {
            authority = host;
        }
    }

    // Canonicalize away the "www." prefix so that "www.example.com" and
    // "example.com" normalize to the exact same URL (this used to only
    // affect the blocklist check, not the final output).
    if (authority.rfind("www.", 0) == 0)
    {
        authority = authority.substr(4);
    }

    // Ignore domains
    if (ignoreDomain.exists(authority))
    {
        source = "";
        return;
    }

    if (authority.find('*') != string::npos)
    {
        source = "";
        return;
    }

    normalizePath(path);

    if (path.empty())
    {
        source = "";
        return;
    }

    source = scheme + "://" + authority + path + query;
}

// relative()
void Normalizer::relativeURL(string &source)
{
    if (seedLink == "empty")
    {
        cout << "Seed link is not valid or relative" << endl;
        source = "";
        return;
    }

    if (source.rfind("mailto:", 0) == 0 ||
        source.rfind("tel:", 0) == 0 ||
        source.rfind("javascript:", 0) == 0 ||
        source.rfind("data:", 0) == 0)
    {
        source = "";
        return;
    }

    // Protocol-relative URL, e.g. "//cdn.example.com/lib.js" - this
    // means "same scheme, different host", NOT a path under seedLink.
    if (source.rfind("//", 0) == 0)
    {
        size_t schemeEnd = seedLink.find("://");
        string scheme = (schemeEnd != string::npos) ? seedLink.substr(0, schemeEnd) : "http";
        source = scheme + ":" + source;
        return;
    }

    if (!source.empty() && source[0] == '/')
        source.erase(0, 1);

    source = seedLink + "/" + source;
}

// DynamicArray<string> normalize()
DynamicArray<string> Normalizer::normalize(DynamicArray<string> &source)
{
    DynamicArray<string> normalisedlink;

    for (int i = 0; i < source.size(); i++)
    {
        // Work on a local copy - the original didn't, so it mutated the
        // caller's array in place even for entries that got skipped.
        string link = source[i];

        removeFragment(link);
        if (link.empty())
            continue;

        string query;
        size_t queryPos = link.find('?');
        if (queryPos != string::npos)
        {
            query = link.substr(queryPos);
            link = link.substr(0, queryPos);
        }

        To_lower(link);

        size_t schemePos = link.find("://");
        if (schemePos == string::npos)
        {
            // No seed-link context available in bulk mode, so relative
            // links can't be resolved here - skip them.
            continue;
        }

        string scheme = link.substr(0, schemePos);
        string remaining = link.substr(schemePos + 3);

        size_t slashPos = remaining.find('/');

        string authority;
        string path;
        if (slashPos == string::npos)
        {
            authority = remaining;
            path = "/";
        }
        else
        {
            authority = remaining.substr(0, slashPos);
            path = remaining.substr(slashPos);
        }

        size_t colonPos = authority.find(':');
        if (colonPos != string::npos)
        {
            string port = authority.substr(colonPos + 1);
            string host = authority.substr(0, colonPos);

            if ((port == "80" && scheme == "http") ||
                (port == "443" && scheme == "https"))
            {
                authority = host;
            }
        }

        if (authority.rfind("www.", 0) == 0)
        {
            authority = authority.substr(4);
        }

        if (ignoreDomain.exists(authority))
            continue;

        if (authority.find('*') != string::npos)
            continue;

        normalizePath(path);
        if (path.empty())
            continue;

        normalisedlink.push_back(scheme + "://" + authority + path + query);
    }

    return normalisedlink;
}