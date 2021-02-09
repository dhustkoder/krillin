#ifndef KRL_STATIC_STRING_H_
#define KRL_STATIC_STRING_H_

#define KRLSS_MAX_CAP (4096)

typedef struct krlss {
	size_t len;
	char data[];
} krlss_t;


#define KRL_STATIC_STRING_DEFINE(name, maxlen) \
static char krl_static_string_assert_##name##_len[(maxlen > 0 && maxlen <= KRLSS_MAX_CAP) ? 1 : -1]; \
typedef struct {\
	size_t len;\
	char data[maxlen + 1];\
} krlss_##name##_t;

#define KRLSS_CAP(ss)  ((sizeof((ss)->data)) - 1)


inline void krl_static_string_internal_fmt_data(krlss_t* ss, size_t cap)
{
	assert(ss->len <= cap);
	
	if (ss->len == cap) {
		for (int i = 0; i < 3; ++i) {
			ss->data[(cap - 3) + i] = '.';
		}
		ss->data[cap] = '\0';
	} else {
		ss->data[ss->len] = '\0';
	}
}

inline void krl_static_string_assign(krlss_t* ss, size_t cap, const char* str, size_t len)
{
	len = len > cap ? cap : len;
	memcpy(ss->data, str, len);
	ss->len = len;

	krl_static_string_internal_fmt_data(ss, cap);
}

inline int krl_static_string_ncmp(const krlss_t* ss, const char* str, size_t len)
{
	return strncmp(ss->data, str, len);
}

inline void krl_static_string_concat(krlss_t* ss, size_t cap, const char* str, size_t len)
{
	len = len > (cap - ss->len) ? (cap - ss->len) : len;
	
	memcpy(ss->data + ss->len, str, len);
	ss->len += len;
	
	krl_static_string_internal_fmt_data(ss, cap);
}




#define krlss_assign_ex(ss, str, len) \
	krl_static_string_assign((krlss_t*)(ss), KRLSS_CAP(ss), str, len)

#define krlss_assign(lhs, rhs) \
	krlss_assign_ex(lhs, rhs.data, rhs.len)

#define krlss_assign_cstr(ss, str) \
	krlss_assign_ex(ss, str, strlen(str))



#define krlss_ncmp_ex(ss, str, len) \
	krl_static_string_ncmp((const krlss_t*)(&ss), str, len)
	
#define krlss_ncmp(lhs, rhs) \
	krlss_ncmp_ex(ss, rhs.data, rhs.len)
	
#define krlss_ncmp_cstr(ss, str, len) \
	krlss_ncmp_ex(ss, str, len)



#define krlss_concat_ex(ss, str, len) \
	krl_static_string_concat((krlss_t*)ss, KRLSS_CAP(ss), str, len)
	
#define krlss_concat(lhs, rhs) \
	krlss_concat_ex(lhs, rhs.data, rhs.len)

#define krlss_concat_cstr(ss, str) \
	krlss_concat_ex(ss, str, strlen(str))




#endif




