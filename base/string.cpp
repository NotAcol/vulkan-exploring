AsanDisable static u64 CstringSize(u8 *Cstr) {
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

static string StringFromCstring(u8 *Cstr) {
    u64 Size = CstringSize(Cstr);
    string String = {Cstr, Size};
    return String;
}

static string StringFromRange(u8 *First, u8 *Last) {
    string Ret = {First, (u64)Last - (u64)First};
    return Ret;
}

static string StringPrefix(string String, u64 Size) {
    Size = ClampTop(Size, String.Size);
    string Ret = {String.Str, Size};
    return Ret;
}

static string StringPostfix(string String, u64 Size) {
    Size = ClampTop(Size, String.Size);
    u8 *Start = String.Str + (String.Size - Size);
    string Ret = {Start, Size};
    return Ret;
}

static string StringChop(string String, u64 Ammount) {
    Ammount = ClampTop(Ammount, String.Size);
    string Ret = {String.Str, String.Size - Ammount};
    return Ret;
}

static string StringSkip(string String, u64 Ammount) {
    Ammount = ClampTop(Ammount, String.Size);
    string Ret = {String.Str + Ammount, String.Size - Ammount};
    return Ret;
}

static string StringSubstringWindow(string String, u64 First, u64 Last) {
    First = ClampTop(First, String.Size);
    Last = ClampTop(Last, String.Size);
    string Ret = {String.Str + First, Last - First};
    return Ret;
}

static string StringSubstringSize(string String, u64 First, u64 Size) {
    string Ret = StringSubstringWindow(String, First, First + Size);
    return Ret;
}

static string StringPushCopy(arena *Arena, string String) {
    string Ret = {};
    // NOTE(acol): +1 to null terminate it for printing or passing to OS
    Ret.Str = (u8 *)ArenaPush(Arena, String.Size + 1);
    Ret.Size = String.Size;
    MemoryCopy(Ret.Str, String.Str, String.Size);
    Ret.Str[Ret.Size] = '\0';
    return Ret;
}

static void StringListPushExplicit(string_list *List, string String, string_node *Node) {
    Node->String = String;
    SllQueuePush_N(List->First, List->Last, Node, Next);

    ++List->NodeCount;
    List->TotalSize += String.Size;
}

static void StringListPush(arena *Arena, string_list *List, string String) {
    string_node *Node = (string_node *)ArenaPush(Arena, sizeof(string_node));
    StringListPushExplicit(List, String, Node);
}

static string StringJoin(arena *Arena, string_list *List, string_join *Join) {
    ProfileFunction();
    u64 Size = 0;
    string_join Temp = {};
    if (!Join) {
        Join = &Temp;
    }
    Size = Join->Pre.Size + Join->Post.Size + Join->Mid.Size * (List->NodeCount - 1) + List->TotalSize;

    u8 *Str = (u8 *)ArenaPush(Arena, Size + 1);
    u8 *Ptr = Str;

    MemoryCopy(Ptr, Join->Pre.Str, Join->Pre.Size);
    Ptr += Join->Pre.Size;

    string_node *Node = List->First;
    MemoryCopy(Ptr, Node->String.Str, Node->String.Size);
    Ptr += Node->String.Size;
    Node = Node->Next;

    for (; Node != 0; Node = Node->Next) {
        MemoryCopy(Ptr, Join->Mid.Str, Join->Mid.Size);
        Ptr += Join->Mid.Size;
        MemoryCopy(Ptr, Node->String.Str, Node->String.Size);
        Ptr += Node->String.Size;
    }

    MemoryCopy(Ptr, Join->Post.Str, Join->Post.Size);
    Ptr += Join->Post.Size;
    *Ptr = '\0';
    string Ret = {Str, Size};
    return Ret;
}

static string_list StringSplit(arena *Arena, string String, u8 *SplitCharacters, u32 SplitCharsCount) {
    // NOTE(acol): This is slow as shit if it gets used often might have to optimize
    ProfileFunction();
    string_list List = {};

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
                StringListPush(Arena, &List, StringFromRange(WordStart, Current));
            }
            WordStart = Current + 1;
        }
    }
    if (WordStart < Current) {
        StringListPush(Arena, &List, StringFromRange(WordStart, Current));
    }
    return List;
}

#include <stdio.h>
#include <stdarg.h>

// TODO(acol): Really should make a custom vsnprintf cause it's slow af, also it should
// quickly calc the size to push when given null pointer instead of buffer so I dont
// have to do the entire pushing 1024 randomly at the start and then popping it
static string StringPushfv(arena *Arena, char *Fmt, va_list Args) {
    ProfileFunction();

    va_list Args2;
    // NOTE(acol): Varargs are weird when used so saving in case of retry
    va_copy(Args2, Args);

    u64 BufferSize = 1024;
    u8 *Buffer = (u8 *)ArenaPush(Arena, BufferSize);
    u64 ActualSize = vsnprintf((char *)Buffer, BufferSize, Fmt, Args);

    string Ret = {};
    if (ActualSize < BufferSize) {
        ArenaPop(Arena, BufferSize - ActualSize - 1);
    } else {
        ArenaPop(Arena, BufferSize);
        Buffer = (u8 *)ArenaPush(Arena, ActualSize + 1);
        ActualSize = vsnprintf((char *)Buffer, ActualSize + 1, Fmt, Args2);
    }
    Ret = {Buffer, ActualSize};
    va_end(Args2);
    return Ret;
}

static string StringPushf(arena *Arena, char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    string Ret = StringPushfv(Arena, Fmt, Args);
    va_end(Args);
    return Ret;
}

static void StringListPushf(arena *Arena, string_list *List, char *Fmt, ...) {
    va_list Args;
    va_start(Args, Fmt);
    string String = StringPushfv(Arena, Fmt, Args);
    va_end(Args);
    StringListPush(Arena, List, String);
}
