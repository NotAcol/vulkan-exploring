AsanDisable static u64 CstringSize(char *Cstr) {
    u64 Size = 0;
    for (;;) {
        if (*Cstr == '\0') return Size;
        if ((IntFromPointer(Cstr) & 3) == 0) break;
        Cstr++;
        Size++;
    }

    for (;;) {
        u32 Val = *(u32 *)Cstr;
        if ((Val - 0x01010101) & (~Val) & 0x80808080UL) break;
        Cstr += 4;
        Size += 4;
    }

    while (*Cstr != '\0') {
        Size++;
        Cstr++;
    }
    return Size;
}
static string8 String8FromCstring(char *Cstr) {
    u64 Size = CstringSize(Cstr);
    string8 String = {(u8 *)Cstr, Size};
    return String;
}

static string8 String8FromRange(u8 *First, u8 *Last) {
    string8 Ret = {First, (u64)Last - (u64)First};
    return Ret;
}

static string8 String8Prefix(string8 String, u64 Size) {
    Size = ClampTop(Size, String.Size);
    string8 Ret = {String.Str, Size};
    return Ret;
}

static string8 String8Postfix(string8 String, u64 Size) {
    Size = ClampTop(Size, String.Size);
    u8 *Start = String.Str + (String.Size - Size);
    string8 Ret = {Start, Size};
    return Ret;
}

static string8 String8Chop(string8 String, u64 Ammount) {
    Ammount = ClampTop(Ammount, String.Size);
    string8 Ret = {String.Str, String.Size - Ammount};
    return Ret;
}

static string8 String8Skip(string8 String, u64 Ammount) {
    Ammount = ClampTop(Ammount, String.Size);
    string8 Ret = {String.Str + Ammount, String.Size - Ammount};
    return Ret;
}

static string8 String8SubstringWindow(string8 String, v2_u64 Range) {
    Range.Min = ClampTop(Range.Min, String.Size);
    Range.Max = ClampTop(Range.Max, String.Size);
    string8 Ret = {String.Str + Range.Min, Range.Max - Range.Min};
    return Ret;
}

static string8 PushString8Copy(arena *Arena, string8 String) {
    string8 Ret = {0};
    // NOTE(acol): +1 to null terminate it for printing or passing to OS
    Ret.Str = (u8 *)ArenaPush(Arena, String.Size + 1);
    Ret.Size = String.Size;
    MemoryCopy(Ret.Str, String.Str, String.Size);
    Ret.Str[Ret.Size] = '\0';
    return Ret;
}

static string8 PushString8Cat(arena *Arena, string8 String1, string8 String2) {
    string8 Ret;
    Ret.Size = String1.Size + String2.Size;
    Ret.Str = PushArrayNoZero(Arena, u8, Ret.Size + 1);
    MemoryCopy(Ret.Str, String1.Str, String1.Size);
    MemoryCopy(Ret.Str + String1.Size, String2.Str, String2.Size);
    Ret.Str[Ret.Size] = '\0';
    return Ret;
}

#include <stdio.h>
#include <stdarg.h>

// TODO(acol): Really should make a custom vsnprintf cause it's slow af, also it should
// quickly calc the size to push when given null pointer instead of buffer so I dont
// have to do the entire pushing 1024 randomly at the start and then popping it
static string8 PushString8fv(arena *Arena, char *Fmt, va_list Args) {
    va_list Args2;
    // NOTE(acol): Varargs are weird when used so saving in case of retry
    va_copy(Args2, Args);

    u64 BufferSize = 1024;
    u8 *Buffer = (u8 *)ArenaPush(Arena, BufferSize);
    u64 ActualSize = vsnprintf((char *)Buffer, BufferSize, Fmt, Args);

    string8 Ret = {0};
    if (ActualSize < BufferSize) {
        ArenaPop(Arena, BufferSize - ActualSize - 1);
    } else {
        ArenaPop(Arena, BufferSize);
        Buffer = (u8 *)ArenaPush(Arena, ActualSize + 1);
        ActualSize = vsnprintf((char *)Buffer, ActualSize + 1, Fmt, Args2);
    }
    Ret = (string8){Buffer, ActualSize};
    va_end(Args2);
    return Ret;
}

static string8 PushString8f(arena *Arena, char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    string8 Ret = PushString8fv(Arena, Fmt, Args);
    va_end(Args);
    return Ret;
}

// NOTE(acol): Assuming the Node is already pushed into some arena, so namespacing it under string8list
// maybe retarded choice but whatever I am kinda assuming this will be a helper for the most part and not
// something I will be using
static string8_node *String8ListPushNode(string8_list *List, string8 String, string8_node *Node) {
    Node->String = String;
    SllQueuePush_N(List->First, List->Last, Node, Next);
    ++List->NodeCount;
    List->TotalSize += String.Size;
    return Node;
}

static string8_node *String8ListPush(arena *Arena, string8_list *List, string8 String) {
    string8_node *Node = (string8_node *)ArenaPush(Arena, sizeof(string8_node));
    String8ListPushNode(List, String, Node);
    return Node;
}

static string8_node *String8ListPushf(arena *Arena, string8_list *List, char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    string8 String = PushString8fv(Arena, Fmt, Args);
    va_end(Args);
    string8_node *Node = String8ListPush(Arena, List, String);
    return Node;
}

static string8_list String8Split(arena *Arena, string8 String, u8 *SplitCharacters, u32 SplitCharsCount) {
    string8_list List = {0};

    u8 *Current = String.Str;
    u8 *WordStart = Current;
    u8 *LastByte = String.Str + String.Size;

    for (; Current < LastByte; Current++) {
        b32 IsSplit = 0;
        for (u32 i = 0; i < SplitCharsCount; i++) {
            if (*Current == SplitCharacters[i]) {
                IsSplit = 1;
                break;
            }
        }

        if (IsSplit) {
            if (WordStart < Current) {
                String8ListPush(Arena, &List, String8FromRange(WordStart, Current));
            }
            WordStart = Current + 1;
        }
    }
    if (WordStart < Current) {
        String8ListPush(Arena, &List, String8FromRange(WordStart, Current));
    }
    return List;
}

static string8 String8ListJoin(arena *Arena, string8_list *List, string8_join *OptionalStringJoin) {
    string8 Ret = {0};
    string8_join Join = {0};
    if (OptionalStringJoin) {
        MemoryCopyStruct(&Join, OptionalStringJoin);
    }
    Ret.Size = Join.Pre.Size + Join.Post.Size + Join.Sep.Size * (List->NodeCount ? List->NodeCount - 1 : 0) +
               List->TotalSize;

    u8 *Ptr = Ret.Str = (u8 *)ArenaPushNoZero(Arena, Ret.Size + 1);

    MemoryCopy(Ptr, Join.Pre.Str, Join.Pre.Size);
    Ptr += Join.Pre.Size;

    for (string8_node *Node = List->First; Node != 0; Node = Node->Next) {
        MemoryCopy(Ptr, Node->String.Str, Node->String.Size);
        Ptr += Node->String.Size;
        if (Node->Next) {
            MemoryCopy(Ptr, Join.Sep.Str, Join.Sep.Size);
            Ptr += Join.Sep.Size;
        }
    }
    MemoryCopy(Ptr, Join.Post.Str, Join.Post.Size);
    Ptr += Join.Post.Size;
    *Ptr = '\0';

    return Ret;
}
