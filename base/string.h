#ifndef STRING_H
#define STRING_H

/*  =====================================================================================

        NOTE(acol): String helpers, not optimized at all this is just convenience

    ===================================================================================== */

// NOTE(acol): To be used as immutable
typedef struct string {
    u8 *Str = 0;
    u64 Size = 0;
} string;

typedef struct string_node {
    struct string_node *Next;
    string String;
} string_node;

typedef struct string_list {
    struct string_node *First = 0;
    struct string_node *Last = 0;
    u64 NodeCount = 0;
    u64 TotalSize = 0;
} string_list;

typedef struct string_array {
    string *V;
    u64 Count;
} string_array;

typedef struct string_join {
    string Pre;
    string Mid;
    string Post;
} string_join;

#define StringLit(S) (string){(u8 *)(S), sizeof(S) - 1}
#define StringTyped(S) \
    (string) { (u8 *)(S), sizeof(*(S)) }

static u64 CstringLength(u8 *Cstr);
static string StringFromCstring(char *Cstr);
static string StringPrefix(string String, u64 Size);
static string StringPostfix(string String, u64 Size);
static string StringChop(string String, u64 Ammount);
static string StringSkip(string String, u64 Ammount);
static string StringSubstringWindow(string String, u64 First, u64 Last);
static string StringSubstringSize(string String, u64 First, u64 Size);

static void StringListPushExplicit(string_list *List, string String, string_node *Node);
static void StringListPush(arena *Arena, string_list *List, string String);

static string StringJoin(arena *Arena, string_list *List, string_join *StringJoin = 0);
static string_list StringSplit(arena *Arena, string String, u8 *SplitCharacters, u32 Length);
static string StringPushfv(arena *Arena, char *Fmt, va_list Args);
static string StringPushf(arena *Arena, char *Fmt, ...);
static void StringListPushf(arena *Arena, string_list *List, char *Fmt, ...);

#endif  // STRING_H
