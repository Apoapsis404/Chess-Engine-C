#include "sutil.h"

#include <string.h>
#include <stdio.h>



// Dynamic String functions


/*
Reallocates space for string
Returns 0 on success and -1 on error
*/
int string_alloc_space(String* s, size_t needed){
    needed++;
    if(needed < s->capacity) return 0;
    if(s->capacity == 0) s->capacity = STRING_SIZE_INIT;

    while (s->capacity < needed) {
        s->capacity *= 2;
    }

    s->string = realloc(s->string, s->capacity * sizeof(char));
    if (s->string == NULL) return -1;
    return 0;
}

/* 
Appends src to the end of dst
Returns: 1 on success, 
Returns -1 on alloc error
*/
int string_append(String *dst, const char src){
    if(string_alloc_space(dst, dst->count) == -1) return -1;
    dst->string[dst->count++] = src;
    dst->string[dst->count] = '\0';
    return 1;
}

/*
Appends src to the end of dst
Returns chars appended
Returns -1 on alloc fail
*/
int string_append_many(String *dst, const char *src, size_t src_size){
    if(string_alloc_space(dst, dst->count + src_size) == -1) return -1;
    memcpy(dst->string + dst->count, src, src_size);
    dst->count += src_size;

    dst->string[dst->count] = '\0';
    return 0;
}

/*
Concats src at the end of dst
Returns chars appended
Returns -1 on alloc fail
*/
int string_cat(String *dst, String *src){
    int ret = string_append_many(dst, src->string, src->count);
    return ret;
}

/*
Concats src at the end of dst, and frees src
Returns chars appended
Returns -1 on alloc fail
string is not free'd on error
*/
int string_cat_free(String *dst, String *src){
    int ret = string_append_many(dst, src->string, src->count);
    if(ret == -1) return -1;
    free_string(src);
    return ret;
}

void free_string(String *s){
    free(s->string);
}

BString bstring_from_string(String *s){
    BString bs;
    bs.string = s->string;
    bs.count = s->count;
    return bs;
}

// For now: Expects there to be something here! (because that garanties a null terminator)
BString bstring_next(BString *bs, char delim){
    size_t i = 0;
    while(i < bs->count && bs->string[i] != '\0' && bs->string[i] != delim){
        i++;
    }

    BString ret;
    ret.string = bs->string;
    ret.count = i;

    if (i < bs->count){
        bs->count -= i + 1;
        bs->string += i + 1;
    } else {
        bs->count -= i;
        bs->string += i;
    }

    return ret;
}

void bstring_print(BString *bs, char end){
    for(size_t i = 0; i < bs->count; ++i){
        printf("%c", bs->string[i]);
    }
    printf("%c", end);
}


int string_map(String *s, char (*f)(char)){
    for (size_t i = 0; i < s->count; ++i){
        s->string[i] = (*f)(s->string[i]);
    }
    return 0;
}











// Character functions

int is_lower(char c){
    if (c >= LOWERSTART && c <= LOWEREND) {
        return 1;
    }
    return 0;
}

int is_upper(char c){
    if (c >= UPPERSTART && c <= UPPEREND) {
        return 1;
    }
    return 0;
}

char to_lower(char c){
    if(is_upper(c)){
        return c + 32;
    }
    return c;
}

char to_upper(char c){
    if(is_lower(c)){
        return c - 32;
    }
    return c;
}

/* Checks if c is (inclusivly) between 48 and 57*/
int is_digit(char c){
    if (c <= DIGITEND && c >= DIGITSTART) {
        return 1;
    }
    return 0;
}

int char_digit_to_int(char c) {
    return c - DIGITSTART;
}

/* Turns a int_digit into a char */
char int_digit_to_char(int i){
    if(i < 0 || i > 9) return 0;
    return i + DIGITSTART;
}