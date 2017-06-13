#include "monetR.h"

#define _mn_clog_id_range(id) (id==MN_CLOGID_NONE||id==MN_CLOGID_ALL||id==MN_CLOGID_CON)?0:1

error13_t mn_clog_init(struct monet* mn){
	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);
    return E13_OK;
}

error13_t mn_clog_add(struct monet* mn, enum mn_clog_type type, mn_clogid_t id, void* ptr){

	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

	if(!_mn_clog_id_range(id))
		return e13_error(E13_RANGE);

    return e13_error(E13_IMPLEMENT);
}

error13_t mn_clog_rm(struct monet* mn, mn_clogid_t id){

	if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

	if(!_mn_clog_id_range(id))
		return e13_error(E13_RANGE);

	return e13_error(E13_IMPLEMENT);
}

/*
    acceptable format:

    c - character
    i - int
    f - float
    e - error13 code
    t - time
    s - string

    TODO: nothing is done in a secure way, i know!

*/

error13_t mn_clog(struct monet* mn, mn_clogid_t id, char* format, ...){

    va_list ap;
    signed char c;              // 8-bit
    long int l;                 // 32-bit
    unsigned long int u;                 // 32-bit
    double d;
    char *s;                    // strings
    size_t total = 0, bufsize = MN_CONF_CLOG_BUFSIZE;
    char buf[MN_CONF_CLOG_BUFSIZE];
    char t13[MAXTIME];

    if(!_mn_is_init(mn)) return e13_error(E13_MISUSE);

    va_start(ap, format);

    for(; *format != '\0'; format++) {
    	if(*format == '%'){
			switch(*(++format)) {
			case 'c': // 8-bit
				c = (signed char)va_arg(ap, int); // promoted
				total += snprintf(buf + total, bufsize - total, "%c", c);
				break;

			case 'i': // 32-bit
				l = va_arg(ap, long int);
				total += snprintf(buf + total, bufsize - total, "%l", l);
				break;

			case 'u': // 32-bit
				u = va_arg(ap, unsigned long int);
				total += snprintf(buf + total, bufsize - total, "%lu", u);
				break;

			case 'd': // float-32
				d = va_arg(ap, double);
				total += snprintf(buf + total, bufsize - total, "%f", d);
				break;

			case 's': // string
				s = va_arg(ap, char*);
				total += snprintf(buf + total, bufsize - total, "%s", s);
				break;

			case 't'://time
				switch(mn->conf.date_fmt){
					case MN_DATE_PERSIAN:
						d13_now(t13);
					break;
					default:
						d13_nowg(t13);
					break;
				}
				total += snprintf(buf + total, bufsize - total, "%s", t13);
				break;

			case 'e'://error13 code
				l = va_arg(ap, int);
				total += snprintf(buf + total, bufsize - total, "%s[%l]",
									e13_codemsg(l), l);
				break;

			default:
				format--;
				break;

			}
    	} else {
    		total += snprintf(buf + total, bufsize - total, "%c", *format);
    	}
    }

    va_end(ap);

	switch(mn->conf.date_fmt){
		case MN_DATE_PERSIAN:
			d13_now(t13);
		break;
		default:
			d13_nowg(t13);
		break;
	}

    if(!mn->clog_list.first){
        printf(".:MN@%s: %s.\n", t13, buf);
    }

    return E13_OK;

}
