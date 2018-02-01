/* This code has been heavily hacked by Tatu Ylonen <ylo@cs.hut.fi> to
   make it compile on machines like Cray that don't have a 32 bit integer
   type.  The interfaces have also been changed. */

/*
 * $Log: md5.c,v $
 * $EndLog$
 */

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#include "sshincludes.h"
#include "sshcrypt.h"
#include "sshcrypti.h"
#include "md5.h"
#include "sshgetput.h"

/* Definition of hash function called "md5". */
const SshHashDef ssh_hash_md5_def =
{
  /* Name of the hash function. */
  "md5",
  /* ASN.1 Object Identifier
     iso(1) member-body(2) US(840) rsadsi(113549) digestAlgorithm(2) 5 */
  "1.2.840.113549.2.5", 
  /* ISO/IEC dedicated hash identifier (doesn't have one). */
  0,
  /* Digest size */
  16,
  /* Input block length */
  64,
  /* Context size */
  ssh_md5_ctxsize,
  /* Reset function, between long operations */
  ssh_md5_reset_context, 
  /* Update function for long operations. */
  ssh_md5_update,
  /* Final function to get the digest. */
  ssh_md5_final
};

/* The type MD5Context is used to represent an MD5 context while the
   computation is in progress.  The normal usage is to first initialize
   the context with md5_init, then add data by calling md5_update one or
   more times, and then call md5_final to get the digest.  */

typedef struct {
  SshUInt32 buf[4];
  SshUInt32 bits[2];
  unsigned char in[64];
} SshMD5Context;

void ssh_md5_reset_context(void *context)
{
  SshMD5Context *ctx = context;
  ctx->buf[0] = 0x67452301L;
  ctx->buf[1] = 0xefcdab89L;
  ctx->buf[2] = 0x98badcfeL;
  ctx->buf[3] = 0x10325476L;

  ctx->bits[0] = 0;
  ctx->bits[1] = 0;
}

size_t ssh_md5_ctxsize()
{
  return sizeof(SshMD5Context);
}

