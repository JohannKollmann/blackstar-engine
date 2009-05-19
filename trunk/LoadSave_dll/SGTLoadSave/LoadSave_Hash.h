 typedef unsigned long long  uint64; // etwaig __int64 == long long
 typedef unsigned char       uchar;
 
 uint64 fnFNV( void* pBuffer, size_t nByteLen )
 {
   uint64 nHashVal    = 0xcbf29ce484222325ULL,
          nMagicPrime = 0x00000100000001b3ULL;
 
   uchar* pFirst = ( uchar* )( pBuffer ),
        * pLast  = pFirst + nByteLen;
 
   while( pFirst < pLast )
     nHashVal ^= *pFirst++,
     nHashVal *= nMagicPrime;
 
   return nHashVal;
 }
