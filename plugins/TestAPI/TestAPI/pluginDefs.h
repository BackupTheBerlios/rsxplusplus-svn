//rip from DCPlusPlus.h
#ifndef PLUGIN_DEFS_H
#define PLUGIN_DEFS_H

#ifdef _WIN32
# define snprintf _snprintf
# define snwprintf _snwprintf
#endif

// Make sure we're using the templates from algorithm...
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
/*
typedef vector<wstring> WStringList;
typedef WStringList::iterator WStringIter;
typedef WStringList::const_iterator WStringIterC;

typedef pair<wstring, wstring> WStringPair;
typedef vector<WStringPair> WStringPairList;
typedef WStringPairList::iterator WStringPairIter;

typedef std::map<wstring, wstring> WStringMap;
typedef WStringMap::iterator WStringMapIter;
*/
#if defined(_MSC_VER)
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%I64d"
#define U64_FMT "%I64d"
#endif

#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#endif //PLUGIN_DEFS_H