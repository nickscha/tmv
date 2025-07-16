/* tmv_tokenizer.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) general tokenizer.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef TMV_TOKENIZER_H
#define TMV_TOKENIZER_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define TMV_TOKENIZER_INLINE inline
#define TMV_TOKENIZER_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_TOKENIZER_INLINE __inline__
#define TMV_TOKENIZER_API static
#elif defined(_MSC_VER)
#define TMV_TOKENIZER_INLINE __inline
#define TMV_TOKENIZER_API static
#else
#define TMV_TOKENIZER_INLINE
#define TMV_TOKENIZER_API static
#endif

typedef enum tmv_tokenizer_type
{
    TOKEN_UNKNOWN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_EOF
} tmv_tokenizer_type;

typedef struct
{
    char *start;
    unsigned long len;
    tmv_tokenizer_type type;

} tmv_tokenizer_token;

typedef struct
{
    char *src;
    unsigned long len;
    unsigned long pos;

} tmv_tokenizer;

#define TMV_TOKENIZER_IS_WHITESPACE(c) ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
#define TMV_TOKENIZER_IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define TMV_TOKENIZER_IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define TMV_TOKENIZER_IS_ALNUM(c) (TMV_TOKENIZER_IS_ALPHA(c) || TMV_TOKENIZER_IS_DIGIT(c))

TMV_TOKENIZER_API TMV_TOKENIZER_INLINE tmv_tokenizer_token tmv_tokenizer_token_next(tmv_tokenizer *tz)
{
    char c;

    tmv_tokenizer_token t = {0};
    t.start = tz->src + tz->pos;

    while (tz->pos < tz->len && TMV_TOKENIZER_IS_WHITESPACE(tz->src[tz->pos]))
    {
        tz->pos++;
        t.start = tz->src + tz->pos;
    }

    if (tz->pos >= tz->len)
    {
        t.type = TOKEN_EOF;
        t.len = 0;
        return t;
    }

    c = tz->src[tz->pos];

    if (TMV_TOKENIZER_IS_ALPHA(c))
    {
        unsigned long start = tz->pos;
        tz->pos++;
        while (tz->pos < tz->len && TMV_TOKENIZER_IS_ALNUM(tz->src[tz->pos]))
        {
            tz->pos++;
        }
        t.len = tz->pos - start;
        t.type = TOKEN_IDENTIFIER;
        return t;
    }

    if (TMV_TOKENIZER_IS_DIGIT(c))
    {
        unsigned long start = tz->pos;
        tz->pos++;
        while (tz->pos < tz->len && TMV_TOKENIZER_IS_DIGIT(tz->src[tz->pos]))
        {
            tz->pos++;
        }
        t.len = tz->pos - start;
        t.type = TOKEN_NUMBER;
        return t;
    }

    /* Single-character symbols */
    t.type = TOKEN_SYMBOL;
    t.len = 1;
    tz->pos++;
    return t;
}

#endif /* TMV_TOKENIZER_H */