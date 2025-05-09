#ifndef STRING_H
#define STRING_H

// NOTE(acol): To be used as immutable
typedef struct string8 {
    u8 *Str;
    u64 Size;
} string8;

typedef struct string8_node {
    struct string8_node *Next;
    string8 String;
} string8_node;

typedef struct string8_list {
    struct string8_node *First;
    struct string8_node *Last;
    u64 NodeCount;
    u64 TotalSize;
} string8_list;

typedef struct string8_array {
    string8 *V;
    u64 Count;
} string8_array;

typedef struct string8_join {
    string8 Pre;
    string8 Sep;
    string8 Post;
} string8_join;

#define String8Lit(S) (string8){(u8 *)(S), sizeof(S) - 1}
#define String8Typed(S) \
    (string8) { (u8 *)(S), sizeof(*(S)) }

static u64 Cstring8Length(char *Cstr);
static string8 String8FromCstring8(char *Cstr);
static string8 String8FromRange(u8 *First, u8 *Last);

static string8 String8Prefix(string8 String, u64 Size);
static string8 String8Postfix(string8 String, u64 Size);
static string8 String8Chop(string8 String, u64 Ammount);
static string8 String8Skip(string8 String, u64 Ammount);
static string8 String8SubstringWindow(string8 String, u64 First, u64 Last);

static string8 PushString8Copy(arena *Arena, string8 String);
static string8 PushString8Cat(arena *Arena, string8 String1, string8 String2);
static string8 PushString8fv(arena *Arena, char *Fmt, va_list Args);
static string8 PushString8f(arena *Arena, char *Fmt, ...);

// TODO(acol): the list stuff I could add a lot more with pushing in the middle or front and blah blah
static string8_node *String8ListPushNode(string8_list *List, string8 String, string8_node *Node);
static string8_node *String8ListPush(arena *Arena, string8_list *List, string8 String);
static string8_node *String8ListPushf(arena *Arena, string8_list *List, char *Fmt, ...);

static string8 String8ListJoin(arena *Arena, string8_list *List, string8_join *OptionalStringJoin);
static string8_list String8Split(arena *Arena, string8 String, u8 *SplitCharacters, u32 Length);

#endif  // STRING_H
