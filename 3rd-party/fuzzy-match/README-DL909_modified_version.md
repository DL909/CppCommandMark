## changelog

- make it usable with C++
- add CMakeLists.txt
- add a chain array that saves characters that is matched

## detail of char array

due to the implementation, the chain array actually is extended in reverse order, and its number actually means the distance between this and last match(the subtraction of their index minus one, and the first one is provided as index).

after using the char array, you should call `recursive_delete(match_end)` to clean it. The program won't clean it by itself and will discard the pointer on next match. So forgetting of cleaning may cause memory leak.

a simple way to use it is shown below, `array` contains index of matched character.

```C++
#include "fuzzy_match.h"
#include <vector>

int main(const int argc, const char** argv)
{
    fuzzy_match("pattern","some text");
    const node * p = match_end;
    int index = -1;
    std::vector<int> array;
    while (p != nullptr)
    {
        index += p->number + 1;
        array.push_back(index);
        p = p->next;
    }
    recursive_delete(match_end);
    return 0;
}
```