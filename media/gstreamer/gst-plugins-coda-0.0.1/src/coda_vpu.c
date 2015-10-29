#include "coda_vpu.h"
#include "blackbird.h"
#include "VpuJpegTable.h"


#define	VpuReadReg(offset)	HAL_GET_UINT32((guint *)(handle->RegBase+offset))
#define VpuWriteReg(offset, value) HAL_PUT_UINT32((guint *)(handle->RegBase+offset), value)

const guchar cDefHuffBits[4][16] = 
{
	{	// DC index 0 (Luminance DC)
		0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}
	,
	{	// AC index 0 (Luminance AC)
		0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
			0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D
	}
	,
	{	// DC index 1 (Chrominance DC)
		0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
	}
	,
	{	// AC index 1 (Chrominance AC)
		0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
			0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77
	}
};

const guchar cDefHuffVal[4][162] = 
{
	{	// DC index 0 (Luminance DC)
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B
	}
	,
	{	// AC index 0 (Luminance AC)
		0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
		0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
		0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
		0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
		0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
		0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
		0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
		0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
		0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
		0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
		0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
		0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
		0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
		0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
		0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
		0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
		0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
		0xF9, 0xFA
	}
	,
	{	// DC index 1 (Chrominance DC)
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0A, 0x0B
	}
	,
	{	// AC index 1 (Chrominance AC)
		0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
		0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
		0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
		0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0,
		0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
		0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
		0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38,
		0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
		0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
		0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
		0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
		0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,
		0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
		0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
		0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
		0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2,
		0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
		0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
		0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
		0xF9, 0xFA
	}
};

#define INT_BIT (8 * sizeof(int))

#define AV_RL32(x)                           \
    ((((const guchar*)(x))[3] << 24) |         \
     (((const guchar*)(x))[2] << 16) |         \
     (((const guchar*)(x))[1] <<  8) |         \
      ((const guchar*)(x))[0])

static unsigned int zero_extend(unsigned int val, unsigned int bits)
{
    return (val << (INT_BIT - bits)) >> (INT_BIT - bits);
}

#define OPEN_READER(name, gb)\
        unsigned int name##_index= (gb)->index;\
        int name##_cache= 0;\
        guchar *name##_s;

#define UPDATE_CACHE(name, gb)\
        name##_s = ((guchar *)(gb)->buffer)+(name##_index>>3);\
        name##_cache= AV_RL32(name##_s) >> (name##_index&0x07);     
 
#define SHOW_UBITS(name, gb, num)\
        zero_extend(name##_cache, num)

#define SKIP_COUNTER(name, gb, num)\
        name##_index += (num);

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define CLOSE_READER(name, gb)\
        (gb)->index= name##_index;

static int get_bits_count(const GetBitContext *s)
{
    return s->index;
}

static void init_get_bits(GetBitContext *s,
                   const guchar *buffer, int bit_size)
{
    int buffer_size= (bit_size+7)>>3;
    if(buffer_size < 0 || bit_size < 0) {
        buffer_size = bit_size = 0;
        buffer = NULL;
    }

    s->buffer= buffer;
    s->size_in_bits= bit_size;
    s->buffer_end= buffer + buffer_size;
    s->index=0;

}

static int get_bits_left(GetBitContext *gb)
{
    //return gb->size_in_bits - get_bits_count(gb);
	return gb->size_in_bits - gb->index;
}

static unsigned int show_bits(GetBitContext *s, int n){
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_UBITS(re, s, n);
    return tmp;
}

static unsigned int get_bits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}

int find_start_code(JpgDecInfo *jpg)
{
	unsigned char byte0, byte1;
	unsigned short word;
	
	while(1) 
	{
		word = show_bits(&jpg->gbc, 16);
		byte0 = word & 0xff;
		byte1 = ( word >> 8 ) & 0xff;
		word = (byte0 << 8) | byte1;
		if ((word > 0xFF00) && (word < 0xFFFF))
			break;
		
		if (get_bits_left(&jpg->gbc) <= 0) 
		{
			////printf("hit end of stream\n");	
			return 0;
		}
		
		get_bits(&jpg->gbc, 8);
	}
	
	return 1;
}

int check_start_code(JpgDecInfo *jpg)
{
	if (show_bits(&jpg->gbc, 8) == 0xFF)
		return 1;
	else
		return 0;
}

int decode_app_header(JpgDecInfo *jpg)
{
	int length;
	guchar byte0, byte1;
	
	byte0 = get_bits(&jpg->gbc, 8);
	byte1 = get_bits(&jpg->gbc, 8);  
	length = (byte0 << 8)|byte1;
	  	
	length -= 2;
	
	while(length-- > 0)
	{
		get_bits(&jpg->gbc, 8);
	}
	
	if (!check_start_code(jpg)) 
	{
		find_start_code(jpg);
		return 0;
	}
	
	return 1;
}

int decode_dri_header(JpgDecInfo *jpg)
{
	guchar byte0, byte1;
    //Length, Lr
    get_bits(&jpg->gbc, 16);
	
	byte0 = get_bits(&jpg->gbc, 8);
	byte1 = get_bits(&jpg->gbc, 8);  
	jpg->rstIntval = (byte0 << 8)|byte1;	
   	
	if (!check_start_code(jpg)) 
	{
		find_start_code(jpg);
		return 0;
	}
	
	return 1;
}

int decode_dqt_header(JpgDecInfo *jpg)
{
	int Pq;
	int Tq;
	int i;
	
	// Lq, Length of DQT
	get_bits(&jpg->gbc, 16);
	
	do {
		// Pq, Quantization Precision
        	Pq = get_bits(&jpg->gbc, 4);
		// Tq, Quantization table destination identifier
		Tq = get_bits(&jpg->gbc, 4);
		
		for (i=0; i<64; i++)
            		jpg->qMatTab[Tq][i] = get_bits(&jpg->gbc, 8);
	} while(!check_start_code(jpg));
	
	if (Pq != 0) // not 8-bit
	{	
		////printf("pq is not set to zero\n");
		return 0;
	} 
	return 1;
}

int decode_dth_header(JpgDecInfo *jpg)
{
	int Tc;
	int Th;
	int ThTc;
	int bitCnt;
	int i;
	
	// Length, Lh
	get_bits(&jpg->gbc, 16);
	
	do {
		// Table class - DC, AC
        	Tc = get_bits(&jpg->gbc, 4);
		// Table destination identifier
		Th = get_bits(&jpg->gbc, 4);
       		// DC_ID0 (b'00) -> 0
		// AC_ID0 (b'10) -> 1
		// DC_ID1 (b'01) -> 2
		// AC_ID1 (b'11) -> 3
		ThTc = ((Th&1)<<1) | (Tc&1);
		
		// Get Huff Bits list
		bitCnt = 0;
		for (i=0; i<16; i++) 
		{
            		jpg->huffBits[ThTc][i] = get_bits(&jpg->gbc, 8);
            		bitCnt += jpg->huffBits[ThTc][i];
           
			if (cDefHuffBits[ThTc][i] != jpg->huffBits[ThTc][i])
				jpg->userHuffTab = 1;
		}
		
		// Get Huff Val list
		for (i=0; i<bitCnt; i++) 
		{
            		jpg->huffVal[ThTc][i] = get_bits(&jpg->gbc, 8);
			
			if (cDefHuffVal[ThTc][i] != jpg->huffVal[ThTc][i])
				jpg->userHuffTab = 1;
		}
	} while(!check_start_code(jpg));
	
	return 1;
}

int decode_sof_header(JpgDecInfo *jpg)
{
	int samplePrecision;
	int sampleFactor;
	int i;
	int Tqi;
	int compID;
	int hSampFact[3];
	int vSampFact[3];
	int picX, picY;
	int numComp;
	guchar byte0, byte1;
	
	// LF, Length of SOF
    	get_bits(&jpg->gbc, 16);
	
   	// Sample Precision: Baseline(8), P
   	 samplePrecision = get_bits(&jpg->gbc, 8);
	
	if (samplePrecision != 8) 
	{
		//printf("Sample Precision is not 8\n");
		return 0;
	}
	
	byte0 = get_bits(&jpg->gbc, 8);
	byte1 = get_bits(&jpg->gbc, 8);  	
    	picY = (byte0 <<8)|byte1;
	if (picY > MAX_VSIZE) 
	{
		//printf("Picture Vertical Size limits Maximum size\n");
		return 0;
	}
	
	byte0 = get_bits(&jpg->gbc, 8);
	byte1 = get_bits(&jpg->gbc, 8);  	
    	picX = (byte0 <<8)|byte1;
	if (picX > MAX_HSIZE) 
	{
		//printf("Picture Horizontal Size limits Maximum size\n");
		return 0;
	}
	
	//Number of Components in Frame: Nf
	numComp = get_bits(&jpg->gbc, 8);
	if (numComp > 3)
	{
		//printf("Picture Horizontal Size limits Maximum size\n");
	}
	
	for (i=0; i<numComp; i++) 
	{
        // Component ID, Ci 0 ~ 255
		compID = get_bits(&jpg->gbc, 8);
		// Horizontal Sampling Factor, Hi			
		hSampFact[i] = get_bits(&jpg->gbc, 4);
		// Vertical Sampling Factor, Vi
		vSampFact[i] = get_bits(&jpg->gbc, 4);
		// Quantization Table Selector, Tqi
		Tqi = get_bits(&jpg->gbc, 8);
		
		jpg->cInfoTab[i][0] = compID;
		jpg->cInfoTab[i][1] = hSampFact[i];
		jpg->cInfoTab[i][2] = vSampFact[i];
		jpg->cInfoTab[i][3] = Tqi;
	}	
	
	//if ( hSampFact[0]>2 || vSampFact[0]>2 || ( numComp == 3 && ( hSampFact[1]!=1 || hSampFact[2]!=1 || vSampFact[1]!=1 || vSampFact[2]!=1) ) )
		//printf("Not Supported Sampling Factor\n");
	
	if (numComp == 1)
		sampleFactor = SAMPLE_400;
	else
		sampleFactor = ((hSampFact[0]&3)<<2) | (vSampFact[0]&3);
	
	switch(sampleFactor) {
	case SAMPLE_420:
		jpg->format = YCBCR420;
		break;
	case SAMPLE_H422:
		jpg->format = YCBCR422H;
		break;
	case SAMPLE_V422:
		jpg->format = YCBCR422V;
		break;
	case SAMPLE_444:
		jpg->format = YCBCR444;
		break;
	default:	// 4:0:0
		jpg->format = YCBCR400;
	}
	
	jpg->picWidth = picX;
	jpg->picHeight = picY;    
	
	return 1;
}

int decode_sos_header(JpgDecInfo *jpg)
{
	int i, j;
	int len;
	int numComp;
	int compID;
	int ss, se, ah, al;
	int dcHufTblIdx[3];
	int acHufTblIdx[3];
	guchar byte0, byte1;	
	
	// Length, Ls
	byte0 = get_bits(&jpg->gbc, 8);
	byte1 = get_bits(&jpg->gbc, 8);  	
	len = (byte0<<8)|byte1;
	
	jpg->ecsPtr = get_bits_count(&jpg->gbc)/8 + len - 2 ;

    //Number of Components in Scan: Ns
	numComp = get_bits(&jpg->gbc, 8);
	
	for (i=0; i<numComp; i++) {
		// Component ID, Csj 0 ~ 255
        	compID = get_bits(&jpg->gbc, 8);
		// dc entropy coding table selector, Tdj
		dcHufTblIdx[i] = get_bits(&jpg->gbc, 4);
        // ac entropy coding table selector, Taj
        	acHufTblIdx[i] = get_bits(&jpg->gbc, 4);
        
		
        	for (j=0; j<numComp; j++) 
		{
            		if (compID == jpg->cInfoTab[j][0]) 
			{
                		jpg->cInfoTab[j][4] = dcHufTblIdx[i];
                		jpg->cInfoTab[j][5] = acHufTblIdx[i];
            		}
        	}
	}	
	
	// Ss 0
	ss = get_bits(&jpg->gbc, 8);
	// Se 3F
	se = get_bits(&jpg->gbc, 8);
	// Ah 0
	ah = get_bits(&jpg->gbc, 4);
	// Al 0
	al = get_bits(&jpg->gbc, 4);
	
	if ((ss != 0) || (se != 0x3F) || (ah != 0) || (al != 0)) 
	{
		//printf("The Jpeg Image must be another profile\n");
		return 0;
	}
	
	return 1;
}

static void genDecHuffTab(JpgDecInfo *jpg, int tabNum)
{
	unsigned char *huffPtr, *huffBits;
	unsigned int *huffMax, *huffMin;
	
	int ptrCnt =0;
	int huffCode = 0;
	int zeroFlag = 0;
	int dataFlag = 0;
	int i;
	
	huffBits	= jpg->huffBits[tabNum];
	huffPtr		= jpg->huffPtr[tabNum];
	huffMax		= jpg->huffMax[tabNum];
	huffMin		= jpg->huffMin[tabNum];
	
	for (i=0; i<16; i++) 
	{
		if (huffBits[i]) // if there is bit cnt value
		{	
			huffPtr[i] = ptrCnt;
			ptrCnt += huffBits[i];
			huffMin[i] = huffCode;
			huffMax[i] = huffCode + (huffBits[i] - 1);
			dataFlag = 1;
			zeroFlag = 0;
		} 
		else 
		{
			huffPtr[i] = 0xFF;
			huffMin[i] = 0xFFFF;
			huffMax[i] = 0xFFFF;
			zeroFlag = 1;
		}
		
		if (dataFlag == 1) 
		{
			if (zeroFlag == 1)
				huffCode <<= 1;
			else
				huffCode = (huffMax[i] + 1) << 1;
		}
	}

}

int JpegDecodeHeader(JpgDecInfo *jpg)
{
	unsigned int code;
	int i;
	int temp;
	guchar *pTmp;
	guchar byte0, byte1;
	guchar *b = jpg->pHeader;
	int size = jpg->headerSize;
	
	if (!b || !size) {
		return 0;
	}
	
	pTmp = (guchar *)jpg;
	for (i=0; i<sizeof(JpgDecInfo); i++)
		*pTmp++ = 0x00;
	
	pTmp = (guchar *)&jpg->gbc;
	for (i=0; i<sizeof(GetBitContext); i++)
		*pTmp++ = 0x00;
	
	init_get_bits(&jpg->gbc, b, size*8);
	
	// Initialize component information table
	for (i=0; i<4; i++) 
	{
		jpg->cInfoTab[i][0] = 0;
		jpg->cInfoTab[i][1] = 0;
		jpg->cInfoTab[i][2] = 0;
		jpg->cInfoTab[i][3] = 0;
		jpg->cInfoTab[i][4] = 0;
		jpg->cInfoTab[i][5] = 0;
	}
	
	for (;;) 
	{
		if (find_start_code(jpg) == 0)
			return 0;	// StartCode
		
		byte0 = get_bits(&jpg->gbc, 8);
		byte1 = get_bits(&jpg->gbc, 8);
		code = (byte0 << 8)|byte1;
		
		switch (code) {
		case SOI_Marker:
			break;
		case JFIF_CODE:
		case EXIF_CODE:
			decode_app_header(jpg);
			break;
		case DRI_Marker:
			decode_dri_header(jpg);
			break;
		case DQT_Marker:
			decode_dqt_header(jpg);
			break;
		case DHT_Marker:
			decode_dth_header(jpg);				
			break;
		case SOF_Marker:
			decode_sof_header(jpg);
			break;
		case SOS_Marker:
			decode_sos_header(jpg);
			goto DONE_DEC_HEADER;
			break;
		case EOI_Marker:
			goto DONE_DEC_HEADER;
		default:
			switch (code&0xFFF0) 
			{
			case 0xFFE0:	// 0xFFEX
			case 0xFFF0:	// 0xFFFX
				if (get_bits_left(&jpg->gbc) <=0 ) {
					return 0;
				} 
				else 
				{
					decode_app_header(jpg);
					break;
				}
			default:
				//printf("code = [%x]\n", code);
				return	0;
			}
			break;
		}
	}
	
DONE_DEC_HEADER:
	// Generate Huffman table information
	for(i=0; i<4; i++)
		genDecHuffTab(jpg, i);
	
	// Q Idx
    temp =             jpg->cInfoTab[0][3];
    temp = temp << 1 | jpg->cInfoTab[1][3];
    temp = temp << 1 | jpg->cInfoTab[2][3];
	jpg->Qidx = temp;
	
	
	// Huff Idx[DC, AC]
    temp =             jpg->cInfoTab[0][4];
    temp = temp << 1 | jpg->cInfoTab[1][4];
    temp = temp << 1 | jpg->cInfoTab[2][4];
	jpg->huffDcIdx = temp;
    
    temp =             jpg->cInfoTab[0][5];
    temp = temp << 1 | jpg->cInfoTab[1][5];
    temp = temp << 1 | jpg->cInfoTab[2][5];
	jpg->huffAcIdx = temp;
	
	
	switch(jpg->format)
	{
	case YCBCR420:
		jpg->busReqNum = 2;
		jpg->mcuBlockNum = 6;
		jpg->compNum = 3;
		jpg->compInfo[0] = 10;
		jpg->compInfo[1] = 5;
		jpg->compInfo[2] = 5;	
		jpg->alignedWidth = ((jpg->picWidth+15)&~15);
		jpg->alignedHeight = ((jpg->picHeight+15)&~15);
		break;
	case YCBCR422H:
		jpg->busReqNum = 3;
		jpg->mcuBlockNum = 4;
		jpg->compNum = 3;
		jpg->compInfo[0] = 9;
		jpg->compInfo[1] = 5;
		jpg->compInfo[2] = 5;		
		jpg->alignedWidth = ((jpg->picWidth+15)&~15);
		jpg->alignedHeight = ((jpg->picHeight+7)&~7);
		break;
	case YCBCR422V:
		jpg->busReqNum = 3;
		jpg->mcuBlockNum = 4;
		jpg->compNum = 3;
		jpg->compInfo[0] = 6;
		jpg->compInfo[1] = 5;
		jpg->compInfo[2] = 5;		
		jpg->alignedWidth = ((jpg->picWidth+7)&~7);
		jpg->alignedHeight = ((jpg->picHeight+15)&~15);
		break;
	case YCBCR444:
		jpg->busReqNum = 4;
		jpg->mcuBlockNum = 3;
		jpg->compNum = 3;
		jpg->compInfo[0] = 5;
		jpg->compInfo[1] = 5;
		jpg->compInfo[2] = 5;
		jpg->alignedWidth = ((jpg->picWidth+7)&~7);
		jpg->alignedHeight = ((jpg->picHeight+7)&~7);
		break;
	case YCBCR400:
		jpg->busReqNum = 4;
		jpg->mcuBlockNum = 1;
		jpg->compNum = 1;
		jpg->compInfo[0] = 5;
		jpg->compInfo[1] = 0;
		jpg->compInfo[2] = 0;
		jpg->alignedWidth = ((jpg->picWidth+7)&~7);
		jpg->alignedHeight = ((jpg->picHeight+7)&~7);
		break;
	}
	
	return 1;
}