void ssh_md5_update(void *context, const unsigned char *buf, size_t len)
{
  SshMD5Context *ctx = context;
  SshUInt32 t;

  /* Update bitcount */

  t = ctx->bits[0];
  if ((ctx->bits[0] = (t + ((SshUInt32)len << 3)) & 0xffffffffL) < t)
    ctx->bits[1]++;             /* Carry from low to high */
  ctx->bits[1] += (SshUInt32)len >> 29;

  t = (t >> 3) & 0x3f;  /* Bytes already in shsInfo->data */

  /* Handle any leading odd-sized chunks */
  if (t)
    {
      unsigned char *p = ctx->in + t;

      t = 64 - t;
      if (len < t)
        {
          memcpy(p, buf, len);
          return;
        }
      memcpy(p, buf, t);
      ssh_md5_transform(ctx->buf, ctx->in);
      buf += t;
      len -= t;
    }

  /* Process data in 64-byte chunks */
  while (len >= 64)
    {
      memcpy(ctx->in, buf, 64);
      ssh_md5_transform(ctx->buf, ctx->in);
      buf += 64;
      len -= 64;
    }

  /* Handle any remaining bytes of data. */
  memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void ssh_md5_final(void *context, unsigned char *digest)
{
  SshMD5Context *ctx = context;
  unsigned int count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx->bits[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = ctx->in + count;
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if (count < 8)
    {
      /* Two lots of padding:  Pad the first block to 64 bytes */
      memset(p, 0, count);
      ssh_md5_transform(ctx->buf, ctx->in);

      /* Now fill the next block with 56 bytes */
      memset(ctx->in, 0, 56);
    }
  else
    {
      /* Pad block to 56 bytes */
      memset(p, 0, count - 8);
    }

  /* Append length in bits and transform */
  SSH_PUT_32BIT_LSB_FIRST(ctx->in + 56, ctx->bits[0]);
  SSH_PUT_32BIT_LSB_FIRST(ctx->in + 60, ctx->bits[1]);
  ssh_md5_transform(ctx->buf, ctx->in);

  /* Convert the internal state to bytes and return as the digest. */
  SSH_PUT_32BIT_LSB_FIRST(digest, ctx->buf[0]);
  SSH_PUT_32BIT_LSB_FIRST(digest + 4, ctx->buf[1]);
  SSH_PUT_32BIT_LSB_FIRST(digest + 8, ctx->buf[2]);
  SSH_PUT_32BIT_LSB_FIRST(digest + 12, ctx->buf[3]);
  memset(ctx, 0, sizeof(ctx));  /* In case it's sensitive */
}

void ssh_md5_of_buffer(unsigned char digest[16], const unsigned char *buf,
                       size_t len)
{
  SshMD5Context context;
  ssh_md5_reset_context(&context);
  ssh_md5_update(&context, buf, len);
  ssh_md5_final(&context, digest);
}

#if !defined(ASM_MD5)


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = (w<<s | w>>(32-s)) & 0xffffffff,  \
          w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void ssh_md5_transform(SshUInt32 buf[4], const unsigned char inext[64])
{
    register SshUInt32 a, b, c, d, i;
    SshUInt32 in[16];
    
    for (i = 0; i < 16; i++)
      in[i] = SSH_GET_32BIT_LSB_FIRST(inext + 4 * i);

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478L, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756L, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070dbL, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceeeL, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0fafL, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62aL, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613L, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501L, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8L, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7afL, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1L, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7beL, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122L, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193L, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438eL, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821L, 22);
    
    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562L, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340L, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51L, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aaL, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105dL, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453L, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681L, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8L, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6L, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6L, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87L, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14edL, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905L, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8L, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9L, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8aL, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942L, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681L, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122L, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380cL, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44L, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9L, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60L, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70L, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6L, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127faL, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085L, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05L, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039L, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5L, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8L, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665L, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244L, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97L, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7L, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039L, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3L, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92L, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47dL, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1L, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4fL, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0L, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314L, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1L, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82L, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235L, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bbL, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391L, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

#else /* ASM_MD5 */

#if defined(WIN32)

/* Following assembler code is perhaps not fastest, however, it may
   very well be faster than the above compiled. You should test this
   experimentally. */

/* Define the suitable macros. */
#define MD5STEPF1(w, t, x, y, z, buffer, data, s) \
  mov t, y; \
  add w, buffer; \
  xor t, z; \
  add w, data; \
  and t, x; \
  xor t, z; \
  add w, t; \
  rol w, s; \
  add w, x;

#define MD5STEPF2(w, t, x, y, z, buffer, data, s) \
  mov t, x; \
  add w, buffer; \
  xor t, y; \
  add w, data; \
  and t, z; \
  xor t, y; \
  add w, t; \
  rol w, s; \
  add w, x;

#define MD5STEPF3(w, t, x, y, z, buffer, data, s) \
  mov t, x; \
  add w, buffer; \
  xor t, y; \
  add w, data; \
  xor t, z; \
  add w, t; \
  rol w, s; \
  add w, x;


#define MD5STEPF4(w, t, x, y, z, buffer, data, s) \
  mov t, z; \
  add w, buffer; \
  not t; \
  add w, data; \
  or  t, x; \
  xor t, y; \
  add w, t; \
  rol w, s; \
  add w, x;



void ssh_md5_transform(SshUInt32 buf[4], const unsigned char inext[64])
{
  __asm
    {
      /* Set esi and edi. */
      mov esi, inext
      mov edi, buf
        
      /* Make a copy of  the input. */
      mov eax, [edi     ]
      mov ebx, [edi +  4]
      mov ecx, [edi +  8]
      mov edx, [edi + 12]
        
      /* The hashing operations. */
      MD5STEPF1(eax, edi, ebx, ecx, edx, [esi + 0*4], 0xd76aa478, 7);
      MD5STEPF1(edx, edi, eax, ebx, ecx, [esi + 1*4], 0xe8c7b756, 12);
      MD5STEPF1(ecx, edi, edx, eax, ebx, [esi + 2*4], 0x242070db, 17);
      MD5STEPF1(ebx, edi, ecx, edx, eax, [esi + 3*4], 0xc1bdceee, 22);
      MD5STEPF1(eax, edi, ebx, ecx, edx, [esi + 4*4], 0xf57c0faf, 7);
      MD5STEPF1(edx, edi, eax, ebx, ecx, [esi + 5*4], 0x4787c62a, 12);
      MD5STEPF1(ecx, edi, edx, eax, ebx, [esi + 6*4], 0xa8304613, 17);
      MD5STEPF1(ebx, edi, ecx, edx, eax, [esi + 7*4], 0xfd469501, 22);
      MD5STEPF1(eax, edi, ebx, ecx, edx, [esi + 8*4], 0x698098d8, 7);
      MD5STEPF1(edx, edi, eax, ebx, ecx, [esi + 9*4], 0x8b44f7af, 12);
      MD5STEPF1(ecx, edi, edx, eax, ebx, [esi + 10*4], 0xffff5bb1, 17);
      MD5STEPF1(ebx, edi, ecx, edx, eax, [esi + 11*4], 0x895cd7be, 22);
      MD5STEPF1(eax, edi, ebx, ecx, edx, [esi + 12*4], 0x6b901122, 7);
      MD5STEPF1(edx, edi, eax, ebx, ecx, [esi + 13*4], 0xfd987193, 12);
      MD5STEPF1(ecx, edi, edx, eax, ebx, [esi + 14*4], 0xa679438e, 17);
      MD5STEPF1(ebx, edi, ecx, edx, eax, [esi + 15*4], 0x49b40821, 22);
      
      MD5STEPF2(eax, edi, ebx, ecx, edx, [esi + 1*4], 0xf61e2562, 5);
      MD5STEPF2(edx, edi, eax, ebx, ecx, [esi + 6*4], 0xc040b340, 9);
      MD5STEPF2(ecx, edi, edx, eax, ebx, [esi + 11*4], 0x265e5a51, 14);
      MD5STEPF2(ebx, edi, ecx, edx, eax, [esi + 0*4], 0xe9b6c7aa, 20);
      MD5STEPF2(eax, edi, ebx, ecx, edx, [esi + 5*4], 0xd62f105d, 5);
      MD5STEPF2(edx, edi, eax, ebx, ecx, [esi + 10*4], 0x02441453, 9);
      MD5STEPF2(ecx, edi, edx, eax, ebx, [esi + 15*4], 0xd8a1e681, 14);
      MD5STEPF2(ebx, edi, ecx, edx, eax, [esi + 4*4], 0xe7d3fbc8, 20);
      MD5STEPF2(eax, edi, ebx, ecx, edx, [esi + 9*4], 0x21e1cde6, 5);
      MD5STEPF2(edx, edi, eax, ebx, ecx, [esi + 14*4], 0xc33707d6, 9);
      MD5STEPF2(ecx, edi, edx, eax, ebx, [esi + 3*4], 0xf4d50d87, 14);
      MD5STEPF2(ebx, edi, ecx, edx, eax, [esi + 8*4], 0x455a14ed, 20);
      MD5STEPF2(eax, edi, ebx, ecx, edx, [esi + 13*4], 0xa9e3e905, 5);
      MD5STEPF2(edx, edi, eax, ebx, ecx, [esi + 2*4], 0xfcefa3f8, 9);
      MD5STEPF2(ecx, edi, edx, eax, ebx, [esi + 7*4], 0x676f02d9, 14);
      MD5STEPF2(ebx, edi, ecx, edx, eax, [esi + 12*4], 0x8d2a4c8a, 20);
      
      MD5STEPF3(eax, edi, ebx, ecx, edx, [esi + 5*4], 0xfffa3942, 4);
      MD5STEPF3(edx, edi, eax, ebx, ecx, [esi + 8*4], 0x8771f681, 11);
      MD5STEPF3(ecx, edi, edx, eax, ebx, [esi + 11*4], 0x6d9d6122, 16);
      MD5STEPF3(ebx, edi, ecx, edx, eax, [esi + 14*4], 0xfde5380c, 23);
      MD5STEPF3(eax, edi, ebx, ecx, edx, [esi + 1*4], 0xa4beea44, 4);
      MD5STEPF3(edx, edi, eax, ebx, ecx, [esi + 4*4], 0x4bdecfa9, 11);
      MD5STEPF3(ecx, edi, edx, eax, ebx, [esi + 7*4], 0xf6bb4b60, 16);
      MD5STEPF3(ebx, edi, ecx, edx, eax, [esi + 10*4], 0xbebfbc70, 23);
      MD5STEPF3(eax, edi, ebx, ecx, edx, [esi + 13*4], 0x289b7ec6, 4);
      MD5STEPF3(edx, edi, eax, ebx, ecx, [esi + 0*4], 0xeaa127fa, 11);
      MD5STEPF3(ecx, edi, edx, eax, ebx, [esi + 3*4], 0xd4ef3085, 16);
      MD5STEPF3(ebx, edi, ecx, edx, eax, [esi + 6*4], 0x04881d05, 23);
      MD5STEPF3(eax, edi, ebx, ecx, edx, [esi + 9*4], 0xd9d4d039, 4);
      MD5STEPF3(edx, edi, eax, ebx, ecx, [esi + 12*4], 0xe6db99e5, 11);
      MD5STEPF3(ecx, edi, edx, eax, ebx, [esi + 15*4], 0x1fa27cf8, 16);
      MD5STEPF3(ebx, edi, ecx, edx, eax, [esi + 2*4], 0xc4ac5665, 23);
      
      MD5STEPF4(eax, edi, ebx, ecx, edx, [esi + 0*4], 0xf4292244, 6);
      MD5STEPF4(edx, edi, eax, ebx, ecx, [esi + 7*4], 0x432aff97, 10);
      MD5STEPF4(ecx, edi, edx, eax, ebx, [esi + 14*4], 0xab9423a7, 15);
      MD5STEPF4(ebx, edi, ecx, edx, eax, [esi + 5*4], 0xfc93a039, 21);
      MD5STEPF4(eax, edi, ebx, ecx, edx, [esi + 12*4], 0x655b59c3, 6);
      MD5STEPF4(edx, edi, eax, ebx, ecx, [esi + 3*4], 0x8f0ccc92, 10);
      MD5STEPF4(ecx, edi, edx, eax, ebx, [esi + 10*4], 0xffeff47d, 15);
      MD5STEPF4(ebx, edi, ecx, edx, eax, [esi + 1*4], 0x85845dd1, 21);
      MD5STEPF4(eax, edi, ebx, ecx, edx, [esi + 8*4], 0x6fa87e4f, 6);
      MD5STEPF4(edx, edi, eax, ebx, ecx, [esi + 15*4], 0xfe2ce6e0, 10);
      MD5STEPF4(ecx, edi, edx, eax, ebx, [esi + 6*4], 0xa3014314, 15);
      MD5STEPF4(ebx, edi, ecx, edx, eax, [esi + 13*4], 0x4e0811a1, 21);
      MD5STEPF4(eax, edi, ebx, ecx, edx, [esi + 4*4], 0xf7537e82, 6);
      MD5STEPF4(edx, edi, eax, ebx, ecx, [esi + 11*4], 0xbd3af235, 10);
      MD5STEPF4(ecx, edi, edx, eax, ebx, [esi + 2*4], 0x2ad7d2bb, 15);
      MD5STEPF4(ebx, edi, ecx, edx, eax, [esi + 9*4], 0xeb86d391, 21);

      /* Copy the result for output. */
      mov edi, buf

      add [edi     ], eax
      add [edi +  4], ebx
      add [edi +  8], ecx
      add [edi + 12], edx
    }
}   

#endif /* WIN32 */
#endif /* ASM_MD5 */
