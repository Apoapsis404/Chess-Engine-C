#ifndef SUTIL_H
#define SUTIL_H

#include <stdlib.h>



// Dynamic String functions
#define STRING_SIZE_INIT 4

typedef struct {
    char *string;
    size_t count;
    size_t capacity;
} String;

/* 
Borrowed String
Does not allow for changes in alloced space
*/
typedef struct {
    char *string;
    size_t count;
} BString;

String *string_init(char *string);
int string_append(String *dst, const char src);
int string_append_many(String *dst, const char *src, size_t src_size);
int string_cat(String *dst, String *src);
int string_cat_free(String *dst, String *src);

void free_string(String *s);
void free_alloced_string(String *s);

BString bstring_from_string(String *s);
BString bstring_next(BString *bs, char delim);

int char_in_string(String *s, char c);
int char_in_bstring(BString *s, char c);


void bstring_print(BString *bs, char end);
#define bstring_println(bs) bstring_print(bs, '\n')

int string_map(String *s, char (*f)(char));
#define string_to_lower(s) string_map(s, to_lower);
#define string_to_upper(s) string_map(s, to_upper);

// Character functions

//ASCII THING
#define DIGITSTART    48
#define DIGITEND      57
#define UPPERSTART    65
#define UPPEREND      90
#define LOWERSTART    97
#define LOWEREND     122
#define FORWARDSLASH  47

int is_lower(char c);
int is_upper(char c);
char to_lower(char c);
char to_upper(char c);

int is_digit(char c);
int char_digit_to_int(char c);
char int_digit_to_char(int i);
int char_in_range(char c, int start, int end);

#endif //SUTIL_H;