gint JpgDecHuffTabSetUp(DecInstCtl *handle, JpgDecInfo *pJpgDecInfo)
{
	gint i, j;
	gint HuffData;	// 16BITS
	gint HuffLength;
	gint temp;

	// MIN Tables
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x003);

	//DC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMin[0][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
	}

	//DC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMin[2][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
	}

	//AC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMin[1][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
	}

	//AC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMin[3][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));	// 32-bit
	}

	// MAX Tables
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x403);
	VpuWriteReg(MJPEG_HUFF_ADDR_REG, 0x440);

	//DC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMax[0][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
	}

	//DC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMax[2][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
	}

	//AC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMax[1][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
	}

	//AC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffMax[3][j];
		temp = (HuffData & 0x8000) >> 15;
		temp = (temp << 15) | (temp << 14) | (temp << 13) | (temp << 12) | (temp << 11) | (temp << 10) | (temp << 9) | (temp << 8) | (temp << 7 ) | (temp << 6) | (temp <<5) | (temp<<4) | (temp<<3) | (temp<<2) | (temp<<1)| (temp) ;
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFF) << 16) | HuffData));
	}

	// PTR Tables
	VpuWriteReg (MJPEG_HUFF_CTRL_REG, 0x803);
	VpuWriteReg (MJPEG_HUFF_ADDR_REG, 0x880);

	//DC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffPtr[0][j];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	//DC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffPtr[2][j];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	//AC Luma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffPtr[1][j];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	//AC Chroma
	for(j=0; j<16; j++)
	{
		HuffData = pJpgDecInfo->huffPtr[3][j];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	// VAL Tables
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0xC03);

	// VAL DC Luma
	HuffLength = 0;
	for(i=0; i<12; i++)
        HuffLength += pJpgDecInfo->huffBits[0][i];
	for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Luma)
		HuffData = pJpgDecInfo->huffVal[0][i];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	for (i=0; i<12-HuffLength; i++) {
		VpuWriteReg(MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
	}

	// VAL DC Chroma
	HuffLength = 0;
	for(i=0; i<12; i++)
        HuffLength += pJpgDecInfo->huffBits[2][i];
	for (i=0; i<HuffLength; i++) {	// 8-bit, 12 row, 1 category (DC Chroma)
		HuffData = pJpgDecInfo->huffVal[2][i];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	for (i=0; i<12-HuffLength; i++) {
		VpuWriteReg(MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
	}

	// VAL AC Luma
	HuffLength = 0;
	for(i=0; i<162; i++)
        HuffLength += pJpgDecInfo->huffBits[1][i];
	for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Luma)
		HuffData = pJpgDecInfo->huffVal[1][i];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}
	for (i=0; i<162-HuffLength; i++) {
		VpuWriteReg(MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
	}

	// VAL AC Chroma
	HuffLength = 0;
	for(i=0; i<162; i++)
        HuffLength += pJpgDecInfo->huffBits[3][i];
	for (i=0; i<HuffLength; i++) {	// 8-bit, 162 row, 1 category (AC Chroma)
		HuffData = pJpgDecInfo->huffVal[3][i];
		temp = (HuffData & 0x80) >> 7;
		temp = (temp<<23)|(temp<<22)|(temp<<21)|(temp<<20)|(temp<<19)|(temp<<18)|(temp<<17)|(temp<<16)|(temp<<15)|(temp<<14)|(temp<<13)|(temp<<12)|(temp<<11)|(temp<<10)|(temp<<9)|(temp<<8)|(temp<<7)|(temp<<6)|(temp<<5)|(temp<<4)|(temp<<3)|(temp<<2)|(temp<<1)|(temp);
		VpuWriteReg (MJPEG_HUFF_DATA_REG, (((temp & 0xFFFFFF) << 8) | HuffData));
	}

	for (i=0; i<162-HuffLength; i++) {
		VpuWriteReg(MJPEG_HUFF_DATA_REG, 0xFFFFFFFF);
	}

	// end SerPeriHuffTab
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x000);

	return 1;
}

gint JpgDecQMatTabSetUp(DecInstCtl *handle, JpgDecInfo *pJpgDecInfo)
{
	gint i;
//	gint line = 0;
	gint table;
	gint val;

	// SetPeriQMatTab
	// Comp 0
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x03);
	table = pJpgDecInfo->cInfoTab[0][3];
	for (i=0; i<64; i++) {
		val = pJpgDecInfo->qMatTab[table][i];
		VpuWriteReg(MJPEG_QMAT_DATA_REG, val);
	}
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x00);

	// Comp 1
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x43);
	table = pJpgDecInfo->cInfoTab[1][3];
	for (i=0; i<64; i++) {
		val = pJpgDecInfo->qMatTab[table][i];
		VpuWriteReg(MJPEG_QMAT_DATA_REG, val);
	}
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x00);

	// Comp 2
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x83);
	table = pJpgDecInfo->cInfoTab[2][3];
	for (i=0; i<64; i++) {
		val = pJpgDecInfo->qMatTab[table][i];
		VpuWriteReg(MJPEG_QMAT_DATA_REG, val);
	}
	VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x00);
	return 1;
}

void JpgDecGramSetup(DecInstCtl *handle, JpgDecInfo *pJpgDecInfo, guint StreamStartAddr)
{
	gint dExtBitBufCurPos;
	gint dExtBitBufBaseAddr;
	gint dMibStatus;

	if (pJpgDecInfo->seqInited==1)
		return;

	dMibStatus			= 1;
	dExtBitBufCurPos	= 0;
	dExtBitBufBaseAddr	= StreamStartAddr;
	
	VpuWriteReg(MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);
	VpuWriteReg(MJPEG_BBC_EXT_ADDR_REG, dExtBitBufBaseAddr + (dExtBitBufCurPos << 8));
	VpuWriteReg(MJPEG_BBC_INT_ADDR_REG, (dExtBitBufCurPos & 1) << 6);
	VpuWriteReg(MJPEG_BBC_DATA_CNT_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	VpuWriteReg(MJPEG_BBC_COMMAND_REG, (STREAM_ENDIAN << 1) | 0);
	
	while (dMibStatus == 1) {
		dMibStatus = VpuReadReg(MJPEG_BBC_BUSY_REG);
	}
	
	dMibStatus			= 1;
	dExtBitBufCurPos	= dExtBitBufCurPos + 1;
		
	VpuWriteReg(MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);
	VpuWriteReg(MJPEG_BBC_EXT_ADDR_REG, dExtBitBufBaseAddr + (dExtBitBufCurPos << 8));
	VpuWriteReg(MJPEG_BBC_INT_ADDR_REG, (dExtBitBufCurPos & 1) << 6);
	VpuWriteReg(MJPEG_BBC_DATA_CNT_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	VpuWriteReg(MJPEG_BBC_COMMAND_REG, (STREAM_ENDIAN << 1) | 0);
	
	while (dMibStatus == 1) {
		dMibStatus = VpuReadReg(MJPEG_BBC_BUSY_REG);
	}
	
	dMibStatus			= 1;
	dExtBitBufCurPos	= dExtBitBufCurPos + 1;
	
	VpuWriteReg(MJPEG_BBC_CUR_POS_REG, dExtBitBufCurPos);	// next uint page pointe

	VpuWriteReg(MJPEG_BBC_CTRL_REG, ((STREAM_ENDIAN & 3) << 1) | 1);	

	VpuWriteReg(MJPEG_GBU_WD_PTR_REG, 0);
	
	VpuWriteReg(MJPEG_GBU_BBSR_REG, 0);
	VpuWriteReg(MJPEG_GBU_BBER_REG, ((256 / 4) * 2) - 1);
	
	VpuWriteReg(MJPEG_GBU_BBIR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	VpuWriteReg(MJPEG_GBU_BBHR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
	
	VpuWriteReg(MJPEG_GBU_CTRL_REG, 4);
	VpuWriteReg(MJPEG_GBU_FF_RPTR_REG, 0); 
	pJpgDecInfo->seqInited=1;
}

/******************************************************************************

    JPEG specific Helper

******************************************************************************/
static void jpgGetHuffTable(EncMjpgParam *param)
{
    // Rearrange and insert pre-defined Huffman table to deticated variable.
	memcpy(&param->huffBits[AC_TABLE_INDEX0], &lumaDcBits, 16);	// Luma DC BitLength
	memcpy(&param->huffVal[AC_TABLE_INDEX0], &lumaDcValue, 16);	// Luma DC HuffValue

	memcpy(&param->huffBits[AC_TABLE_INDEX0], &lumaAcBits, 16);	// Luma DC BitLength
	memcpy(&param->huffVal[AC_TABLE_INDEX0], &lumaAcValue, 162);	// Luma DC HuffValue

	memcpy(&param->huffBits[DC_TABLE_INDEX1], &chromaDcBits, 16);	// Chroma DC BitLength
	memcpy(&param->huffVal[DC_TABLE_INDEX1], &chromaDcValue, 16);	// Chroma DC HuffValue

	memcpy(&param->huffBits[AC_TABLE_INDEX1], &chromaAcBits, 16);	// Chroma AC BitLength
	memcpy(&param->huffVal[AC_TABLE_INDEX1], &chromaAcValue, 162); // Chorma AC HuffValue
}

static void jpgGetQMatrix(EncMjpgParam *param)
{
	// Rearrange and insert pre-defined Q-matrix to deticated variable.
	memcpy(&param->qMatTab[DC_TABLE_INDEX0], &lumaQ2, 64);
	memcpy(&param->qMatTab[AC_TABLE_INDEX0], &chromaBQ2, 64);
	memcpy(&param->qMatTab[DC_TABLE_INDEX1], &param->qMatTab[DC_TABLE_INDEX0], 64);
	memcpy(&param->qMatTab[AC_TABLE_INDEX1], &param->qMatTab[AC_TABLE_INDEX0], 64);
}

static void jpgGetCInfoTable(EncMjpgParam *param)
{
	int format = param->mjpg_sourceFormat;
    memcpy(&param->cInfoTab[format], &cInfoTable[format], 4*6);
}

guint GetNewNormalReadPtr(void *VPUhandle, guchar CodecInst, guchar dec_enc)
{
	
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return ( VpuReadReg(BIT_RD_PTR_0 + 8 * CodecInst)  );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return ( VpuReadReg(BIT_RD_PTR_0 + 8 * CodecInst)  );
	}
    
}

guint GetNewReadPtr(void *VPUhandle, guchar CodecInst, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return ( VpuReadReg(BIT_EXACT_RD_PTR) + 1 );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return ( VpuReadReg(BIT_EXACT_RD_PTR) + 1 );		
	}
}

guint GetNewWritePtr(void *VPUhandle, guchar CodecInst, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return ( VpuReadReg(BIT_WR_PTR_0 + 8 * CodecInst)  );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return ( VpuReadReg(BIT_WR_PTR_0 + 8 * CodecInst)  );
	}
}

guint GetVPUIntStatus(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return ( VpuReadReg(BIT_INT_REASON) );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return ( VpuReadReg(BIT_INT_REASON) );
	}
}

void EnableVPUInt(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		VpuWriteReg(BIT_INT_ENABLE, VPU_ISR_ALL);
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		VpuWriteReg(BIT_INT_ENABLE, VPU_ISR_ALL);
	}
}

void SetVPUIntMask(void *VPUhandle, guint mask, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		VpuWriteReg(BIT_INT_ENABLE, mask);
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		VpuWriteReg(BIT_INT_ENABLE, mask);
	}
}

guint GetVPUIntMask(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return ( VpuReadReg(BIT_INT_ENABLE) );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return ( VpuReadReg(BIT_INT_ENABLE) );
	}
	
}

void ClrVPUBitInt(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		VpuWriteReg( BIT_INT_CLEAR, 0x1 );
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		VpuWriteReg( BIT_INT_CLEAR, 0x1 );
	}
}

void ClrVPUIntStatus(void *VPUhandle, guint IntStatus, guint IntReason, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		VpuWriteReg(BIT_INT_REASON, IntStatus & (~IntReason));
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		VpuWriteReg(BIT_INT_REASON, IntStatus & (~IntReason));		
	}

}

gint JPU_IsBusy(void *VPUhandle, guchar dec_enc)
{
	guint val;

	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		val = VpuReadReg(MJPEG_PIC_STATUS_REG);
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		val = VpuReadReg(MJPEG_PIC_STATUS_REG);		
	}
	
	if (val&0x01 || val&0x02)
		return 0;

	return 1; 
}

void JPU_ClrStatus(void *VPUhandle, guchar dec_enc, guint val)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		if (val != 0)
			VpuWriteReg(MJPEG_PIC_STATUS_REG, val);		
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		if (val != 0)
			VpuWriteReg(MJPEG_PIC_STATUS_REG, val);	
	}

}

guint JPU_GetStatus(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return VpuReadReg(MJPEG_PIC_STATUS_REG);
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return VpuReadReg(MJPEG_PIC_STATUS_REG);
	}
}

#if 0
CodecRetCode CheckDecInstanceValidity(CodecInst * pCodecInst)
{
	CodecRetCode ret;

	if (!pCodecInst->inUse) {
		return RETCODE_INVALID_HANDLE;
	}
	if (pCodecInst->codecMode != AVC_DEC && 
		pCodecInst->codecMode != VC1_DEC &&
		pCodecInst->codecMode != MP2_DEC &&
		pCodecInst->codecMode != MP4_DEC &&
		pCodecInst->codecMode != AVS_DEC &&
		pCodecInst->codecMode != H263_DEC &&
		pCodecInst->codecMode != DIV3_DEC &&
		pCodecInst->codecMode != RV_DEC &&
		pCodecInst->codecMode != MJPG_DEC) {
		return RETCODE_INVALID_HANDLE;
	}

	return RETCODE_SUCCESS;
}

void FreeCodecInstance(CodecInst * pCodecInst)
{
	pCodecInst->inUse = 0;
}
#endif

void VPURegInit(void *VPUhandle, gushort start_addr, guchar len, guchar dec_enc)
{
	/* Set the registers value 0 from 0x180 to 0x1FC */
	guchar i;

	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		for ( i = 0; i < len; i++ ) {
			VpuWriteReg(start_addr+i*4, 0);
		}		
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		for ( i = 0; i < len; i++ ) {
			VpuWriteReg(start_addr+i*4, 0);
		}		
	}

}

void BitIssueCommand(DecInstCtl *handle, gint instIdx, gint cdcMode, gint cmd)
{
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_RUN_INDEX, instIdx);
	VpuWriteReg(BIT_RUN_COD_STD, cdcMode);
	VpuWriteReg(BIT_RUN_COMMAND, cmd);
}

void EncBitIssueCommand(EncInstCtl *handle, gint instIdx, gint cdcMode, gint cmd)
{
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_RUN_INDEX, instIdx);
	VpuWriteReg(BIT_RUN_COD_STD, cdcMode);
	VpuWriteReg(BIT_RUN_COMMAND, cmd);
}


void GetInstIndex(DecInstCtl *handle, guchar *index)
{
	*index = VpuReadReg(BIT_RUN_INDEX);
}

guint VPU_GetDispFlag(DecInstCtl *handle, guchar inst_index)
{
	return (VpuReadReg(BIT_FRAME_DIS_FLAG_0+4*inst_index));
}

CodecRetCode CheckDecOpenParam( DecSeqOpenParam * pop)
{
	if (pop == 0) {
		GST_ERROR("set param null!\n");
		return RETCODE_INVALID_PARAM;
	}
	if (pop->streamBufStartAddr % 4) { // not 4-byte aligned
		GST_ERROR("stream buffer start address not 4-byte aligned, %x\n", pop->streamBufStartAddr);
		return RETCODE_INVALID_PARAM;
	}
	if (pop->streamBufSize % 1024 ||
			pop->streamBufSize < 1024 ||
			pop->streamBufSize > 16383 * 1024) {
		GST_ERROR("stream buffer size not valid %x\n", pop->streamBufSize);
		return RETCODE_INVALID_PARAM;
	}
	
	return RETCODE_SUCCESS;
}

#if 0
void SetDecodeFrameParam(DecFrameCfg *param)
{
	param->chunkSize = 0;
	param->dispDelayedPic = 0;
	param->iframeSearchEnable = 0;
	param->picStartByteOffset = 0;
	param->picStreamBufferAddr = 0;
	param->prescanEnable = 0;
	param->prescanMode = 0;
	param->skipframeMode = 0;
	param->skipframeNum = 0;
	
	return;
}
#endif

gint DecBitstreamBufEmpty(DecInstCtl *handle, guchar inst_index)
{
	return ( VpuReadReg(BIT_RD_PTR_0 + 8*inst_index) == VpuReadReg(BIT_WR_PTR_0 + 8*inst_index) );
}

#if 0
void SetParaSet(DecHandle handle, INT32 paraSetType, DecParamSet * para)
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	INT32 i;
	UINT32 * src;

	pCodecInst = handle;
	pDecInfo = &(pCodecInst->decInfo);

	src = para->paraSet;
	for (i = 0; i < para->size; i += 4) {
		VpuWriteReg(paraBuffer + i, *src++);
	}
	VpuWriteReg(CMD_DEC_PARA_SET_TYPE, paraSetType); // 0: SPS, 1: PPS
	VpuWriteReg(CMD_DEC_PARA_SET_SIZE, para->size);
	BitIssueCommand(handle, pCodecInst->instIndex, pCodecInst->codecMode, DEC_PARA_SET);
	while (VpuReadReg(BIT_BUSY_FLAG));
}
#endif

/* the VPU API function */

guchar VPU_IsBusy(void *VPUhandle, guchar dec_enc)
{
	if (dec_enc){
		DecInstCtl *handle;
		handle=(DecInstCtl *)VPUhandle;
		return VpuReadReg(BIT_BUSY_FLAG) != 0;
	}else{
		EncInstCtl *handle;
		handle=(EncInstCtl*)VPUhandle;	
		return VpuReadReg(BIT_BUSY_FLAG) != 0;
	}
}

