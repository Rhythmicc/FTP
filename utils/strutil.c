#include "strutil.h"
#include "../common.h"

/**
 * 去掉字符串末尾的空白符
 * @param str
 */
void str_strip(char *str) {
    char *p = &str[strlen(str)-1];
    while(*p == '\r' || *p == '\n' || *p == ' ') *p-- = '\0';
}

/**
 * 分割字符串
 * @param str 源串
 * @param left 存储分割后左部分
 * @param right 存储分割后的右部分
 * @param c 用于分割的字符
 */
void str_split(const char *str , char *left, char *right, char c) {
    char *p = strchr(str, c);
    if (p == NULL) strcpy(left, str);
    else {
        strncpy(left, str, p - str);
        strcpy(right, p + 1);
    }
}

/**
 * 判断字符串中是否全为空格
 * @param str
 * @return
 */
int str_all_space(const char *str) {
    while (*str) {
        if (!isspace(*str)) return 0;
        ++str;
    }
    return 1;
}

/**
 * 将字符串中所有字母大写
 * @param str
 */
void str_upper(char *str) {
    while (*str) {
        *str = toupper(*str);
        ++str;
    }
}

/**
 * 字符串八进制转十进制
 * @param str
 * @return 十进制值
 */
unsigned int str_octal_to_uint(const char *str) {
    unsigned int result = 0;
    int seen_non_zero_digit = 0;

    while (*str) {
        int digit = *str;
        if (!isdigit(digit) || digit > '7')
            break;

        if (digit != '0')
            seen_non_zero_digit = 1;

        if (seen_non_zero_digit) {
            result <<= 3;
            result += (digit - '0');
        }
        ++str;
    }
    return result;
}

