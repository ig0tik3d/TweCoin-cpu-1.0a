#include "cpuminer-config.h"
#include "miner.h"

#include <string.h>
#include <stdint.h>
#include "sphlib/sph_fugue.h"
#include "hamsi256/simd-2/hamsi.h"
#include "sphlib/sph_panama.h"
#include "sphlib/sph_shavite.h"	
//-----------------------------------------------------------------------------------------
typedef struct {
	sph_fugue256_context fugue1;
    sph_shavite256_context shavite1;
    sph_panama_context panama1;
	hashStateHM hm1;
} twe_context_holder;
//-----------------------------------------------------------------------------------------
twe_context_holder base_contexts;
//=========================================================================================
void init_twehash_contexts()
{
	 sph_fugue256_init(&base_contexts.fugue1);
	 sph_shavite256_init(&base_contexts.shavite1);
	 sph_panama_init(&base_contexts.panama1);
	 InitHM(&base_contexts.hm1,256);
}
//=========================================================================================
inline void twehash(void *output, const void *input)
{	
    unsigned char hash[128];
    memset(hash, 0, 128);  
    twe_context_holder ctx;	
	
	memcpy(&ctx, &base_contexts, sizeof(base_contexts));
	
    sph_fugue256(&ctx.fugue1, input, 80);
    sph_fugue256_close(&ctx.fugue1, hash); 
    
    sph_shavite256(&ctx.shavite1, hash, 64);
    sph_shavite256_close(&ctx.shavite1, hash + 64);
	
    // hamsi256 simd-2 		
	UpdateHM(&ctx.hm1,(const BitSequence *)(hash+64),512);
	FinalHM(&ctx.hm1,( BitSequence *)hash);
    
    sph_panama(&ctx.panama1, hash, 64);
    sph_panama_close(&ctx.panama1, hash + 64);

    memcpy(output, hash + 64, 32);
}
//=========================================================================================
int scanhash_twe(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
    uint32_t max_nonce, unsigned long *hashes_done)
{
    uint32_t n = pdata[19] - 1;
    const uint32_t first_nonce = pdata[19];
    const uint32_t Htarg = ptarget[7];

    uint32_t hash64[8] __attribute__((aligned(32)));
    uint32_t endiandata[32];

    //we need bigendian data...
    //lessons learned: do NOT endianchange directly in pdata, this will all proof-of-works be considered as stale from minerd.... 
    int kk=0;
    for (; kk < 32; kk++)
    {
	      be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
    };

    do {
	      pdata[19] = ++n;
	      be32enc(&endiandata[19], n); 
	      twehash(hash64, &endiandata);
            if (((hash64[7]&0xFFFFFF00)==0) && 
			      fulltest(hash64, ptarget)) {
                *hashes_done = n - first_nonce + 1;
		      return true;
	      }
    } while (n < max_nonce && !work_restart[thr_id].restart);

    *hashes_done = n - first_nonce + 1;
    pdata[19] = n;
    return 0;
}