gint JpgEncGenHuffTab(JpgEncInfo *pJpgInfo, gint tabNum)
{
	gint p, i, l, lastp, si, maxsymbol;
	gint huffsize[256];
	gint huffcode[256];
	gint code;
	guchar *bitleng, *huffval;
	gint *ehufco, *ehufsi;

	bitleng	= pJpgInfo->pHuffBits[tabNum];
	huffval	= pJpgInfo->pHuffVal[tabNum];
	ehufco	= pJpgInfo->huffCode[tabNum];
	ehufsi	= pJpgInfo->huffSize[tabNum];
	maxsymbol = tabNum & 1 ? 256 : 16;

	/* Figure C.1: make table of Huffman code length for each symbol */

	p = 0;
	for (l=1; l<=16; l++) {
		i = bitleng[l-1];
		if (i < 0 || p + i > maxsymbol)
			return 0;
		while (i--)
			huffsize[p++] = l;
	}
	lastp = p;

	/* Figure C.2: generate the codes themselves */
	/* We also validate that the counts represent a legal Huffman code tree. */

	code = 0;
	si = huffsize[0];
	p = 0;
	while (p < lastp) {
		while (huffsize[p] == si) {
			huffcode[p++] = code;
			code++;
		}
		if (code >= (1 << si))
			return 0;
		code <<= 1;
		si++;
	}

	/* Figure C.3: generate encoding tables */
	/* These are code and size indexed by symbol value */

	memset(ehufsi, 0, sizeof(gint) * 256);
	memset(ehufco, 0, sizeof(gint) * 256);

	for (p=0; p<lastp; p++) {
		i = huffval[p];
		if (i < 0 || i >= maxsymbol || ehufsi[i])
			return 0;
		ehufco[i] = huffcode[p];
		ehufsi[i] = huffsize[p];
	}

	return 1;
}


gint JpgEncLoadHuffTab(EncInstCtl *handle, JpgEncInfo *pJpgInfo)
{
	gint i, j, t;
	gint huffData;

	for (i=0; i<4; i++)
		JpgEncGenHuffTab(pJpgInfo, i);

	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x3);

	for (j=0; j<4; j++) 
	{
		t = (j==0) ? AC_TABLE_INDEX0 : (j==1) ? AC_TABLE_INDEX1 : (j==2) ? DC_TABLE_INDEX0 : DC_TABLE_INDEX1;	
		for (i=0; i<256; i++) 
		{
			if ((t==DC_TABLE_INDEX0 || t==DC_TABLE_INDEX1) && (i>15))	// DC
				break;		

			if ((pJpgInfo->huffSize[t][i] == 0) && (pJpgInfo->huffCode[t][i] == 0))
				huffData = 0;
			else 
			{
				huffData =                    (pJpgInfo->huffSize[t][i] - 1);	// Code length (1 ~ 16), 4-bit
				huffData = (huffData << 16) | (pJpgInfo->huffCode[t][i]    );	// Code word, 16-bit
			}
			VpuWriteReg(MJPEG_HUFF_DATA_REG, huffData);
		}
	}
	VpuWriteReg(MJPEG_HUFF_CTRL_REG, 0x0);
	return 1;
}

gint JpgEncLoadQMatTab(EncInstCtl *handle, JpgEncInfo *pJpgInfo)
{
	gint dividend = 0x80000;
	gint quotient;
	gint quantID;
	gint divisor;
	gint comp;
	gint i, t;

	for (comp=0; comp<3; comp++) {
		quantID = pJpgInfo->pCInfoTab[comp][3];
		if (quantID >= 4)
			return 0;
		t = (comp==0)? Q_COMPONENT0 :
		(comp==1)? Q_COMPONENT1 : Q_COMPONENT2;
		VpuWriteReg(MJPEG_QMAT_CTRL_REG, 0x3 + t);
		for (i=0; i<64; i++) {
			divisor = pJpgInfo->pQMatTab[quantID][i];
			quotient= dividend / divisor;
			VpuWriteReg(MJPEG_QMAT_DATA_REG, (int) quotient);	
		}
		VpuWriteReg(MJPEG_QMAT_CTRL_REG, t);
	}
	return 1;
}

#define PUT_BYTE(_p, _b) \
	if (tot++ > len) return 0; \
	*_p++ = (unsigned char)(_b);	

static gint JpgEncEncodeHeader(EncInstCtl *handle, EncParamSet * para)
{
 //   EncInfo *pEncInfo=&(handle->pEncInfo);
	guchar *p;
	gint i, tot, len, pad;

	tot = 0;
	p = para->pParaSet;
	len = para->size;

	// SOI Header
	PUT_BYTE(p, 0xff);
	PUT_BYTE(p, 0xD8);

	// APP9 Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xE9);
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x04);
	PUT_BYTE(p, (handle->jpgInfo.frameIdx >> 8));
	PUT_BYTE(p, (handle->jpgInfo.frameIdx & 0xFF));

	// DRI header
	if (handle->jpgInfo.rstIntval) {
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xDD);
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x04);
		PUT_BYTE(p, (handle->jpgInfo.rstIntval >> 8));
		PUT_BYTE(p, (handle->jpgInfo.rstIntval & 0xff));
	}

	// DQT Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xDB);
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x43);
	PUT_BYTE(p, 0x00);

	for (i=0; i<64; i++) {
		PUT_BYTE(p, handle->jpgInfo.pQMatTab[0][i]);
	}

	if (handle->jpgInfo.format != SAMPLE_400) {
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xDB);
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x43);
		PUT_BYTE(p, 0x01);

		for (i=0; i<64; i++) {
			PUT_BYTE(p, handle->jpgInfo.pQMatTab[1][i]);
		}
	}

	// DHT Header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC4);
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0x1F);
	PUT_BYTE(p, 0x00);

	for (i=0; i<16; i++) {
		PUT_BYTE(p, handle->jpgInfo.pHuffBits[0][i]);
	}

	for (i=0; i<12; i++) {
		PUT_BYTE(p, handle->jpgInfo.pHuffVal[0][i]);
	}

	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC4);
	PUT_BYTE(p, 0x00);
	PUT_BYTE(p, 0xB5);
	PUT_BYTE(p, 0x10);

	for (i=0; i<16; i++) {
		PUT_BYTE(p, handle->jpgInfo.pHuffBits[1][i]);
	}
	for (i=0; i<162; i++) {
		PUT_BYTE(p, handle->jpgInfo.pHuffVal[1][i]);
	}
	if (handle->jpgInfo.format != SAMPLE_400) {
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xC4);
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0x1F);
		PUT_BYTE(p, 0x01);

		for (i=0; i<16; i++) {
			PUT_BYTE(p, handle->jpgInfo.pHuffBits[2][i]);
		}	
		for (i=0; i<12; i++) {
			PUT_BYTE(p, handle->jpgInfo.pHuffVal[2][i]);
		}
		PUT_BYTE(p, 0xFF);
		PUT_BYTE(p, 0xC4);
		PUT_BYTE(p, 0x00);
		PUT_BYTE(p, 0xB5);
		PUT_BYTE(p, 0x11);
		for (i=0; i<16; i++) {
			PUT_BYTE(p, handle->jpgInfo.pHuffBits[3][i]);
		}
		for (i=0; i<162; i++) {
			PUT_BYTE(p, handle->jpgInfo.pHuffVal[3][i]);			
		}
	}

	// SOF header
	PUT_BYTE(p, 0xFF);
	PUT_BYTE(p, 0xC0);
	PUT_BYTE(p, (((8+(handle->jpgInfo.compNum*3)) >> 8) & 0xFF));
	PUT_BYTE(p, ((8+(handle->jpgInfo.compNum*3)) & 0xFF));
	PUT_BYTE(p, 0x08);
	PUT_BYTE(p, (handle->jpgInfo.picHeight >> 8));
	PUT_BYTE(p, (handle->jpgInfo.picHeight & 0xFF));
	PUT_BYTE(p, (handle->jpgInfo.picWidth >> 8));
	PUT_BYTE(p, (handle->jpgInfo.picWidth & 0xFF));
	PUT_BYTE(p, handle->jpgInfo.compNum);
	for (i=0; i<handle->jpgInfo.compNum; i++) {
		PUT_BYTE(p, (i+1));
		PUT_BYTE(p, ((handle->jpgInfo.pCInfoTab[i][1]<<4) & 0xF0) + (handle->jpgInfo.pCInfoTab[i][2] & 0x0F));
		PUT_BYTE(p, handle->jpgInfo.pCInfoTab[i][3]);		
	}
	//tot = p - para->pParaSet;
	pad = 0;
	if (tot % 8) {
		pad = tot % 8;
		pad = 8-pad;
		for (i=0; i<pad; i++) {
			PUT_BYTE(p, 0x00);
		}
	}
	handle->jpgInfo.frameIdx++;
	para->size = tot;
	return tot;
}



CodecRetCode VPU_Init(BitPrcBuffer *PrcBuf, void *vpu_handle, guchar dec_enc)
{
	//void * handle;
	PhysicalAddress codeBuffer;
//	guint top_reg_base_addr;
	guint i;
	guint data, dataH, dataL;
	guint cur_pc, val;

//#ifdef USE_SECOND_AXI
#if 0
	top_reg_base_addr = (guint)hal_paddr_to_vaddr(SOC_REG_KA_TO_PA(SOC_REG_BASE_SYS));
	HAL_PUT_UINT32((guint*)(top_reg_base_addr+0x0100), ((HAL_GET_UINT32((guint*)(top_reg_base_addr+0x0100))&0xffffff) | 0x11000000));
#endif
	if ( dec_enc ){
		/* dec handle */
		DecInstCtl *handle;
		handle=(DecInstCtl *)vpu_handle;
	

	if ( VpuReadReg(BIT_CODEC_BUSY) ){
		val = VpuReadReg( BIT_DEC_FUNC_CTRL );
		val |= 0x3C;
		VpuWriteReg(BIT_DEC_FUNC_CTRL, val);	
		g_usleep(10000);
	}
	
	if ( VpuReadReg(BIT_INT_STS) )
		ClrVPUBitInt(handle, dec_enc);
	
	if ( VpuReadReg(BIT_CUR_PC) ){
		VpuWriteReg(BIT_CODE_RUN, 0);
		VpuWriteReg(BIT_CODE_RESET, 1);
		g_usleep(10000);	
	}

	//VPURegInit(handle, 0x100, 64);
	for ( i = 0; i < 64; i++ ) {
		VpuWriteReg(0x100+i*4, 0);
	}

	codeBuffer = PrcBuf->code_buf;

	cur_pc = VpuReadReg(BIT_CUR_PC);
	
	if ( !cur_pc ){
		/* copy full microcode to code buffer reserved on SDRAM */
		for (i=0; i<sizeof(bit_code)/sizeof(bit_code[0]); i+=4) {
			dataH = (bit_code[i+0] << 16) | bit_code[i+1];
			dataL = (bit_code[i+2] << 16) | bit_code[i+3];
			*(guint *)(codeBuffer + i * 2) = dataL;	
			*(guint *)(codeBuffer + i * 2 + 4) = dataH;		
		}
		//hal_cache_flush(codeBuffer, sizeof(bit_code));
	}else {
		GST_ERROR("After Reset, Cur Pc is not zero\n");
		return RETCODE_FAILURE;
	}

	VpuWriteReg(BIT_WORK_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->work_buf)));
	VpuWriteReg(BIT_PARA_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->para_buf)));
	VpuWriteReg(BIT_CODE_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)codeBuffer));
	
	VpuWriteReg(BIT_CODE_RUN, 0);
	
	for( i = 0; i < 1024; i++ ) {
		data = bit_code[i];
		VpuWriteReg(BIT_CODE_DOWN, i << 16 | data);
	}


	data = STREAM_ENDIAN;
	data |= STREAM_32ENDIAN << 1;
	data |= STREAM_FULL_EMPTY_CHECK_DISABLE << 2;
	data |= 1 << 3;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, data);

	data = IMAGE_ENDIAN;
	data |= IMAGE_32ENDIAN << 1;
	data |= IMAGE_INTERLEAVE << 2;
	data |= 3 << 3;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, data);
	
	VpuWriteReg(BIT_INT_ENABLE, 0);

	VpuWriteReg(BIT_AXI_SRAM_USE, 0);
	
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_CODE_RUN, 1);
	#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
 		;	
	#endif
	}else{
		/* enc handle */
		EncInstCtl *handle;
		handle=(EncInstCtl*)vpu_handle;	
	if ( VpuReadReg(BIT_CODEC_BUSY) ){
		val = VpuReadReg( BIT_DEC_FUNC_CTRL );
		val |= 0x3C;
		VpuWriteReg(BIT_DEC_FUNC_CTRL, val);	
		g_usleep(10000);
	}
	
	if ( VpuReadReg(BIT_INT_STS) )
		ClrVPUBitInt(handle, dec_enc);
	
	if ( VpuReadReg(BIT_CUR_PC) ){
		VpuWriteReg(BIT_CODE_RUN, 0);
		VpuWriteReg(BIT_CODE_RESET, 1);
		g_usleep(10000);	
	}

	//VPURegInit(handle, 0x100, 64);
	for ( i = 0; i < 64; i++ ) {
		VpuWriteReg(0x100+i*4, 0);
	}

	codeBuffer = PrcBuf->code_buf;

	cur_pc = VpuReadReg(BIT_CUR_PC);
	
	if ( !cur_pc ){
		/* copy full microcode to code buffer reserved on SDRAM */
		for (i=0; i<sizeof(bit_code)/sizeof(bit_code[0]); i+=4) {
			dataH = (bit_code[i+0] << 16) | bit_code[i+1];
			dataL = (bit_code[i+2] << 16) | bit_code[i+3];
			*(guint *)(codeBuffer + i * 2) = dataL;	
			*(guint *)(codeBuffer + i * 2 + 4) = dataH;		
		}
		//hal_cache_flush(codeBuffer, sizeof(bit_code));
	}else {
		GST_ERROR("After Reset, Cur Pc is not zero\n");
		return RETCODE_FAILURE;
	}

	VpuWriteReg(BIT_WORK_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->work_buf)));
	VpuWriteReg(BIT_PARA_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)(PrcBuf->para_buf)));
	VpuWriteReg(BIT_CODE_BUF_ADDR, (guint)hal_vaddr_to_paddr((gpointer)codeBuffer));
	
	VpuWriteReg(BIT_CODE_RUN, 0);
	
	for( i = 0; i < 1024; i++ ) {
		data = bit_code[i];
		VpuWriteReg(BIT_CODE_DOWN, i << 16 | data);
	}


	data = STREAM_ENDIAN;
	data |= STREAM_32ENDIAN << 1;
	data |= STREAM_FULL_EMPTY_CHECK_DISABLE << 2;
	data |= 1 << 3;
	VpuWriteReg(BIT_BIT_STREAM_CTRL, data);

	data = IMAGE_ENDIAN;
	data |= IMAGE_32ENDIAN << 1;
	data |= IMAGE_INTERLEAVE << 2;
	data |= 3 << 3;
	VpuWriteReg(BIT_FRAME_MEM_CTRL, data);
	
	VpuWriteReg(BIT_INT_ENABLE, 0);

	VpuWriteReg(BIT_AXI_SRAM_USE, 0);
	
	VpuWriteReg(BIT_BUSY_FLAG, 1);
	VpuWriteReg(BIT_CODE_RUN, 1);
	#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
 		;	
	#endif
		
	}
	return RETCODE_SUCCESS;
}


