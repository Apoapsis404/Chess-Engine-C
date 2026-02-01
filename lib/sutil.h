#ifndef SUTIL_H
#define SUTIL_H

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

#endif //SUTIL_H;