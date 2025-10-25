#ifndef FUZZY_MATCH_H
#define FUZZY_MATCH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {

#endif
struct node
{
    int number;
    struct node* next;
};

extern struct node* match_end;
int32_t fuzzy_match(const char* pattern, const char* str);
int recursive_delete(struct node* p);
#ifdef __cplusplus
}
#endif


#endif /* FUZZY_MATCH_H */