CodecRetCode VPU_GetVersionInfo( DecInstCtl *handle, guint *versionInfo )
{
	guint ver;
	
	if (VpuReadReg(BIT_CUR_PC) == 0)
		return RETCODE_NOT_INITIALIZED;


	VpuWriteReg( RET_VER_NUM , 0 );

	VpuWriteReg( BIT_BUSY_FLAG, 0x1 );
	BitIssueCommand(handle, 0, 0, FIRMWARE_GET );
 	while (VpuReadReg(BIT_BUSY_FLAG))
 			;
		
	ver = VpuReadReg( RET_VER_NUM );
	
	if( ver == 0 )
		return RETCODE_FAILURE;

	*versionInfo = ver;

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_DecOpen(guchar inst_index, DecInstCtl *handle)
{
	CodecRetCode ret;
	DecSeqOpenParam *SeqOpenParam=&(handle->SeqOpenParam);
	DecSeqGetInfo *SeqInfo=&(handle->SeqInfo);

	if (VpuReadReg(BIT_CUR_PC) == 0){
		return RETCODE_NOT_INITIALIZED;
	}

	ret = CheckDecOpenParam(&handle->SeqOpenParam);
	if (ret != RETCODE_SUCCESS) {
		return ret;
	}

	if (handle->BitstreamFormat != STD_MJPG) {
		SeqOpenParam->streamRdPtrRegAddr = BIT_RD_PTR_0;
		SeqOpenParam->streamWrPtrRegAddr = BIT_WR_PTR_0;
		SeqOpenParam->frameDisplayFlagRegAddr = BIT_FRAME_DIS_FLAG_0;
	}
	else {
		SeqOpenParam->streamRdPtrRegAddr = MJPEG_BBC_RD_PTR_REG;
		SeqOpenParam->streamWrPtrRegAddr = MJPEG_BBC_WR_PTR_REG;
		SeqOpenParam->frameDisplayFlagRegAddr = 0;
	}

	handle->SeqInfo.initialInfoObtained = 0;
	handle->SeqOpenParam.vc1BframeDisplayValid = 0;

	SeqInfo->JpgDecInfo.frameIdx = 0;

	VpuWriteReg(BIT_RD_PTR_0+inst_index*8 , SeqOpenParam->streamBufStartAddr);
	VpuWriteReg(BIT_WR_PTR_0+inst_index*8 , SeqOpenParam->streamBufStartAddr);
	//LOG_PRINTF("3 Writep %x\n", VpuReadReg(0x124));
	
    if (handle->BitstreamFormat != STD_MJPG) {
		VpuWriteReg(SeqOpenParam->frameDisplayFlagRegAddr, 0);
		VpuWriteReg(BIT_INT_ENABLE, VPU_ISR_ALL);
	} else {
		SeqInfo->JpgDecInfo.frameIdx = 0;
		SeqInfo->JpgDecInfo.seqInited = 0;
		VpuWriteReg(MJPEG_BBC_BAS_ADDR_REG, SeqOpenParam->streamBufStartAddr);
		VpuWriteReg(MJPEG_BBC_END_ADDR_REG, SeqOpenParam->streamBufEndAddr);

		VpuWriteReg(MJPEG_BBC_STRM_CTRL_REG, 0);
		return RETCODE_SUCCESS;
	}

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecClose(DecInstCtl *handle, guchar inst_index)
{
	if (handle->BitstreamFormat == STD_MJPG) {
		return RETCODE_SUCCESS;	
	}

	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, SEQ_END);
#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
 			;	
#endif

	return RETCODE_SUCCESS;
}

CodecRetCode CheckEncOpenParam(EncInstCtl * handle, EncOpenParam * pop)
{
    gint picWidth;
    gint picHeight;

    if (pop == 0) {
        return RETCODE_INVALID_PARAM;
    }
    picWidth = pop->picWidth;
    picHeight = pop->picHeight;

    if (pop->bitstreamBuffer % 4) { // not 4-bit aligned
        return RETCODE_INVALID_PARAM;
    }
    if (pop->bitstreamBufferSize % 1024 ||
            pop->bitstreamBufferSize < 1024 ||
            pop->bitstreamBufferSize > 16383 * 1024) {
        return RETCODE_INVALID_PARAM;
    }
    if (handle->BitstreamFormat != STD_AVC &&
            handle->BitstreamFormat != STD_MP4 &&
            handle->BitstreamFormat != STD_H263 &&
            handle->BitstreamFormat != STD_MJPG ) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->bitRate > 32767 || pop->bitRate < 0) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->bitRate !=0 && pop->initialDelay > 32767) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->bitRate !=0 && pop->initialDelay != 0 && pop->vbvBufferSize < 0) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->enableAutoSkip != 0 && pop->enableAutoSkip != 1) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->gopSize > 60) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->slicemode.sliceMode != 0 && pop->slicemode.sliceMode != 1) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->slicemode.sliceMode == 1) {
        if (pop->slicemode.sliceSizeMode != 0 && pop->slicemode.sliceSizeMode != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (pop->slicemode.sliceSizeMode == 1 && pop->slicemode.sliceSize == 0 ) {
            return RETCODE_INVALID_PARAM;
        }
    }
    if (pop->intraRefresh < 0 || pop->intraRefresh >= (picWidth * picHeight /256)) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->MESearchRange < 0 || pop->MESearchRange >= 4) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->MEUseZeroPmv != 0 && pop->MEUseZeroPmv != 1) {
        return RETCODE_INVALID_PARAM;
    }
    if (pop->IntraCostWeight < 0 || pop->IntraCostWeight >= 65535) {
        return RETCODE_INVALID_PARAM;
    }

    if (handle->BitstreamFormat == STD_MP4) {
        EncMp4Param * param = &pop->EncStdParam.mp4Param;
        if (param->mp4_dataPartitionEnable != 0 && param->mp4_dataPartitionEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->mp4_dataPartitionEnable == 1) {
            if (param->mp4_reversibleVlcEnable != 0 && param->mp4_reversibleVlcEnable != 1) {
                return RETCODE_INVALID_PARAM;
            }
        }
        if (param->mp4_intraDcVlcThr < 0 || 7 < param->mp4_intraDcVlcThr) {
            return RETCODE_INVALID_PARAM;
        }
        if (picWidth < 96 || picWidth > MAX_ENC_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }
        if (picHeight < 16 || picHeight > MAX_ENC_PIC_HEIGHT ) {
            return RETCODE_INVALID_PARAM;
        }
    }
    else if (handle->BitstreamFormat == STD_H263) {
        EncH263Param * param = &pop->EncStdParam.h263Param;
        guint frameRateInc, frameRateRes;
        if (param->h263_annexJEnable != 0 && param->h263_annexJEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->h263_annexKEnable != 0 && param->h263_annexKEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->h263_annexTEnable != 0 && param->h263_annexTEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->h263_annexJEnable == 0 &&
                param->h263_annexKEnable == 0 &&
                param->h263_annexTEnable == 0
           ) {
            if (!(picWidth == 128 && picHeight == 96) &&
                    !(picWidth == 176 && picHeight == 144) &&
                    !(picWidth == 352 && picHeight == 288) &&
                    !(picWidth == 704 && picHeight == 576)) {
            //  return RETCODE_INVALID_PARAM;
            }
        }
        if (picWidth < 64 || picWidth > MAX_ENC_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }
        if (picHeight < 16 || picHeight > MAX_ENC_PIC_HEIGHT ) {
            return RETCODE_INVALID_PARAM;
        }
        frameRateInc = ((pop->frameRateInfo>>16) &0xFFFF) + 1;
        frameRateRes = pop->frameRateInfo & 0xFFFF;
        if( (frameRateRes/frameRateInc) <15 )
            return RETCODE_INVALID_PARAM;

    }
    else if (handle->BitstreamFormat == STD_AVC) {
        EncAvcParam * param = &pop->EncStdParam.avcParam;
        if (param->avc_constrainedIntraPredFlag != 0 && param->avc_constrainedIntraPredFlag != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_disableDeblk != 0 && param->avc_disableDeblk != 1 && param->avc_disableDeblk != 2) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_deblkFilterOffsetAlpha < -6 || 6 < param->avc_deblkFilterOffsetAlpha) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_deblkFilterOffsetBeta < -6 || 6 < param->avc_deblkFilterOffsetBeta) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_chromaQpOffset < -12 || 12 < param->avc_chromaQpOffset) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_audEnable != 0 && param->avc_audEnable != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_frameCroppingFlag != 0 &&param->avc_frameCroppingFlag != 1) {
            return RETCODE_INVALID_PARAM;
        }
        if (param->avc_frameCropLeft & 0x01 ||
            param->avc_frameCropRight & 0x01 ||
            param->avc_frameCropTop & 0x01 ||
            param->avc_frameCropBottom & 0x01) {
            return RETCODE_INVALID_PARAM;
        }      

        if (picWidth < 64 || picWidth > MAX_ENC_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }
        if (picHeight < 16 || picHeight > MAX_ENC_PIC_HEIGHT ) {
            return RETCODE_INVALID_PARAM;
        }      

    }
    else if (handle->BitstreamFormat == STD_MJPG) {
        if (picWidth < 16 || picWidth > MAX_ENC_MJPG_PIC_WIDTH ) {
            return RETCODE_INVALID_PARAM;
        }
        if (picHeight < 16 || picHeight > MAX_ENC_MJPG_PIC_HEIGHT ) {
            return RETCODE_INVALID_PARAM;
        }       
    }

    return RETCODE_SUCCESS;
}

CodecRetCode VPU_EncOpen(guchar inst_index, EncInstCtl * handle)
{
	EncOpenParam * pop=&(handle->EncSeqOpenParam);
	EncInfo *pEncInfo = &(handle->pEncInfo);
	JpgEncInfo *pJpgInfo = &(handle->jpgInfo);
    CodecRetCode ret;
    guint  val;

    if (handle->BitstreamFormat != STD_MJPG) {
		if (VpuReadReg(BIT_CUR_PC) == 0)
			return RETCODE_NOT_INITIALIZED;
	}

    ret = CheckEncOpenParam(handle, pop);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pop->codecModeAux = 0;
    if( handle->BitstreamFormat == STD_MP4 || handle->BitstreamFormat == STD_H263 )
        pop->codecMode = MP4_ENC;
    else if( handle->BitstreamFormat == STD_AVC )
        pop->codecMode = AVC_ENC;
    else if( handle->BitstreamFormat == STD_MJPG )
        pop->codecMode = MJPG_ENC;

	if (pop->codecMode != MJPG_ENC) {
		pEncInfo->streamRdPtrRegAddr = BIT_RD_PTR_0;
		pEncInfo->streamWrPtrRegAddr = BIT_WR_PTR_0;
	} else {
		pEncInfo->streamRdPtrRegAddr = MJPEG_BBC_RD_PTR_REG;
		pEncInfo->streamWrPtrRegAddr = MJPEG_BBC_WR_PTR_REG;
	}
    pEncInfo->streamBufStartAddr = pop->bitstreamBuffer;
    pEncInfo->streamBufSize = pop->bitstreamBufferSize;
    pEncInfo->streamBufEndAddr = pop->bitstreamBuffer + pop->bitstreamBufferSize;
    //pEncInfo->frameBufPool = 0;

    pEncInfo->secAxiUse.useBitEnable = 0;
    pEncInfo->secAxiUse.useIpEnable = 0;
    pEncInfo->secAxiUse.useDbkYEnable = 0;
    pEncInfo->secAxiUse.useDbkCEnable = 0;
    pEncInfo->secAxiUse.useOvlEnable = 0;
	pEncInfo->secAxiUse.useBtpEnable = 0;
    pEncInfo->secAxiUse.useHostBitEnable = 0;
    pEncInfo->secAxiUse.useHostIpEnable = 0;
    pEncInfo->secAxiUse.useHostDbkYEnable = 0;
    pEncInfo->secAxiUse.useHostDbkCEnable = 0;
    pEncInfo->secAxiUse.useHostOvlEnable = 0;
	pEncInfo->secAxiUse.useHostBtpEnable = 0;

    pEncInfo->rotationEnable = 0;
    pEncInfo->mirrorEnable = 0;
    pEncInfo->mirrorDirection = MIRDIR_NONE;
    pEncInfo->rotationAngle = 0;

    // MB Aligned source resolution
    pEncInfo->srcFrameWidth = (pop->picWidth + 15) & ~15;
    pEncInfo->srcFrameHeight = (pop->picHeight + 15) & ~15;
    pEncInfo->initialInfoObtained = 0;
    pEncInfo->dynamicAllocEnable = pop->dynamicAllocEnable;
    pEncInfo->ringBufferEnable = pop->ringBufferEnable;
    pEncInfo->cacheConfig.Bypass = 1;                       // By default, turn off MC cache
    pEncInfo->subFrameSyncConfig.subFrameSyncOn = 0;        // By default, turn off SubFrameSync

    VpuWriteReg(pEncInfo->streamRdPtrRegAddr, pEncInfo->streamBufStartAddr);
    VpuWriteReg(pEncInfo->streamWrPtrRegAddr, pEncInfo->streamBufStartAddr);

	if (pop->codecMode == MJPG_ENC) {
		pJpgInfo->frameIdx = 0;
		pJpgInfo->seqInited = 0;
		pJpgInfo->format = pop->EncStdParam.mjpgParam.mjpg_sourceFormat;
		pJpgInfo->picWidth= pop->picWidth;
		pJpgInfo->picHeight = pop->picHeight;
		// Picture size alignment
		if (pJpgInfo->format == YCBCR420 || pJpgInfo->format == YCBCR422H)
			pJpgInfo->alignedWidth = ((pJpgInfo->picWidth+15)/16)*16;
		else
			pJpgInfo->alignedWidth = ((pJpgInfo->picWidth+7)/8)*8;

		if (pJpgInfo->format == YCBCR420 || pJpgInfo->format == YCBCR422V)
			pJpgInfo->alignedHeight = ((pJpgInfo->picHeight+15)/16)*16;
		else
			pJpgInfo->alignedHeight = ((pJpgInfo->picHeight+7)/8)*8;
		pJpgInfo->rstIntval = pop->EncStdParam.mjpgParam.njpg_restartInterval;

		for (val=0; val<4; val++){
			//pJpgInfo->pHuffVal[val] = pop->EncStdParam.mjpgParam.huffVal[val];
			memcpy(&pJpgInfo->pHuffVal[val], &pop->EncStdParam.mjpgParam.huffVal[val], 162);
		}
		for (val=0; val<4; val++){
			//pJpgInfo->pHuffBits[val] = pop->EncStdParam.mjpgParam.huffBits[val];
			memcpy(&pJpgInfo->pHuffBits[val], &pop->EncStdParam.mjpgParam.huffBits[val], 256);
		}
		for (val=0; val<4; val++){
			//pJpgInfo->pQMatTab[val] = pop->EncStdParam.mjpgParam.qMatTab[val];
			memcpy(&pJpgInfo->pQMatTab[val], &pop->EncStdParam.mjpgParam.qMatTab[val], 64);
		}
		for (val=0; val<4; val++){
			//pJpgInfo->pCInfoTab[val] = pop->EncStdParam.mjpgParam.cInfoTab[val];
			memcpy(&pJpgInfo->pCInfoTab[val], &pop->EncStdParam.mjpgParam.cInfoTab[val], 6);
		}

		return RETCODE_SUCCESS;
	} 

    val = VpuReadReg( BIT_BIT_STREAM_CTRL );
    val &= 0xFFC7;
    if( pEncInfo->ringBufferEnable == 0 ) { 
        val |= ( pEncInfo->dynamicAllocEnable << 5 );       
        val |= 1 << 4;  
    } else {
        val |= 1 << 3;
    }   

    VpuWriteReg(BIT_BIT_STREAM_CTRL, val);

    return RETCODE_SUCCESS;
}

CodecRetCode VPU_EncClose(EncInstCtl * handle, guchar inst_index)
{
	if (handle->BitstreamFormat == STD_MJPG) {
		VpuWriteReg(MJPEG_BBC_FLUSH_CMD_REG, 0);
		return RETCODE_SUCCESS;	
	}
    EncBitIssueCommand(handle, inst_index, handle->EncSeqOpenParam.codecMode, SEQ_END);
	while(VPU_IsBusy(handle, 0)) 
			;
    

    return RETCODE_SUCCESS;
}



