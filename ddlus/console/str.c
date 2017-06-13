#include "double.h"

size_t strlen_(const char *str, size_t max_size)
{
    size_t ret;
    for (ret = 0; ret < max_size; ret++)
        if (str[ret] == '\0')
            break;

    return ret;
}

size_t strcpy_(char *dest, const char *src, size_t size)
{
    size_t ret = strlen_(src, size);
    size_t len = (ret >= size) ? size - 1 : ret;

    assert(dest);

    if (size) {
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return ret;
}

size_t strcat_(char *dest, const char *src, size_t count)
{
    size_t dsize = strlen_(dest, count);
    size_t len = strlen_(src, count);
    size_t res = dsize + len;

    assert(dest);

    dest += dsize;
    count -= dsize;
    if (len >= count)
        len = count - 1;
    memcpy(dest, src, len);
    dest[len] = 0;
    return res;
}

char* reverse_(char* text, int len) {
    int start = -1;
    int end = len;
    char temp;
    while (++start < --end) {
        temp = text[start];
        text[start] = text[end];
        text[end] = temp;
    }
    return text;
}

int wildcmp(const char* dst, const char* src, const char any,
                const char one, const char escape){

    while(*src){
        //printf("%c & %c;", *src, *dst);
        if(*src == escape){
            src++;
            goto direct_check;
        }
        if(*src == any){
            while(*src == any) src++;//jump to first non-* character
            //bypass a dst character in the middle of the string
            //the above term is not needed, it just complicates things
            //now see if the *src is found in the rest of dst
            if(*src != one){
                dst = strchr(dst, *src);
                if(!dst) return -1;
            }
            //dst++;//this must point to the
            return wildcmp(dst, src, any, one, escape);
        } else if(*src == one && *dst){
            while(*src == one && *dst){
                dst++;
                src++;
            }
            //return s13_wildcmp(dst, src, any, one);
        } else {
direct_check:
            if(*dst != *src) return -1;
            dst++;
            src++;
        }
    }
    if(strlen(dst) && *(src-1) != any) return -1;
    //printf("#\n");
    return 0;
}

char* rstrpbrk(char* buf, char* delim){

    size_t len = strlen(buf);
    if(!len) return NULL;

    do{
        if(strchr(delim, buf[--len])) break;

    } while(len);

    return len?buf + len:NULL;
}