CodecRetCode VPU_DecSetEscSeqInit( DecInstCtl *handle, gint escape )
{
	if (handle->BitstreamFormat == STD_MJPG) {
		return RETCODE_SUCCESS;
	}

	VpuWriteReg(BIT_DEC_FUNC_CTRL,  escape & 0x01  );	

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecInit(DecInstCtl *handle, guchar inst_index)
{
	DecSeqOpenParam *SeqOpenParam = &handle->SeqOpenParam;
	DecSeqGetInfo *SeqGetInfo = &handle->SeqInfo;
	JpgDecInfo * pJpgDecInfo = &SeqGetInfo->JpgDecInfo;
	guint picSize;
	guint val;
#if 0
	if ( SeqGetInfo->initialInfoObtained) {
		printf("has got the seq info before\n");
		return RETCODE_CALLED_BEFORE;
	}
	#endif

	if ( handle->BitstreamFormat == STD_AVC ) {
		SeqOpenParam->codecMode = AVC_DEC;
	} else if ( handle->BitstreamFormat == STD_VC1) {
		SeqOpenParam->codecMode = VC1_DEC;
	} else if ( handle->BitstreamFormat == STD_MP2 ) {
		SeqOpenParam->codecMode = MP2_DEC;
	} else if ( handle->BitstreamFormat == STD_MP4 ) {
		SeqOpenParam->codecMode = MP4_DEC;
	} else if ( handle->BitstreamFormat == STD_H263 ) {
		SeqOpenParam->codecMode = H263_DEC;
	} else if ( handle->BitstreamFormat == STD_DIV3 ) {
		SeqOpenParam->codecMode = DIV3_DEC;
	} else if ( handle->BitstreamFormat == STD_RV) {
		SeqOpenParam->codecMode = RV_DEC;
	} else if ( handle->BitstreamFormat == STD_AVS ) {
		SeqOpenParam->codecMode = AVS_DEC;
	} else if ( handle->BitstreamFormat == STD_MJPG ) {
		SeqOpenParam->codecMode = MJPG_DEC;
	}

    if (SeqOpenParam->codecMode == MJPG_DEC) {
		if (!JpegDecodeHeader(pJpgDecInfo))
			return RETCODE_FAILURE;

		VpuWriteReg(MJPEG_GBU_TT_CNT_REG, 0);
		VpuWriteReg(MJPEG_PIC_CTRL_REG, pJpgDecInfo->huffAcIdx << 10 |pJpgDecInfo->huffDcIdx << 7 | pJpgDecInfo->userHuffTab << 6 | 0 << 2 | 0);
		VpuWriteReg(MJPEG_PIC_SIZE_REG, (pJpgDecInfo->alignedWidth << 16) | pJpgDecInfo->alignedHeight);

		VpuWriteReg(MJPEG_ROT_INFO_REG, 0);
		VpuWriteReg(MJPEG_OP_INFO_REG, pJpgDecInfo->busReqNum);
		VpuWriteReg(MJPEG_MCU_INFO_REG, pJpgDecInfo->mcuBlockNum << 16 | pJpgDecInfo->compNum << 12 | pJpgDecInfo->compInfo[0] << 8 | pJpgDecInfo->compInfo[1] << 4 | pJpgDecInfo->compInfo[2]);
		VpuWriteReg(MJPEG_SCL_INFO_REG, 0);
		VpuWriteReg(MJPEG_DPB_CONFIG_REG, IMAGE_ENDIAN << 1 | IMAGE_INTERLEAVE);
		VpuWriteReg(MJPEG_RST_INTVAL_REG, pJpgDecInfo->rstIntval);

		if (pJpgDecInfo->userHuffTab) {
			if (!JpgDecHuffTabSetUp(handle, pJpgDecInfo)) {
				return RETCODE_INVALID_PARAM;
			}
		}

		if (!JpgDecQMatTabSetUp(handle,pJpgDecInfo)) {
			return RETCODE_INVALID_PARAM;
		}

		SeqGetInfo->picWidth = pJpgDecInfo->picWidth;
		SeqGetInfo->picHeight = pJpgDecInfo->picHeight;
		SeqGetInfo->minFrameBufferCount = 1;
		SeqGetInfo->mjpg_sourceFormat = pJpgDecInfo->format;
		SeqGetInfo->mjpg_ecsPtr = pJpgDecInfo->ecsPtr;
		SeqGetInfo->initialInfoObtained = 1;

		return RETCODE_SUCCESS;
    } 

	VpuWriteReg(BIT_RD_PTR_0+inst_index*8 , SeqOpenParam->streamBufStartAddr);
	VpuWriteReg(BIT_EXACT_RD_PTR , SeqOpenParam->streamBufStartAddr);		

	if (DecBitstreamBufEmpty(handle, inst_index)) {
		//printf("ves buf is empty\n");
		//printf("readp is %x\n", (OMX_U32)(VpuReadReg(BIT_RD_PTR_0 + 8*inst_index)));
		return RETCODE_WRONG_CALL_SEQUENCE;
	}


	VPURegInit(handle, 0x180, 32, 1);
	VpuWriteReg(BIT_FRAME_DIS_FLAG_0+4*inst_index, 0);

	VpuWriteReg(CMD_DEC_SEQ_BB_START, SeqOpenParam->streamBufStartAddr);
	VpuWriteReg(CMD_DEC_SEQ_BB_SIZE, SeqOpenParam->streamBufSize/1024); // size in KBytes
	
	if( handle->SeqOpenParam.filePlayEnable == 1 )
		VpuWriteReg(CMD_DEC_SEQ_START_BYTE, SeqOpenParam->streamStartByteOffset);

	val = ((SeqOpenParam->dynamicAllocEnable << 3) & 0x8) | ((SeqOpenParam->filePlayEnable << 2) & 0x4) | ((SeqOpenParam->reorderEnable << 1) & 0x2) | (SeqOpenParam->mp4DeblkEnable & 0x1);	
	VpuWriteReg(CMD_DEC_SEQ_OPTION, val);					

	if( SeqOpenParam->codecMode == AVC_DEC ) {
		VpuWriteReg( CMD_DEC_SEQ_PS_BB_START, SeqOpenParam->psSaveBuffer );
		VpuWriteReg( CMD_DEC_SEQ_PS_BB_SIZE, SeqOpenParam->psSaveBufferSize / 1024 );
	}

	if ( handle->BitstreamFormat == STD_DIV3 ) {
		VpuWriteReg(BIT_RUN_AUX_STD, 1);
	} else  {
		VpuWriteReg(BIT_RUN_AUX_STD, 0);
	}

	/* assume the pic source size is 720*576, the register can be omitted*/
	picSize = 720 << 16 | 576;
	VpuWriteReg( CMD_DEC_SEQ_SRC_SIZE, picSize );
	
	BitIssueCommand(handle, inst_index, SeqOpenParam->codecMode, SEQ_INIT);

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecRegisterFrameBuffer(DecInstCtl *handle, BitPrcBuffer *VPUbuffer, guchar inst_index)
{
	DecSeqGetInfo* GetInfo = &handle->SeqInfo;
	FrameBuffer * bufArray;
	gint i;
	//CodecRetCode ret;
	guint address[150];
	guchar tmp;	
#ifdef USE_SECOND_AXI	
	guint vc1_dbky_size;
#endif

	if ( handle->BitstreamFormat == STD_MJPG)
		return RETCODE_SUCCESS;

	bufArray = GetInfo->frameBufPool;

	if (!GetInfo->initialInfoObtained) {
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if (bufArray == 0) {
		return RETCODE_INVALID_FRAME_BUFFER;
	}

	if (GetInfo->stride < GetInfo->picWidth || GetInfo->stride % 8 != 0 ) {
		//printf("current stride%x\n", (unsigned int)GetInfo->stride);
		return RETCODE_INVALID_STRIDE;
	}
	
	VPURegInit(handle, 0x180, 32, 1);
	for ( i = 0; i < 150; i++ )
		address[i] = 0;

	// Let the decoder know the addresses of the frame buffers.
	for (i = 0; i < GetInfo->numFrameBuffers; i++) {
		address[i*3] =  bufArray[i].bufY;
		address[i*3+1] =  bufArray[i].bufCb;
		address[i*3+2] =  bufArray[i].bufCr;
		if( handle->SeqOpenParam.codecMode == AVC_DEC )
			address[i+96] =  bufArray[i].bufMvCol;
	}

	// Let the decoder know the addresses of the frame buffers.
	tmp = (GetInfo->numFrameBuffers * 3 + 1 ) >> 1;
	for (i = 0; i < tmp; i++) {
		*(guint *)(VPUbuffer->para_buf + i * 2 * 4) =  address[i*2+1];
		*(guint *)(VPUbuffer->para_buf + i * 2 * 4 + 4) =  address[i*2];
	}
	tmp = ( GetInfo->numFrameBuffers + 1 ) >> 1;
	for (i = 0; i < tmp; i++) {
		if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
			*(guint *)(VPUbuffer->para_buf + ( i + 48 ) * 8) =  address[97+i*2];
			*(guint *)(VPUbuffer->para_buf + ( i + 48 ) * 8 + 4) =  address[96+i*2];
		}
	}
	
	if( handle->SeqOpenParam.codecMode != AVC_DEC && handle->SeqOpenParam.codecMode != MP2_DEC && handle->SeqOpenParam.codecMode != MJPG_DEC ) {
		*(guint *)(VPUbuffer->para_buf + 388) = bufArray[0].bufMvCol;
	}

	//hal_cache_flush(VPUbuffer->para_buf, 2048);


	// Tell the decoder how much frame buffers were allocated.
	VpuWriteReg(CMD_SET_FRAME_BUF_NUM, GetInfo->numFrameBuffers);
	VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, GetInfo->stride);

#ifdef USE_SECOND_AXI
	VPUbuffer->dbky_buf = 0;
	VPUbuffer->dbkc_buf = 0;			
	VPUbuffer->bit_buf = 0;
	VPUbuffer->ipacdc_buf = 0;
	VPUbuffer->ovl_buf = 0;

	//MEMSET(&(GetInfo->second_axi_use), 0, sizeof(GetInfo->second_axi_use));

	switch(handle->BitstreamFormat){
		case STD_AVC:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_VC1:
			if ( GetInfo->profile == 0 || GetInfo->profile == 1 ){
				/*Simple and main profile*/
				GetInfo->second_axi_use.bitpro_use = 1;
				GetInfo->second_axi_use.dbk_use = 1;
				if ( GetInfo->picWidth > 1280){
					GetInfo->second_axi_use.ipacdc_use = 0;
					vc1_dbky_size = DBK_INTERNAL_BUF_SIZE_MP_VC1_HD;
					GetInfo->second_axi_use.ovl_use = 0;
				}else{
					GetInfo->second_axi_use.ipacdc_use = 1;
					GetInfo->second_axi_use.ovl_use = 1;
					vc1_dbky_size = DBKY_INTERNAL_BUF_SIZE_MP_VC1;
				}
				VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
				VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + vc1_dbky_size;			
				VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + vc1_dbky_size;
				VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE_VC1;	
				VPUbuffer->ovl_buf = VPUbuffer->ipacdc_buf + IPACDC_INTERNAL_BUF_SIZE_VC1;
			}else{
			    /* GetInfo->profile == 2   Advanced profile */
				GetInfo->second_axi_use.dbk_use = 1;
				GetInfo->second_axi_use.ipacdc_use = 0;
				GetInfo->second_axi_use.ovl_use = 0;	
				if ( GetInfo->picWidth > 1280){
					GetInfo->second_axi_use.bitpro_use = 0;
					vc1_dbky_size = DBK_INTERNAL_BUF_SIZE_AP_VC1_HD;
				}else{
					GetInfo->second_axi_use.bitpro_use = 1;		
					vc1_dbky_size = DBKY_INTERNAL_BUF_SIZE_AP_VC1;
				}
				VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
				VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + vc1_dbky_size;			
				VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + vc1_dbky_size;
				VPUbuffer->ipacdc_buf = 0;	
				VPUbuffer->ovl_buf = 0;
			}
			break;
		case STD_MP2:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 0;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			break;
		case STD_MP4:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;
			//VPUbuffer->ipacdc_buf = SECOND_AXI_BASE_ADDR;
			break;
		case STD_H263:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_DIV3:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_RV:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			#if 1
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;		
			#else
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			#endif
			break;
		case STD_AVS:
			GetInfo->second_axi_use.bitpro_use = 1;
			GetInfo->second_axi_use.dbk_use = 1;
			GetInfo->second_axi_use.ipacdc_use = 1;
			GetInfo->second_axi_use.ovl_use = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_MJPG:
			GetInfo->second_axi_use.bitpro_use = 0;
			GetInfo->second_axi_use.dbk_use = 0;
			GetInfo->second_axi_use.ipacdc_use = 0;
			GetInfo->second_axi_use.ovl_use = 0;
			break;
		default:
			break;
			
	}

	VpuWriteReg(BIT_AXI_SRAM_USE, GetInfo->second_axi_use.ovl_use << 10 | GetInfo->second_axi_use.dbk_use << 9 | 
		GetInfo->second_axi_use.ipacdc_use << 8 | GetInfo->second_axi_use.bitpro_use << 7 | GetInfo->second_axi_use.ovl_use << 3 | 
		GetInfo->second_axi_use.dbk_use << 2 | GetInfo->second_axi_use.ipacdc_use << 1 | GetInfo->second_axi_use.bitpro_use );


	VpuWriteReg( CMD_SET_FRAME_AXI_BIT_ADDR, VPUbuffer->bit_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_ACDC_ADDR, VPUbuffer->ipacdc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKY_ADDR, VPUbuffer->dbky_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKC_ADDR, VPUbuffer->dbkc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_OVL_ADDR, VPUbuffer->ovl_buf & 0x1FFFFFFF);	

#endif

	if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_START, VPUbuffer->slice_buf & 0x1fffffff );
		VpuWriteReg( CMD_SET_FRAME_SLICE_BB_SIZE, SLICE_BUF_SIZE/1024 );
	}

	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, SET_FRAME_BUF);

	return RETCODE_SUCCESS;
}


/* maybe it is useless */

#if 0
CodecRetCode VPU_DecGetBitstreamBuffer( DecOper *operation,
		PhysicalAddress * prdPrt,
		PhysicalAddress * pwrPtr,
		UINT32 * size)
{
	CodecInst * pCodecInst;
	DecInfo * pDecInfo;
	PhysicalAddress rdPtr;
	PhysicalAddress wrPtr;
	UINT32 room;
	CodecRetCode ret;

	pCodecInst = &operation->VPU_handle;
	ret = CheckDecInstanceValidity(pCodecInst);
	if (ret != RETCODE_SUCCESS)
		return ret;


	if ( prdPrt == 0 || pwrPtr == 0 || size == 0) {
		return RETCODE_INVALID_PARAM;
	}

	pDecInfo = &pCodecInst->decInfo;


	rdPtr = VpuReadReg(pDecInfo->streamRdPtrRegAddr);
	wrPtr = pDecInfo->streamWrPtr;
	
	if (wrPtr < rdPtr) {
		room = rdPtr - wrPtr - 1;
	}
	else {
		room = ( pDecInfo->streamBufEndAddr - wrPtr ) + ( rdPtr - pDecInfo->streamBufStartAddr ) - 1;	
	}

	*prdPrt = rdPtr;
	*pwrPtr = wrPtr;
	*size = room;

	return RETCODE_SUCCESS;
}
#endif

CodecRetCode VPU_DecUpdateBitstreamBuffer(DecInstCtl *handle, guchar inst_index, PhysicalAddress wrPtr,guint size)
{
	guint val;

	if (handle->BitstreamFormat != STD_MJPG){
		val = VpuReadReg( (BIT_WR_PTR_0 + 8*inst_index));
		if ( val != (wrPtr & 0xFFFFFE00) ){
			VpuWriteReg( (BIT_WR_PTR_0 + 8*inst_index), wrPtr & 0xFFFFFE00);
		}
	}

	if (size == 0) 
	{
        if (handle->BitstreamFormat == STD_MJPG) {
			val = (wrPtr-handle->SeqOpenParam.streamBufStartAddr) / 256;
			if ((wrPtr-handle->SeqOpenParam.streamBufStartAddr) % 256)
				val = val + 1;
			VpuWriteReg(MJPEG_BBC_STRM_CTRL_REG, (1 << 31 | val));
			return RETCODE_SUCCESS;
		}
	
		/* the stream has finished */
		VpuWriteReg( (BIT_WR_PTR_0 + 8*inst_index), wrPtr);

		val = VpuReadReg( BIT_DEC_FUNC_CTRL );
		val |= 1 << ( inst_index + 2);
		VpuWriteReg(BIT_DEC_FUNC_CTRL, val);
	}

	if (handle->BitstreamFormat == STD_MJPG) {
		wrPtr = handle->SeqOpenParam.streamBufStartAddr;
		wrPtr += size;

		if (wrPtr == handle->SeqOpenParam.streamBufEndAddr) {
            wrPtr = handle->SeqOpenParam.streamBufStartAddr;
        }

		VpuWriteReg(MJPEG_BBC_CUR_POS_REG, 0);
		//pDecInfo->streamWrPtr = wrPtr;
		VpuWriteReg(handle->SeqOpenParam.streamWrPtrRegAddr, wrPtr);

		return RETCODE_SUCCESS;
	}

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecStartOneFrame(DecInstCtl *handle, guchar inst_index)
{
	DecSeqOpenParam *SeqOpenParam = &handle->SeqOpenParam;
	DecFrameCfg *param= &handle->FrameCfgParam;
	DecSeqGetInfo* GetInfo = &handle->SeqInfo;
	FrameBuffer * bufArray;
	JpgDecInfo *pJpgDecInfo=&(GetInfo->JpgDecInfo);
	guint val;//, rot_mir_flag;
	guint rotMir=0;
	//CodecRetCode ret;

	if (SeqOpenParam->codecMode == MJPG_DEC) {
		if(SeqOpenParam->filePlayEnable == 1) {
			VpuWriteReg(MJPEG_BBC_CUR_POS_REG, 0);
			if (SeqOpenParam->dynamicAllocEnable == 1) {
				VpuWriteReg(MJPEG_BBC_BAS_ADDR_REG, param->picStreamBufferAddr);
				VpuWriteReg(MJPEG_BBC_END_ADDR_REG, param->picStreamBufferAddr + param->chunkSize);
				//pDecInfo->streamWrPtr = param->picStreamBufferAddr + param->chunkSize;
				//VpuWriteReg(pDecInfo->streamWrPtrRegAddr, pDecInfo->streamWrPtr);
			}
			else {
				//pDecInfo->streamWrPtr = pDecInfo->streamBufStartAddr + param->chunkSize;
				//VpuWriteReg(pDecInfo->streamWrPtrRegAddr, pDecInfo->streamWrPtr);
			}
		}

		JpgDecGramSetup(handle, pJpgDecInfo, SeqOpenParam->streamBufStartAddr);

		VpuWriteReg(MJPEG_RST_INDEX_REG, 0);	// RST index at the beginning.
		VpuWriteReg(MJPEG_RST_COUNT_REG, 0);

		VpuWriteReg(MJPEG_DPCM_DIFF_Y_REG, 0);
		VpuWriteReg(MJPEG_DPCM_DIFF_CB_REG, 0);
		VpuWriteReg(MJPEG_DPCM_DIFF_CR_REG, 0);

		VpuWriteReg(MJPEG_GBU_FF_RPTR_REG, 0);
		VpuWriteReg(MJPEG_GBU_CTRL_REG, 3);		

		VpuWriteReg(MJPEG_ROT_INFO_REG, rotMir);

		if (rotMir & 1) {
            pJpgDecInfo->format = (pJpgDecInfo->format==YCBCR422H) ? YCBCR422V : (pJpgDecInfo->format==YCBCR422V) ? YCBCR422H : pJpgDecInfo->format;
		}

		val = (pJpgDecInfo->format == YCBCR420 || pJpgDecInfo->format == YCBCR422H || pJpgDecInfo->format == YCBCR400) ? 2 : 1;
		if (rotMir & 0x10) {
			VpuWriteReg(MJPEG_DPB_YSTRIDE_REG, SeqOpenParam->rotatorStride);
			VpuWriteReg(MJPEG_DPB_CSTRIDE_REG, SeqOpenParam->rotatorStride/(int)val);

			VpuWriteReg(MJPEG_DPB_BASE00_REG, SeqOpenParam->rotatorOutput.bufY);
			VpuWriteReg(MJPEG_DPB_BASE01_REG, SeqOpenParam->rotatorOutput.bufCb);
			VpuWriteReg(MJPEG_DPB_BASE02_REG, SeqOpenParam->rotatorOutput.bufCr);
		} else {
			VpuWriteReg(MJPEG_DPB_YSTRIDE_REG, GetInfo->stride);
			VpuWriteReg(MJPEG_DPB_CSTRIDE_REG, GetInfo->stride/(int)val);
			val = (pJpgDecInfo->frameIdx%GetInfo->numFrameBuffers);		
			VpuWriteReg(MJPEG_DPB_BASE00_REG, GetInfo->frameBufPool[val].bufY);
			VpuWriteReg(MJPEG_DPB_BASE01_REG, GetInfo->frameBufPool[val].bufCb);
			VpuWriteReg(MJPEG_DPB_BASE02_REG, GetInfo->frameBufPool[val].bufCr);
		}

		VpuWriteReg(MJPEG_PIC_START_REG, 1);

		return RETCODE_SUCCESS;
	}


	bufArray = GetInfo->frameBufPool;

	if (handle->SeqInfo.frameBufPool == 0) { // This means frame buffers have not been registered.
		return RETCODE_WRONG_CALL_SEQUENCE;
	}

	if( param->iframeSearchEnable == 1 ) // if iframeSearch is Enable, other bit is ignore;
		val =  param->iframeSearchEnable << 2  & 0x4;
	else
		val =  param->skipframeMode << 3  |  param->iframeSearchEnable << 2  |  param->prescanMode << 1  | param->prescanEnable;
	
	VpuWriteReg( CMD_DEC_PIC_OPTION, val );
	VpuWriteReg( CMD_DEC_PIC_SKIP_NUM, param->skipframeNum );

	if( handle->SeqOpenParam.codecMode == AVC_DEC ) {
		if( SeqOpenParam->reorderEnable == 1 ) {
			VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
		}
	}
	if( handle->SeqOpenParam.codecMode == VC1_DEC ) {
		if( SeqOpenParam->filePlayEnable == 1 ) {
			VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
		}
	}
	else if( handle->SeqOpenParam.codecMode == MP2_DEC ) {
		VpuWriteReg( BIT_DEC_FUNC_CTRL, param->dispReorderBuf << 1 | VpuReadReg( BIT_DEC_FUNC_CTRL ) );	
	}
			
	if( SeqOpenParam->filePlayEnable == 1 )
	{
		VpuWriteReg( CMD_DEC_PIC_CHUNK_SIZE, param->chunkSize );
		if( SeqOpenParam->dynamicAllocEnable == 1 )
			VpuWriteReg( CMD_DEC_PIC_BB_START, param->picStreamBufferAddr );					
		
		VpuWriteReg(CMD_DEC_PIC_START_BYTE, param->picStartByteOffset);
	}


	if ( param->rotConfig.rotFlag || handle->SeqOpenParam.codecMode == MJPG_DEC) {
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_Y, param->rotConfig.bufY);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CB, param->rotConfig.bufCb);
		VpuWriteReg(CMD_DEC_PIC_ROT_ADDR_CR, param->rotConfig.bufCr);
		VpuWriteReg(CMD_DEC_PIC_ROT_STRIDE, param->rotConfig.stride);	 
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, param->rotConfig.rotFlag);
	} else{
		VpuWriteReg(CMD_DEC_PIC_ROT_MODE, 0);
	}

//	hal_cache_flush(0x89000000, 0x300000);
//	hal_cache_flush_all();
//	hal_cache_invalidate(0x88000000, 0x1800000);
//	hal_icache_invalidate_all();
	
	BitIssueCommand(handle, inst_index, handle->SeqOpenParam.codecMode, PIC_RUN);
	//LOG_PRINTF("buf used %x\n", VpuReadReg(0x150));

	return RETCODE_SUCCESS;
}


CodecRetCode VPU_DecBitBufferFlush(DecInstCtl *handle, guchar inst_idx)
{
	//CodecRetCode ret;

	BitIssueCommand(handle, inst_idx, handle->SeqOpenParam.codecMode, DEC_BUF_FLUSH);
	

	#if 1
	while (VpuReadReg(BIT_BUSY_FLAG))
		;
	#endif

	VpuWriteReg((BIT_WR_PTR_0+8*inst_idx), handle->SeqOpenParam.streamBufStartAddr);
	//LOG_PRINTF("0 Writep %x\n", VpuReadReg(0x124));

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_DecClrDispFlag( DecInstCtl *handle,  guchar inst_index, guchar frm_index )
{
	DecSeqGetInfo *SeqInfo=&handle->SeqInfo;
	guint ClrFlag;

	if (handle->BitstreamFormat == STD_MJPG) {
		return RETCODE_SUCCESS;
	}

#if 1
	if (frm_index > (SeqInfo->numFrameBuffers - 1)) {
		return	RETCODE_INVALID_PARAM;
	}

	ClrFlag = ~(1 << frm_index);
#else
	ClrFlag = ~frm_index;
#endif
	ClrFlag &= VpuReadReg(BIT_FRAME_DIS_FLAG_0+4*inst_index);
	VpuWriteReg(BIT_FRAME_DIS_FLAG_0+4*inst_index, ClrFlag);

	return RETCODE_SUCCESS;
	
}

CodecRetCode VPU_DecGiveCommand(DecInstCtl *handle,  guchar inst_index,	CodecCommand cmd, gpointer param)
{

	switch (cmd) 
	{
		case DEC_SET_SPS_RBSP:
			{
				if (handle->SeqOpenParam.codecMode != AVC_DEC) {
					return RETCODE_INVALID_COMMAND;
				}
				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				//SetParaSet(handle, 0, param);
				break;
			}

		case DEC_SET_PPS_RBSP:
			{
				if (handle->SeqOpenParam.codecMode != AVC_DEC) {
					return RETCODE_INVALID_COMMAND;
				}
				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				//SetParaSet(handle, 1, param);
				break;
			}
		case ENABLE_ROTATION:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x10;
			break;
		case DISABLE_ROTATION:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFEF;
			break;
		case ENABLE_MIRRORING:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x10;
			break;
		case DISABLE_MIRRORING:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFEF;
			break;
		case ENABLE_DERING:
			handle->FrameCfgParam.rotConfig.rotFlag |= 0x20;
			break;
		case DISABLE_DERING:
			handle->FrameCfgParam.rotConfig.rotFlag &=0xFFFFFFDF;
			break;
		case SET_MIRROR_DIRECTION:
			{
				MirrorDirection info = *(MirrorDirection*)param;		
				handle->FrameCfgParam.rotConfig.rotFlag &= 0xFFFFFFF3;
				switch (info)
				{
					case MIRDIR_VER:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0x4;
						break;
					case MIRDIR_HOR:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0x8;
						break;
					case MIRDIR_HOR_VER:
						handle->FrameCfgParam.rotConfig.rotFlag |= 0xC;
						break;
					default:
						break;
				}
				break;
			}
		case SET_ROTATION_ANGLE:
			{
				RotatorAngle info =  *(RotatorAngle*)param;
				handle->FrameCfgParam.rotConfig.rotFlag &= 0xFFFFFFFC;
				switch(info)
				{
					case ROTANG_90:
						handle->FrameCfgParam.rotConfig.rotFlag |= 1;
						break;
					case ROTANG_180:
						handle->FrameCfgParam.rotConfig.rotFlag |= 2;
						break;
					case ROTANG_270:
						handle->FrameCfgParam.rotConfig.rotFlag |= 3;
						break;
					default:
						break;
				}
				break;
			}
		case SET_ROTATION_OUTPUT:
			{
				FrameBuffer * frame;

				if (param == 0) {
					return RETCODE_INVALID_PARAM;
				}
				frame = (FrameBuffer *)param;
				handle->FrameCfgParam.rotConfig.bufY = frame->bufY;
				handle->FrameCfgParam.rotConfig.bufCb = frame->bufCb;
				handle->FrameCfgParam.rotConfig.bufCr = frame->bufCr;
				break;
			}
		case SET_ROTATION_STRIDE:
			handle->FrameCfgParam.rotConfig.stride = *(guint *)param;
			break;
		default:
			return RETCODE_INVALID_COMMAND;
	}
	return RETCODE_SUCCESS;
}

CodecRetCode VPU_isr_pic_run(DecInstCtl *handle)
{
	DecFrameOutputInfo *FrmOutInfo=&handle->FrameOutInfo;
	DecSeqGetInfo *SeqInfo=&handle->SeqInfo;
	JpgDecInfo *pJpgDecInfo=&(SeqInfo->JpgDecInfo);
	//CodecRetCode		ret;
	guint		val = 0;
	//OMX_S32			i = 0;
	//OMX_U8       index;

	if (handle->BitstreamFormat == STD_MJPG) {
		val = VpuReadReg(MJPEG_PIC_STATUS_REG);

		if ((val & 0x4) >> 2)
			return RETCODE_WRONG_CALL_SEQUENCE;

		if ((val & 0x1))
		{
			FrmOutInfo->decodingSuccess = 1;
			FrmOutInfo->decPicWidth = pJpgDecInfo->alignedWidth;
			FrmOutInfo->decPicHeight = pJpgDecInfo->alignedHeight;
			FrmOutInfo->indexFrameDecoded = 0;
			FrmOutInfo->indexFrameDisplay = (pJpgDecInfo->frameIdx%SeqInfo->numFrameBuffers);

			SeqInfo->mjpg_consumedByte = VpuReadReg(MJPEG_GBU_TT_CNT_REG)/8;
			pJpgDecInfo->frameIdx++;	
		}
		else
		{
			FrmOutInfo->numOfErrMBs = VpuReadReg(MJPEG_PIC_ERRMB_REG);
			FrmOutInfo->decodingSuccess = 0;
		}

		if (val != 0)
			VpuWriteReg(MJPEG_PIC_STATUS_REG, val);	

		return RETCODE_SUCCESS;
	}


	val = VpuReadReg(RET_DEC_PIC_SIZE);
	SeqInfo->picWidth = val >> 16 & 0xffff ;
	SeqInfo->picHeight =  val & 0xffff ;
	
	val = VpuReadReg( RET_DEC_PIC_SUCCESS );
	FrmOutInfo->notSufficientPsBuffer = val >> 3 & 0x1;
	FrmOutInfo->notSufficientSliceBuffer = val >> 2 & 0x1;

	if ( (val & 0x01) == 0) {
		//pendingInst = 0;
		FrmOutInfo->headerdecodingSuccess = 0;
		return RETCODE_FAILURE;
	}

	if ( handle->FrameCfgParam.prescanEnable == 1){
		FrmOutInfo->prescanresult = VpuReadReg(RET_DEC_PIC_OPTION );
		if ( FrmOutInfo->prescanresult == 0 )
			return RETCODE_FRAME_NOT_COMPLETE;			//not enough frame buffer
	}
	else
		FrmOutInfo->prescanresult = 0;	

	FrmOutInfo->indexFrameDisplay = VpuReadReg(RET_DEC_PIC_FRAME_IDX);
	FrmOutInfo->indexFrameDecoded = VpuReadReg(RET_DEC_PIC_CUR_IDX);
	val = VpuReadReg(RET_DEC_PIC_TYPE);
	FrmOutInfo->picType = val & 0x3F;
	//FrmOutInfo->picParam = ( val >> 18 ) & 0x3FF;
	FrmOutInfo->numOfErrMBs = VpuReadReg(RET_DEC_PIC_ERR_MB);

#if 0
	val = VpuReadReg(RET_DEC_PIC_NEXT_IDX);

	for (i = 0 ; i < 3 ; i++) {
		if (i < pDecInfo->initialInfo.nextDecodedIdxNum) {
			info->indexFrameNextDecoded[i] = ((val >> (i * 5)) & 0x1f);
		} else {
			info->indexFrameNextDecoded[i] = -1;
		}
	}
#endif

	if (handle->SeqOpenParam.codecMode == VC1_DEC && FrmOutInfo->indexFrameDisplay != -3) {
		if (handle->SeqOpenParam.vc1BframeDisplayValid == 0) {
			if ( FrmOutInfo->picType == 2) {
				FrmOutInfo->indexFrameDisplay = -3;
			} else {
				handle->SeqOpenParam.vc1BframeDisplayValid = 1;
			}
		}
	}

	#if 0
	GetInstIndex(handle, &index);
	//LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));
	if ( handle->FrameCfgParam.prescanEnable == 1){
		//if ( FrmOutInfo->prescanresult == 1 )
		{
	//		LOG_PRINTF("***************inst: %d************\n", index);
	//		LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));			
		}
	} else {
		//LOG_PRINTF("%d frame has been decoded %x!\n", (VpuReadReg(RET_DEC_PIC_FRAME_NUM)+1), VpuReadReg(0x120+index*8));
	}
	#endif
	//pendingInst = 0;

	return RETCODE_SUCCESS;	
}

CodecRetCode VPU_isr_seq_init(DecInstCtl *handle)
{
	DecSeqGetInfo *GetInfo=&handle->SeqInfo;
	guint val, val2;
	//CodecRetCode ret;

	if (VpuReadReg(RET_DEC_SEQ_SUCCESS) == 0)
		return RETCODE_FAILURE;
	
	val = VpuReadReg(RET_DEC_SEQ_SRC_SIZE);
	GetInfo->picWidth =  val >> 16 & 0xffff ;
	GetInfo->picHeight =  val & 0xffff ;

	if ( handle->BitstreamFormat != STD_AVC){
		val = VpuReadReg(RET_DEC_SEQ_SRC_F_RATE);
		GetInfo->frameRateDiv = ((val >> 16 ) & 0xffff) + 1;
		GetInfo->frameRateRes = val & 0xffff;
	}else{
		GetInfo->frameRateDiv = VpuReadReg(RET_DEC_SEQ_NUM_UNITS_IN_TICK);
		GetInfo->frameRateRes = VpuReadReg(RET_DEC_SEQ_TIME_SCALE);
	}

	GetInfo->numFrameBuffers = VpuReadReg(RET_DEC_SEQ_FRAME_NEED);
	//printf("numframebuffers %x\n", (unsigned int)GetInfo->numFrameBuffers);
	
	GetInfo->frameBufDelay = VpuReadReg(RET_DEC_SEQ_FRAME_DELAY);

	GetInfo->profile = VpuReadReg(RET_DEC_SEQ_HEADER_REPORT)&0xff;
	
	if (handle->SeqOpenParam.codecMode == AVC_DEC)
	{
		val = VpuReadReg(RET_DEC_SEQ_CROP_LEFT_RIGHT);	
		val2 = VpuReadReg(RET_DEC_SEQ_CROP_TOP_BOTTOM);
		if( val == 0 && val2 == 0 )
		{
			GetInfo->picCropRect.left = 0;
			GetInfo->picCropRect.right = 0;
			GetInfo->picCropRect.top = 0;
			GetInfo->picCropRect.bottom = 0;
		}
		else
		{
			GetInfo->picCropRect.left = ( val>>10 & 0x3FF )*2;
			GetInfo->picCropRect.right = GetInfo->picWidth -   (val & 0x3FF )*2 ;
			GetInfo->picCropRect.top = ( val2>>10 & 0x3FF )*2;
			GetInfo->picCropRect.bottom = GetInfo->picHeight -  ( val2 & 0x3FF )*2 ;
		}
		
		val = (GetInfo->picWidth * GetInfo->picHeight * 3 / 2) / 1024;
		GetInfo->normalSliceSize = val / 4;
		GetInfo->worstSliceSize = val / 2;
	}else if ( handle->SeqOpenParam.codecMode == MJPG_DEC) {
		VpuWriteReg(CMD_DEC_SEQ_CLIP_MODE, 0);
		VpuWriteReg(CMD_DEC_SEQ_SAM_XY, 0);
		GetInfo->mjpg_sourceFormat = VpuReadReg(RET_DEC_SEQ_JPG_PARA) & 7;
		GetInfo->mjpg_thumbNailEnable = VpuReadReg(RET_DEC_SEQ_JPG_THUMB_IND) & 1;
	}

	GetInfo->initialInfoObtained = 1;

	return RETCODE_SUCCESS;
}

void VPU_isr_seq_end(DecInstCtl *handle) 
{
	//FreeCodecInstance(pCodecInst);

	return;
}

void VPU_isr_flush_buffer(void)
{
    return;
}

void EncodeHeader(EncInstCtl *handle, guchar inst_index, EncHeaderParam *HeaderParam)
{
    EncInfo * pEncInfo=&(handle->pEncInfo);
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);
    gint frameCroppingFlag=0;

    if( pEncInfo->dynamicAllocEnable == 1 ) {
        VpuWriteReg( CMD_ENC_HEADER_BB_START, OpenParam->bitstreamBuffer);
        VpuWriteReg( CMD_ENC_HEADER_BB_SIZE, OpenParam->bitstreamBufferSize);
    }

    if( HeaderParam->headerType == 0 && handle->BitstreamFormat == STD_AVC) {
        guint CropV, CropH;
        if (OpenParam->EncStdParam.avcParam.avc_frameCroppingFlag == 1) {
            frameCroppingFlag = 1;
            CropH = OpenParam->EncStdParam.avcParam.avc_frameCropLeft << 16;
            CropH |= OpenParam->EncStdParam.avcParam.avc_frameCropRight;
            CropV = OpenParam->EncStdParam.avcParam.avc_frameCropTop << 16;
            CropV |= OpenParam->EncStdParam.avcParam.avc_frameCropBottom;
            VpuWriteReg( CMD_ENC_HEADER_FRAME_CROP_H, CropH );
            VpuWriteReg( CMD_ENC_HEADER_FRAME_CROP_V, CropV );
        }
    }
#ifdef ENC_VOS_HEADER_LEVEL_PROFILE
    if (HeaderParam->headerType == VOS_HEADER ||
        HeaderParam->headerType == SPS_RBSP) {
        gint data =0;
        data = (((HeaderParam->userProfileLevelIndication&0xFF)<<8) |
                ((HeaderParam->userProfileLevelEnable&0x01)<<4) |
                ((HeaderParam->headerType | (frameCroppingFlag << 2))&0x0F));
        VpuWriteReg(CMD_ENC_HEADER_CODE, data);
    } else {
        VpuWriteReg(CMD_ENC_HEADER_CODE, HeaderParam->headerType | (frameCroppingFlag << 2)); // 0: SPS, 1: PPS
    }
#else
    VpuWriteReg(CMD_ENC_HEADER_CODE, HeaderParam->headerType | (frameCroppingFlag << 2)); // 0: SPS, 1: PPS
#endif

    EncBitIssueCommand(handle, inst_index, OpenParam->codecMode, ENC_HEADER);
	while(VPU_IsBusy(handle, 0)) 
		;

	return;
}

CodecRetCode VPU_EncInit(EncInstCtl *handle, guchar inst_index)
{
    EncInfo * pEncInfo=&(handle->pEncInfo);
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);
	JpgEncInfo *pJpgInfo = &(handle->jpgInfo);
	EncInitialInfo *info = &(handle->initialInfo);
    gint picWidth;
    gint picHeight;
    guint  data;
//    CodecRetCode ret;
//   guchar * tableBuf = 0;
//    guchar * tableBuf2 = 0;

    if (pEncInfo->initialInfoObtained) {
        return RETCODE_CALLED_BEFORE;
    }

    picWidth = OpenParam->picWidth;
    picHeight = OpenParam->picHeight;
	if (OpenParam->codecMode == MJPG_ENC) {
		VpuWriteReg(MJPEG_BBC_BAS_ADDR_REG, pEncInfo->streamBufStartAddr);
		VpuWriteReg(MJPEG_BBC_END_ADDR_REG, pEncInfo->streamBufEndAddr);
		VpuWriteReg(MJPEG_BBC_WR_PTR_REG, pEncInfo->streamBufStartAddr);
		VpuWriteReg(MJPEG_BBC_RD_PTR_REG, pEncInfo->streamBufStartAddr);
		VpuWriteReg(MJPEG_BBC_CUR_POS_REG, 0);
		VpuWriteReg(MJPEG_BBC_DATA_CNT_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
		VpuWriteReg(MJPEG_BBC_EXT_ADDR_REG, pEncInfo->streamBufStartAddr);
		VpuWriteReg(MJPEG_BBC_INT_ADDR_REG, 0);

		//JpgEncGbuResetReg
		VpuWriteReg(MJPEG_GBU_BT_PTR_REG, 0);
		VpuWriteReg(MJPEG_GBU_WD_PTR_REG, 0);
		VpuWriteReg(MJPEG_GBU_BBSR_REG, 0);

		VpuWriteReg(MJPEG_GBU_BBER_REG, ((256 / 4) * 2) - 1);
		VpuWriteReg(MJPEG_GBU_BBIR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte
		VpuWriteReg(MJPEG_GBU_BBHR_REG, 256 / 4);	// 64 * 4 byte == 32 * 8 byte	

		VpuWriteReg(MJPEG_PIC_CTRL_REG, 0x18);

		VpuWriteReg(MJPEG_PIC_SIZE_REG, (pJpgInfo->alignedWidth<<16) | pJpgInfo->alignedHeight);
		VpuWriteReg(MJPEG_ROT_INFO_REG, 0);	

		if (pJpgInfo->format == YCBCR400) {
			pJpgInfo->compInfo[1] = 0;
			pJpgInfo->compInfo[2] = 0;
		}
		else {
			pJpgInfo->compInfo[1] = 5;
			pJpgInfo->compInfo[2] = 5;
		}

		if (pJpgInfo->format == YCBCR400)
			pJpgInfo->compNum = 1;
		else
			pJpgInfo->compNum = 3;	

		if (pJpgInfo->format == YCBCR420) {
			pJpgInfo->mcuBlockNum = 6;
			pJpgInfo->compInfo[0] = 10;
			pJpgInfo->busReqNum = 2;
		} 
		else if (pJpgInfo->format == YCBCR422H) {
			pJpgInfo->mcuBlockNum = 4;
			pJpgInfo->busReqNum = 3;
			pJpgInfo->compInfo[0] = 9;
		} else if (pJpgInfo->format == YCBCR422V) {
			pJpgInfo->mcuBlockNum = 4;
			pJpgInfo->busReqNum  = 3;
			pJpgInfo->compInfo[0] = 6;
			pJpgInfo->compInfo[0] = 6;
		} else if (pJpgInfo->format == YCBCR444) {
			pJpgInfo->mcuBlockNum = 3;
			pJpgInfo->compInfo[0] = 5;
			pJpgInfo->busReqNum = 4;
		} else if (pJpgInfo->format == YCBCR400) {
			pJpgInfo->mcuBlockNum = 1;
			pJpgInfo->busReqNum = 4;
			pJpgInfo->compInfo[0] = 5;
		}
		VpuWriteReg(MJPEG_MCU_INFO_REG, pJpgInfo->mcuBlockNum << 16 | pJpgInfo->compNum << 12 
			| pJpgInfo->compInfo[0] << 8 | pJpgInfo->compInfo[1] << 4 | pJpgInfo->compInfo[2]);	

		VpuWriteReg(MJPEG_SCL_INFO_REG, 0);
		VpuWriteReg(MJPEG_DPB_CONFIG_REG, IMAGE_ENDIAN << 1 | IMAGE_INTERLEAVE);
		VpuWriteReg(MJPEG_RST_INTVAL_REG, pJpgInfo->rstIntval);
		VpuWriteReg(MJPEG_BBC_CTRL_REG, ((STREAM_ENDIAN & 3) << 1) | 1);
		VpuWriteReg(MJPEG_OP_INFO_REG, pJpgInfo->busReqNum);

		if (!JpgEncLoadHuffTab(handle, pJpgInfo)) {
			return RETCODE_INVALID_PARAM;
		}	

		if (!JpgEncLoadQMatTab(handle, pJpgInfo)) {
			return RETCODE_INVALID_PARAM;
		}

		info->minFrameBufferCount = 0;	
		pEncInfo->initialInfoObtained = 1;

		return RETCODE_SUCCESS;
	}
    VpuWriteReg(CMD_ENC_SEQ_BB_START, pEncInfo->streamBufStartAddr);
    VpuWriteReg(CMD_ENC_SEQ_BB_SIZE, pEncInfo->streamBufSize / 1024); // size in KB

    // Rotation Left 90 or 270 case : Swap XY resolution for VPU internal usage
    if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270)
        data = (picHeight << 16) | picWidth;
    else
        data = (picWidth << 16) | picHeight;
    VpuWriteReg(CMD_ENC_SEQ_SRC_SIZE, data);
    VpuWriteReg(CMD_ENC_SEQ_SRC_F_RATE, OpenParam->frameRateInfo);

    if (handle->BitstreamFormat == STD_MP4) {
        VpuWriteReg(CMD_ENC_SEQ_COD_STD, 3);
        data = OpenParam->EncStdParam.mp4Param.mp4_intraDcVlcThr << 2 |
            OpenParam->EncStdParam.mp4Param.mp4_reversibleVlcEnable << 1 |
            OpenParam->EncStdParam.mp4Param.mp4_dataPartitionEnable;
        data |= ((OpenParam->EncStdParam.mp4Param.mp4_hecEnable >0)? 1:0)<<5;
        data |= ((OpenParam->EncStdParam.mp4Param.mp4_verid == 2)? 0:1) << 6;
        VpuWriteReg(CMD_ENC_SEQ_MP4_PARA, data);
    }
    else if (handle->BitstreamFormat == STD_H263) {
        VpuWriteReg(CMD_ENC_SEQ_COD_STD, 11);
        data = OpenParam->EncStdParam.h263Param.h263_annexIEnable << 3 |
            OpenParam->EncStdParam.h263Param.h263_annexJEnable << 2 |
            OpenParam->EncStdParam.h263Param.h263_annexKEnable << 1|
            OpenParam->EncStdParam.h263Param.h263_annexTEnable;        // FIXME : oring RotRightAngle << 4
        VpuWriteReg(CMD_ENC_SEQ_263_PARA, data);
    }
    else if (handle->BitstreamFormat == STD_AVC) {
        VpuWriteReg(CMD_ENC_SEQ_COD_STD, 0);
        data = (OpenParam->EncStdParam.avcParam.avc_deblkFilterOffsetBeta & 15) << 12 |
            (OpenParam->EncStdParam.avcParam.avc_deblkFilterOffsetAlpha & 15) << 8 |
            OpenParam->EncStdParam.avcParam.avc_disableDeblk << 6 |
            OpenParam->EncStdParam.avcParam.avc_constrainedIntraPredFlag << 5 |
            (OpenParam->EncStdParam.avcParam.avc_chromaQpOffset & 31);
        VpuWriteReg(CMD_ENC_SEQ_264_PARA, data);
    } 

    if( handle->BitstreamFormat != STD_MJPG )
    {
        data = OpenParam->slicemode.sliceSize << 2 |
            OpenParam->slicemode.sliceSizeMode << 1 |
            OpenParam->slicemode.sliceMode;
        VpuWriteReg(CMD_ENC_SEQ_SLICE_MODE, data);
        VpuWriteReg(CMD_ENC_SEQ_GOP_NUM, OpenParam->gopSize);
    }

    if (OpenParam->bitRate) { // rate control enabled
        data = (!OpenParam->enableAutoSkip) << 31 |
            OpenParam->initialDelay << 16 |
            OpenParam->bitRate << 1 | 1;
        VpuWriteReg(CMD_ENC_SEQ_RC_PARA, data);
    }
    else {
        VpuWriteReg(CMD_ENC_SEQ_RC_PARA, 0);
    }
    VpuWriteReg(CMD_ENC_SEQ_RC_BUF_SIZE, OpenParam->vbvBufferSize);
    VpuWriteReg(CMD_ENC_SEQ_INTRA_REFRESH, OpenParam->intraRefresh);

    if(OpenParam->rcIntraQp>=0)    
        data = (1 << 5);
    else
        data = 0;
    VpuWriteReg(CMD_ENC_SEQ_INTRA_QP, OpenParam->rcIntraQp);

    if (OpenParam->codecMode == AVC_ENC) {
        data |= (OpenParam->EncStdParam.avcParam.avc_audEnable << 2);
    }
    if(OpenParam->userQpMax) {
        data |= (1<<6);
        VpuWriteReg(CMD_ENC_SEQ_RC_QP_MAX, OpenParam->userQpMax);
    } 
    if(OpenParam->userGamma) {
        data |= (1<<7);
        VpuWriteReg(CMD_ENC_SEQ_RC_GAMMA, OpenParam->userGamma);
    }
    VpuWriteReg(CMD_ENC_SEQ_OPTION, data);
    VpuWriteReg(CMD_ENC_SEQ_RC_INTERVAL_MODE, (OpenParam->MbInterval<<2) | OpenParam->RcIntervalMode);
    VpuWriteReg(CMD_ENC_SEQ_ME_OPTION, (OpenParam->MEUseZeroPmv << 2) | OpenParam->MESearchRange);
    VpuWriteReg(CMD_ENC_SEQ_INTRA_WEIGHT, OpenParam->IntraCostWeight);

    EncBitIssueCommand(handle, inst_index, OpenParam->codecMode, SEQ_INIT);

	return RETCODE_SUCCESS;
}

CodecRetCode VPU_EncRegisterFrameBuffer(
        EncInstCtl *handle,
        BitPrcBuffer *VPUbuffer,
        guchar inst_index, 
        PhysicalAddress subSampBaseA,
        PhysicalAddress subSampBaseB)
{
    EncInfo * pEncInfo=&(handle->pEncInfo);
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);
	EncInitialInfo *GetInfo=&handle->initialInfo;
	FrameBuffer * bufArray;
    gint i;
    guint val;
//    gchar frameAddr[22][3][4];
	guint address[150];
	guint tmp;

	bufArray = pEncInfo->frameBufPool;
    if (pEncInfo->frameBufPool) {
        return RETCODE_CALLED_BEFORE;
    }

    if (!pEncInfo->initialInfoObtained) {
        return RETCODE_WRONG_CALL_SEQUENCE;
    }

    pEncInfo->numFrameBuffers = GetInfo->minFrameBufferCount;
    pEncInfo->stride = GetInfo->stride;

	if (OpenParam->codecMode == MJPG_ENC)
		 return RETCODE_SUCCESS;
    // Let the codec know the addresses of the frame buffers.
    if (OpenParam->codecMode != MJPG_ENC)
    {
        // Let the decoder know the addresses of the frame buffers.
        for (i=0; i<pEncInfo->numFrameBuffers; i++) {
			address[i*3] =  bufArray[i].bufY;
			address[i*3+1] =  bufArray[i].bufCb;
			address[i*3+2] =  bufArray[i].bufCr;
        }
		tmp = (pEncInfo->numFrameBuffers * 3 + 1 ) >> 1;
		for (i = 0; i < tmp; i++) {
			*(guint *)(VPUbuffer->para_buf + i * 2 * 4) =  address[i*2+1];
			*(guint *)(VPUbuffer->para_buf + i * 2 * 4 + 4) =  address[i*2];
		}    
	}
    // Tell the codec how much frame buffers were allocated.
    VpuWriteReg(CMD_SET_FRAME_BUF_NUM, pEncInfo->numFrameBuffers);
    VpuWriteReg(CMD_SET_FRAME_BUF_STRIDE, pEncInfo->stride);

#ifdef USE_SECOND_AXI
	VPUbuffer->dbky_buf = 0;
	VPUbuffer->dbkc_buf = 0;			
	VPUbuffer->bit_buf = 0;
	VPUbuffer->ipacdc_buf = 0;
	VPUbuffer->ovl_buf = 0;
	VPUbuffer->btp_buf = 0;

	//MEMSET(&(GetInfo->second_axi_use), 0, sizeof(GetInfo->second_axi_use));

	switch(handle->BitstreamFormat){
		case STD_AVC:
			pEncInfo->secAxiUse.useBitEnable = 1;
			pEncInfo->secAxiUse.useDbkYEnable = 1;
			pEncInfo->secAxiUse.useDbkCEnable = 1;
			pEncInfo->secAxiUse.useIpEnable = 1;
			pEncInfo->secAxiUse.useOvlEnable = 0;
			pEncInfo->secAxiUse.useBtpEnable = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_MP4:
			pEncInfo->secAxiUse.useBitEnable = 1;
			pEncInfo->secAxiUse.useDbkYEnable = 0;
			pEncInfo->secAxiUse.useDbkCEnable = 0;
			pEncInfo->secAxiUse.useIpEnable = 1;
			pEncInfo->secAxiUse.useOvlEnable = 0;
			pEncInfo->secAxiUse.useBtpEnable = 0;
			VPUbuffer->bit_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;
			break;
		case STD_H263:
			pEncInfo->secAxiUse.useBitEnable = 1;
			pEncInfo->secAxiUse.useDbkYEnable = 1;
			pEncInfo->secAxiUse.useDbkCEnable = 1;
			pEncInfo->secAxiUse.useIpEnable = 1;
			pEncInfo->secAxiUse.useOvlEnable = 0;
			pEncInfo->secAxiUse.useBtpEnable = 0;
			VPUbuffer->dbky_buf = SECOND_AXI_BASE_ADDR;
			VPUbuffer->dbkc_buf = VPUbuffer->dbky_buf + DBKY_INTERNAL_BUF_SIZE;			
			VPUbuffer->bit_buf = VPUbuffer->dbkc_buf + DBKC_INTERNAL_BUF_SIZE;
			VPUbuffer->ipacdc_buf = VPUbuffer->bit_buf + BIT_INTERNAL_BUF_SIZE;						
			break;
		case STD_MJPG:
			pEncInfo->secAxiUse.useBitEnable = 0;
			pEncInfo->secAxiUse.useDbkYEnable = 0;
			pEncInfo->secAxiUse.useDbkCEnable = 0;
			pEncInfo->secAxiUse.useIpEnable = 0;
			pEncInfo->secAxiUse.useOvlEnable = 0;
			pEncInfo->secAxiUse.useBtpEnable = 0;
			break;
		default:
			break;
			
	}

	VpuWriteReg(BIT_AXI_SRAM_USE, pEncInfo->secAxiUse.useBtpEnable << 13 | pEncInfo->secAxiUse.useOvlEnable << 12 | 
		pEncInfo->secAxiUse.useDbkCEnable << 11 | pEncInfo->secAxiUse.useDbkYEnable << 10 | 
		pEncInfo->secAxiUse.useIpEnable << 9 | pEncInfo->secAxiUse.useBitEnable << 8 | 
		pEncInfo->secAxiUse.useBtpEnable << 5 | pEncInfo->secAxiUse.useOvlEnable << 4 |
		pEncInfo->secAxiUse.useDbkCEnable << 3 | pEncInfo->secAxiUse.useDbkYEnable << 2 | 
		pEncInfo->secAxiUse.useIpEnable << 1 | pEncInfo->secAxiUse.useBitEnable );


	VpuWriteReg( CMD_SET_FRAME_AXI_BIT_ADDR, VPUbuffer->bit_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_ACDC_ADDR, VPUbuffer->ipacdc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKY_ADDR, VPUbuffer->dbky_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_DBKC_ADDR, VPUbuffer->dbkc_buf & 0x1FFFFFFF);
	VpuWriteReg( CMD_SET_FRAME_AXI_OVL_ADDR, VPUbuffer->ovl_buf & 0x1FFFFFFF);	
	VpuWriteReg( CMD_SET_FRAME_AXI_BTP_ADDR, VPUbuffer->btp_buf & 0x1FFFFFFF);	

#endif

    // Maverick Cache Configuration
    val = (pEncInfo->cacheConfig.luma.cfg.PageSizeX << 28) |
            (pEncInfo->cacheConfig.luma.cfg.PageSizeY << 24) |
            (pEncInfo->cacheConfig.luma.cfg.CacheSizeX << 20) |
            (pEncInfo->cacheConfig.luma.cfg.CacheSizeY << 16) |
            (pEncInfo->cacheConfig.chroma.cfg.PageSizeX << 12) |
            (pEncInfo->cacheConfig.chroma.cfg.PageSizeY << 8) |
            (pEncInfo->cacheConfig.chroma.cfg.CacheSizeX << 4) |
            (pEncInfo->cacheConfig.chroma.cfg.CacheSizeY << 0);
    VpuWriteReg(CMD_SET_FRAME_CACHE_SIZE, val);
    val = (pEncInfo->cacheConfig.Bypass << 4) |
            (pEncInfo->cacheConfig.DualConf << 2) |
            (pEncInfo->cacheConfig.PageMerge << 0);
    val = val << 24;
    val |= (pEncInfo->cacheConfig.luma.cfg.BufferSize << 16) |
            (pEncInfo->cacheConfig.chroma.cfg.BufferSize << 8) |
            (pEncInfo->cacheConfig.chroma.cfg.BufferSize << 8);
    VpuWriteReg(CMD_SET_FRAME_CACHE_CONFIG, val);

    // Magellan Encoder specific : Subsampling ping-pong Buffer
    // Set Sub-Sampling buffer for ME-Reference and DBK-Reconstruction
    // BPU will swap below two buffer internally every pic by pic
    VpuWriteReg(CMD_SET_FRAME_SUBSAMP_A, subSampBaseA);
    VpuWriteReg(CMD_SET_FRAME_SUBSAMP_B, subSampBaseB);

    EncBitIssueCommand(handle, inst_index, OpenParam->codecMode, SET_FRAME_BUF);

	return RETCODE_SUCCESS;
}

CodecRetCode SetEncOpenParamDefault(guchar format, EncOpenParam *pEncOP, EncConfigParam *pEncConfig)
{
    guchar bitFormat;

    bitFormat = format;
    pEncOP->picWidth = pEncConfig->picWidth;
    pEncOP->picHeight = pEncConfig->picHeight;
    pEncOP->picQpY = 23;
    pEncOP->frameRateInfo = pEncConfig->framerate_denominator / pEncConfig->framerate_numerator;
    pEncOP->bitRate = pEncConfig->bitrate;
    pEncOP->initialDelay = 0;
    pEncOP->vbvBufferSize = pEncConfig->buf_size;          // 0 = ignore
    pEncOP->enableAutoSkip = 1;         // for compare with C-model ( C-model = only 1 )
    pEncOP->gopSize = pEncConfig->gop_size;                   // only first picture is I
    pEncOP->slicemode.sliceMode = 1;        // 1 slice per picture
    pEncOP->slicemode.sliceSizeMode = 1;
    pEncOP->slicemode.sliceSize = 115;
    pEncOP->intraRefresh = 0;
    pEncOP->rcIntraQp = -1;             // disable == -1
    pEncOP->userQpMax       = 0;
    pEncOP->userGamma       = (3*32768/4);     //  (0*32768 < gamma < 1*32768)
    pEncOP->RcIntervalMode= 1;                      // 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    pEncOP->MbInterval  = 0;
    // Magellan Architecture specific
    pEncOP->MESearchRange = 0;
    pEncOP->MEUseZeroPmv = 1;
    pEncOP->IntraCostWeight = 400;
    // Standard specific
    if( bitFormat == STD_MP4 ) {
        pEncOP->EncStdParam.mp4Param.mp4_dataPartitionEnable = 0;
        pEncOP->EncStdParam.mp4Param.mp4_reversibleVlcEnable = 0;
        pEncOP->EncStdParam.mp4Param.mp4_intraDcVlcThr = 0;
        pEncOP->EncStdParam.mp4Param.mp4_hecEnable  = 0;
        pEncOP->EncStdParam.mp4Param.mp4_verid = 2;
    }
    else if( bitFormat == STD_H263 ) {
        pEncOP->EncStdParam.h263Param.h263_annexIEnable = 0;
        pEncOP->EncStdParam.h263Param.h263_annexJEnable = 0;
        pEncOP->EncStdParam.h263Param.h263_annexKEnable = 0;
        pEncOP->EncStdParam.h263Param.h263_annexTEnable = 0;
    }
    else if( bitFormat == STD_AVC ) {
        pEncOP->EncStdParam.avcParam.avc_constrainedIntraPredFlag = 0;
        pEncOP->EncStdParam.avcParam.avc_disableDeblk = 1;
        pEncOP->EncStdParam.avcParam.avc_deblkFilterOffsetAlpha = 6;
        pEncOP->EncStdParam.avcParam.avc_deblkFilterOffsetBeta = 0;
        pEncOP->EncStdParam.avcParam.avc_chromaQpOffset = 10;
        pEncOP->EncStdParam.avcParam.avc_audEnable = 0;
        pEncOP->EncStdParam.avcParam.avc_frameCroppingFlag = 0;
        pEncOP->EncStdParam.avcParam.avc_frameCropLeft = 0;
        pEncOP->EncStdParam.avcParam.avc_frameCropRight = 0;
        pEncOP->EncStdParam.avcParam.avc_frameCropTop = 0;
        pEncOP->EncStdParam.avcParam.avc_frameCropBottom = 0;
    	if (pEncOP->picHeight == 1080) {
        	// In case of AVC encoder, when we want to use unaligned display width(For example, 1080),
        	// frameCroppingFlag parameters should be adjusted to displayable rectangle
   	        if (pEncOP->EncStdParam.avcParam.avc_frameCroppingFlag == 0) {
       	        pEncOP->EncStdParam.avcParam.avc_frameCroppingFlag = 1;
           	    // frameCropBottomOffset = picHeight(MB-aligned) - displayable rectangle height
               	pEncOP->EncStdParam.avcParam.avc_frameCropBottom = 8;
           	}
    	}
		
    }
    else if( bitFormat == STD_MJPG )
    {
        pEncOP->EncStdParam.mjpgParam.mjpg_sourceFormat = pEncConfig->mjpgChromaFormat;
        pEncOP->EncStdParam.mjpgParam.njpg_restartInterval = 60;   
		jpgGetHuffTable(&pEncOP->EncStdParam.mjpgParam);
        jpgGetQMatrix(&pEncOP->EncStdParam.mjpgParam);
		jpgGetCInfoTable(&pEncOP->EncStdParam.mjpgParam);
    }
    else {
        return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;

}


CodecRetCode CheckEncParam(EncInstCtl *handle, EncParam * param)
{
//    EncInfo *pEncInfo=&(handle->pEncInfo);
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);

    if (param == 0) {
        return RETCODE_INVALID_PARAM;
    }
    if (param->skipPicture != 0 && param->skipPicture != 1) {
        return RETCODE_INVALID_PARAM;
    }
    if (param->skipPicture == 0) {
        if (param->sourceFrame == 0) {
            return RETCODE_INVALID_FRAME_BUFFER;
        }
        if (param->forceIPicture != 0 && param->forceIPicture != 1) {
            return RETCODE_INVALID_PARAM;
        }
    }

    if (OpenParam->bitRate == 0) { // no rate control
        if (OpenParam->codecMode == MP4_ENC) {
            if (param->quantParam < 1 || param->quantParam > 31) {
                return RETCODE_INVALID_PARAM;
            }
        }
        else { // AVC_ENC
            if (param->quantParam < 0 || param->quantParam > 51) {
                return RETCODE_INVALID_PARAM;
            }
        }
    }
    return RETCODE_SUCCESS;
}


void Vpu_EncSetHostParaAddr(PhysicalAddress baseAddr, PhysicalAddress paraAddr)
{
	*(guint *)(baseAddr) =  paraAddr;
	*(guint *)(baseAddr + 4) =  0;
}

void VPU_EncPutHeader(EncInstCtl *handle, guchar inst_index)
{
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);
	EncHeaderParam encHeaderParam;

    if( handle->BitstreamFormat == STD_MP4 ) {
#ifdef ENC_VOS_HEADER_LEVEL_PROFILE
        int mbPicNum = (OpenParam->picWidth+15)/16*(OpenParam->picHeight+15)/16;

        encHeaderParam.headerType = VOS_HEADER;
        encHeaderParam.userProfileLevelEnable = 0;
        if (OpenParam->picWidth <=176 && OpenParam->picHeight <=144 && mbPicNum*OpenParam->frameRateInfo <= 1485)
            encHeaderParam.userProfileLevelIndication = 1;
        else if (OpenParam->picWidth <=352 && OpenParam->picHeight <=288 && mbPicNum*OpenParam->frameRateInfo <= 5940)
            encHeaderParam.userProfileLevelIndication = 2;
        else if (OpenParam->picWidth <=352 && OpenParam->picHeight <=288 && mbPicNum*OpenParam->frameRateInfo <= 11880)
            encHeaderParam.userProfileLevelIndication = 3;
        else if (OpenParam->picWidth <=640 && OpenParam->picHeight <=480 && mbPicNum*OpenParam->frameRateInfo <= 36000)
            encHeaderParam.userProfileLevelIndication = 4;
        else if (OpenParam->picWidth <=720 && OpenParam->picHeight <=576 && mbPicNum*OpenParam->frameRateInfo <= 40500)
            encHeaderParam.userProfileLevelIndication = 5;
        else
            encHeaderParam.userProfileLevelIndication = 6;

        //VPU_EncGiveCommand(handle, ENC_PUT_MP4_HEADER, &encHeaderParam);
        EncodeHeader(handle, inst_index, &encHeaderParam);
		#if 0
        if( OpenParam->ringBufferEnable == 0 )
            ReadBsResetBufHelper( bsFp, encHeaderParam.buf, encHeaderParam.size );    
		#endif
#endif
        encHeaderParam.headerType = VOL_HEADER;
        EncodeHeader(handle, inst_index, &encHeaderParam);
		#if 0
        if( OpenParam->ringBufferEnable == 0 )
            ReadBsResetBufHelper( bsFp, encHeaderParam.buf, encHeaderParam.size );  
		#endif
    }else if( handle->BitstreamFormat == STD_AVC ) {
#ifdef ENC_VOS_HEADER_LEVEL_PROFILE
        int mbNumX = (OpenParam->picWidth+15)/16;
        int mbNumY = (OpenParam->picHeight+15)/16;
        int mbPicNum = (mbNumX*mbNumY);
        int mbps = mbPicNum*OpenParam->frameRateInfo;
        encHeaderParam.userProfileLevelEnable = 1;
        if (mbPicNum <=99 && mbPicNum*OpenParam->frameRateInfo <= 1485)
            encHeaderParam.userProfileLevelIndication = 10;
        else if (mbPicNum <=396 && mbPicNum*OpenParam->frameRateInfo <= 11880)
            encHeaderParam.userProfileLevelIndication = 20;
        else if (mbPicNum <=1620 && mbPicNum*OpenParam->frameRateInfo <= 40500)
            encHeaderParam.userProfileLevelIndication = 30; //SD
        else if (mbPicNum <=3600 && mbPicNum*OpenParam->frameRateInfo <= 108000)
            encHeaderParam.userProfileLevelIndication = 31; // HD
        else if (mbPicNum <=8192 && mbPicNum*OpenParam->frameRateInfo <= 245760)
            encHeaderParam.userProfileLevelIndication = 40; // Full HD
        else
            printf("Unsupported Level\n");
#endif
        encHeaderParam.headerType = SPS_RBSP;
        EncodeHeader(handle, inst_index, &encHeaderParam);
		#if 0
        if( OpenParam->ringBufferEnable == 0 )
            ReadBsResetBufHelper( bsFp, encHeaderParam.buf, encHeaderParam.size );       
		#endif
        encHeaderParam.headerType = PPS_RBSP;
        EncodeHeader(handle, inst_index, &encHeaderParam);
		#if 0
        if( OpenParam->ringBufferEnable == 0 )
            ReadBsResetBufHelper( bsFp, encHeaderParam.buf, encHeaderParam.size );  
		#endif
    }else if( handle->BitstreamFormat == STD_MJPG ) {
		 EncParamSet encHeaderParam = {0};
		 encHeaderParam.size = OpenParam->bitstreamBufferSize;
		 encHeaderParam.pParaSet = (gpointer)hal_vaddr_to_paddr((gpointer)OpenParam->bitstreamBuffer);
		 JpgEncEncodeHeader(handle, &encHeaderParam); // return exact header size int endHeaderparam.siz;
	 }
}

CodecRetCode VPU_EncStartOneFrame(EncInstCtl *handle, guchar inst_index )
{
    EncInfo * pEncInfo=&(handle->pEncInfo);
	EncParam *pEncParam = &(handle->encParam);
	EncOpenParam *OpenParam = &(handle->EncSeqOpenParam);
	JpgEncInfo *pJpgInfo = &(handle->jpgInfo);
    FrameBuffer * pSrcFrame;
//    guint data = 0;
    guint rotMirEnable;
    guint rotMirMode;
    guint val;
    CodecRetCode ret;

    // When doing pre-rotation, mirroring is applied first and rotation later,
    // vice versa when doing post-rotation.
    // For consistency, pre-rotation is converted to post-rotation orientation.
    //static Uint32 rotatorModeConversion[] = {
    //  0, 1, 2, 3, 4, 7, 6, 5,
    //  6, 5, 4, 7, 2, 3, 0, 1
    //};

	if (OpenParam->codecMode != MJPG_ENC) {
		if (pEncInfo->frameBufPool == 0) { // This means frame buffers have not been registered.
			return RETCODE_WRONG_CALL_SEQUENCE;
		}
	}

    ret = CheckEncParam(handle, pEncParam);
    if (ret != RETCODE_SUCCESS) {
        return ret;
    }

    pSrcFrame = pEncParam->sourceFrame;
    rotMirEnable = 0;
    rotMirMode = 0;
    if (pEncInfo->rotationEnable) {
        rotMirEnable = 0x10; // Enable rotator
        switch (pEncInfo->rotationAngle) {
            case 0:
                rotMirMode |= 0x0;
                break;
            case 90:
                rotMirMode |= 0x1;
                break;
            case 180:
                rotMirMode |= 0x2;
                break;
            case 270:
                rotMirMode |= 0x3;
                break;
        }
    }
    if (pEncInfo->mirrorEnable) {
        rotMirEnable = 0x10; // Enable rotator
        switch (pEncInfo->mirrorDirection) {
            case MIRDIR_NONE :
                rotMirMode |= 0x0;
                break;
            case MIRDIR_VER :
                rotMirMode |= 0x4;
                break;
            case MIRDIR_HOR :
                rotMirMode |= 0x8;
                break;
            case MIRDIR_HOR_VER :
                rotMirMode |= 0xc;
                break;
        }
    }

	if (OpenParam->codecMode == MJPG_ENC) {
		if (rotMirMode & 1)
			VpuWriteReg(MJPEG_PIC_SIZE_REG, pJpgInfo->alignedHeight<<16 | pJpgInfo->alignedWidth);
		else
			VpuWriteReg(MJPEG_PIC_SIZE_REG, pJpgInfo->alignedWidth<<16 | pJpgInfo->alignedHeight);
		VpuWriteReg(MJPEG_ROT_INFO_REG, (rotMirEnable|rotMirMode));

		if (rotMirEnable)
			pJpgInfo->format = (pJpgInfo->format==YCBCR422H) ? YCBCR422V : (pJpgInfo->format==YCBCR422V) ? YCBCR422H : pJpgInfo->format;

		if (pJpgInfo->format == YCBCR422H) {
			if (rotMirMode & 1) 
				pJpgInfo->compInfo[0] = 6;
			else
				pJpgInfo->compInfo[0] = 9;
		} else if (pJpgInfo->format == YCBCR422V) {
			if (rotMirMode & 1) 
				pJpgInfo->compInfo[0] = 9;
			else
				pJpgInfo->compInfo[0] = 6;
		}

		VpuWriteReg(MJPEG_MCU_INFO_REG, pJpgInfo->mcuBlockNum << 16 | pJpgInfo->compNum << 12 
			| pJpgInfo->compInfo[0] << 8 | pJpgInfo->compInfo[1] << 4 | pJpgInfo->compInfo[2]);

		VpuWriteReg(MJPEG_DPB_BASE00_REG, pSrcFrame->bufY);
		VpuWriteReg(MJPEG_DPB_BASE01_REG, pSrcFrame->bufCb);
		VpuWriteReg(MJPEG_DPB_BASE02_REG, pSrcFrame->bufCr);

		val = (pJpgInfo->format == YCBCR420 || pJpgInfo->format == YCBCR422H || pJpgInfo->format == YCBCR400) ? 2 : 1;
		VpuWriteReg(MJPEG_DPB_YSTRIDE_REG, handle->initialInfo.stride);
		VpuWriteReg(MJPEG_DPB_CSTRIDE_REG, handle->initialInfo.stride/(int)val);
		
		VpuWriteReg(MJPEG_PIC_START_REG, 1);

		return RETCODE_SUCCESS;
	}
    //rotMirMode = rotatorModeConversion[rotMirMode];
    //rotMirMode |= rotMirEnable;
    VpuWriteReg(CMD_ENC_PIC_ROT_MODE, rotMirMode);

    VpuWriteReg(CMD_ENC_PIC_QS, pEncParam->quantParam);

    if (pEncParam->skipPicture) {
        VpuWriteReg(CMD_ENC_PIC_OPTION,
            (pEncParam->enReportSliceInfo<<5) | (pEncParam->enReportMVInfo<<4) |(pEncParam->enReportMBInfo<<3) | 1);
    }
    else {
        // Registering Source Frame Buffer information
        // Hide GDI IF under FW level
        VpuWriteReg(CMD_ENC_PIC_SRC_INDEX, 0);
        VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_Y, pSrcFrame->bufY);
        VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CB, pSrcFrame->bufCb);
        VpuWriteReg(CMD_ENC_PIC_SRC_ADDR_CR, pSrcFrame->bufCr);
        VpuWriteReg(CMD_ENC_PIC_SRC_STRIDE, handle->initialInfo.stride);

        VpuWriteReg(CMD_ENC_PIC_OPTION,
            (pEncParam->enReportSliceInfo<<5) | (pEncParam->enReportMVInfo<<4) |(pEncParam->enReportMBInfo<<3) |
            (pEncParam->forceIPicture << 1 & 0x2) );
    }

    if( pEncInfo->dynamicAllocEnable == 1 ) {
        VpuWriteReg( CMD_ENC_PIC_BB_START, pEncParam->picStreamBufferAddr );
        VpuWriteReg( CMD_ENC_PIC_BB_SIZE, pEncParam->picStreamBufferSize / 1024 ); // size in KB
		VpuWriteReg( pEncInfo->streamRdPtrRegAddr, pEncParam->picStreamBufferAddr); 
    }

    if(pEncParam->enReportMBInfo || pEncParam->enReportMVInfo || pEncParam->enReportSliceInfo) {
        VpuWriteReg(CMD_ENC_PIC_PARA_BASE_ADDR, pEncParam->picParaBaseAddr);
        
        if(pEncParam->enReportMBInfo)
            Vpu_EncSetHostParaAddr(pEncParam->picParaBaseAddr       , pEncParam->picMbInfoAddr);
        if(pEncParam->enReportMVInfo)
            Vpu_EncSetHostParaAddr(pEncParam->picParaBaseAddr + 8   , pEncParam->picMvInfoAddr);
        if(pEncParam->enReportSliceInfo)
            Vpu_EncSetHostParaAddr(pEncParam->picParaBaseAddr + 16  , pEncParam->picSliceInfoAddr);
    }   

    val = (
        pEncInfo->subFrameSyncConfig.subFrameSyncOn << 15 |
        pEncInfo->subFrameSyncConfig.sourceBufNumber << 8 |      // Source buffer number - 1
        pEncInfo->subFrameSyncConfig.sourceBufIndexBase << 0);   // Base index of PRP source index, Start index of PRP source
    VpuWriteReg(CMD_ENC_PIC_SUB_FRAME_SYNC, val);

    EncBitIssueCommand(handle, inst_index, OpenParam->codecMode, PIC_RUN);

    return RETCODE_SUCCESS;
}

	
CodecRetCode VPU_isr_enc_seq_init(EncInstCtl *handle)
{
	EncInitialInfo * Initinfo = &(handle->initialInfo);
	EncInfo *info = &(handle->pEncInfo);
	
    if (VpuReadReg(RET_ENC_SEQ_SUCCESS) & (1<<31))
        return RETCODE_MEMORY_ACCESS_VIOLATION;

    if (VpuReadReg(RET_ENC_SEQ_SUCCESS) == 0) {
        return RETCODE_FAILURE;
    }
    if (handle->EncSeqOpenParam.codecMode == MJPG_ENC)
        Initinfo->minFrameBufferCount = 0;	// 0 + 1
    else
        Initinfo->minFrameBufferCount = 2; // 2 + 3   reconstructed frame + reference frame

    info->initialInfoObtained = 1;

    return RETCODE_SUCCESS;
}

CodecRetCode VPU_isr_enc_reg_framebuf(EncInstCtl *handle )
{
    if (VpuReadReg(RET_SET_FRAME_SUCCESS) & (1<<31))
        return RETCODE_MEMORY_ACCESS_VIOLATION;  

    return RETCODE_SUCCESS;

}

CodecRetCode VPU_isr_enc_pic_run(EncInstCtl *handle)
{
	EncOutputInfo *pEncOutInfo=&(handle->EncOutInfo);
    EncInfo * pEncInfo= &(handle->pEncInfo);
//    CodecRetCode ret;
    PhysicalAddress rdPtr;
    PhysicalAddress wrPtr;

	if (handle->EncSeqOpenParam.codecMode == MJPG_ENC) {
		pEncOutInfo->bitstreamBuffer = pEncInfo->streamBufStartAddr;
		pEncOutInfo->bitstreamSize = pEncInfo->streamBufStartAddr - VpuReadReg(MJPEG_BBC_WR_PTR_REG);
		handle->jpgInfo.frameIdx++;
		pEncOutInfo->picType = 0;
		pEncOutInfo->numOfSlices = 0;
		return RETCODE_SUCCESS;
	}
    if (VpuReadReg(RET_ENC_PIC_SUCCESS) & (1<<31))
        return RETCODE_MEMORY_ACCESS_VIOLATION;

    pEncOutInfo->picType = VpuReadReg(RET_ENC_PIC_TYPE);

    if( pEncInfo->ringBufferEnable == 0 ) {     
        if( pEncInfo->dynamicAllocEnable == 1 ) {
            rdPtr = VpuReadReg( CMD_ENC_PIC_BB_START );
            wrPtr = VpuReadReg( pEncInfo->streamWrPtrRegAddr);
            pEncOutInfo->bitstreamBuffer = rdPtr;
            pEncOutInfo->bitstreamSize = wrPtr - rdPtr;
        }
        else {
            rdPtr = pEncInfo->streamBufStartAddr;
            wrPtr = VpuReadReg(pEncInfo->streamWrPtrRegAddr);
            pEncOutInfo->bitstreamBuffer = rdPtr;
            pEncOutInfo->bitstreamSize = wrPtr - rdPtr;
        }   
    }
    pEncOutInfo->numOfSlices = VpuReadReg(RET_ENC_PIC_SLICE_NUM);
    pEncOutInfo->bitstreamWrapAround = VpuReadReg(RET_ENC_PIC_FLAG);

 

    return RETCODE_SUCCESS;

}

