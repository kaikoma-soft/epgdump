#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "ts_ctl.h"
#include "xmldata.c"

#include "nit.h"


int parseNIThead(unsigned char *data, NIThead *h) {
	int boff = 0;

	memset(h, 0, sizeof(NIThead));

	h->table_id = getBit(data, &boff, 8);
	h->section_syntax_indicator = getBit(data, &boff, 1);
	h->reserved1 = getBit(data, &boff, 3);
	h->section_length = getBit(data, &boff, 12);
	h->network_id = getBit(data, &boff, 16);
	h->reserved2 = getBit(data, &boff, 2);
	h->version_number = getBit(data, &boff, 5);
	h->current_next_indicator = getBit(data, &boff, 1);
	h->section_number = getBit(data, &boff, 8);
	h->last_section_number = getBit(data, &boff, 8);
	h->reserved_future_use3 = getBit(data, &boff, 4);
	h->network_descriptors_length = getBit(data, &boff, 12);
	memset(h->network_descriptor,0,sizeof(h->network_descriptor));
	getStr(h->network_descriptor,data,&boff,h->network_descriptors_length);
	h->reserved_future_use2 = getBit(data, &boff, 4);
	h->transport_stream_loop_length = getBit(data, &boff, 12);
#ifdef DEBUG
	printf("NIT %x %x [%s][%d] loop[%d]\n",h->table_id,h->network_id,h->network_descriptor,h->network_descriptors_length,h->transport_stream_loop_length);
#endif
	return boff/8;
}

int parseNITbody(unsigned char *data, NITbody *b) {
	int boff = 0;

	memset(b, 0, sizeof(NITbody));

	b->transport_stream_id = getBit(data, &boff, 16);
	b->original_network_id = getBit(data, &boff, 16);
	b->reserved_future_use = getBit(data, &boff, 4);
	b->transport_descriptors_loop_length = getBit(data, &boff, 12);
#ifdef DEBUG
	printf("NITB %x %x\n",b->transport_stream_id ,b->original_network_id);
#endif

	return boff/8;
}

int parseServiceListDescriptor(unsigned char *data,ServiceList *sl)
{
	int boff = 0;
	int i;
	int length,tag;

	memset(sl,0,sizeof(ServiceList));

	tag = getBit(data,&boff,8);
	length = getBit(data,&boff,8);
	sl->length = length;

	for(i=0;i<length;) {
		sl->sld[i].service_id = getBit(data, &boff, 16);
		sl->sld[i].service_type = getBit(data, &boff, 8);
#ifdef DEBUG
		printf("id[%d] 0x[%x]\n",sl->sld[i].service_id,sl->sld[i].service_type);
#endif
		i+=3;
	}
	
	//printf("\n");
	return boff/8;
}

int parseSatelliteDeliverySystemDescriptor(unsigned char *data, SatelliteDeliverySystemDescriptor *sdsd)
{
	int boff = 0;
	int length,tag,i;

	tag = getBit(data,&boff,8);
	length = getBit(data,&boff,8);

	sdsd->frequency = 0;
	// 4bit BCDx8 xx.xxxxxxGHz
	for(i=0;i<7;i++) {
		sdsd->frequency += getBit(data,&boff,4);
		sdsd->frequency *= 10;
	}
	sdsd->frequency += getBit(data,&boff,4);

	sdsd->orbital_position = 0;
	// 4bit BCDx4 xxx.x
	for(i=0;i<3;i++) {
		sdsd->orbital_position += getBit(data,&boff,4);
		sdsd->orbital_position *= 10;
	}
	sdsd->orbital_position += getBit(data,&boff,4);

	sdsd->west_east_flag = getBit(data,&boff,1);
	sdsd->polarisation = getBit(data,&boff,2);
	sdsd->modulation = getBit(data,&boff,5);

	sdsd->symbol_rate = 0;
	// 4bit BCDx6 xxx.xxxx
	for(i=0;i<5;i++) {
		sdsd->symbol_rate += getBit(data,&boff,4);
		sdsd->symbol_rate *= 10;
	}
	sdsd->symbol_rate += getBit(data,&boff,4);

	sdsd->FEC_inner = getBit(data,&boff,4);

	return boff/8;
}

void	setsdtinfo(SVT_CONTROL *top, NITbody *nitb,SatelliteDeliverySystemDescriptor *sdsd,ServiceList *sl)
{
	        SVT_CONTROL     *cur = top ;
            int i;
	        while(cur != NULL){
			for(i=0;i<sl->length;i++)
				if (sl->sld[i].service_id == cur->event_id)
					cur->frequency = sdsd->frequency;
/*
			if ((cur->transport_stream_id == nitb->transport_stream_id) &&
			   (cur->original_network_id == nitb->original_network_id))
			{
				cur->frequency = sdsd->frequency;
			}
*/
                	cur = cur->next ;
	        }
		return;
}


void dumpNIT(unsigned char *ptr, SVT_CONTROL *top)
{

	int len = 0;
	int loop_len = 0;
	int loop_blen = 0;
	int boff;
	int i;
	NIThead nith;
	NITbody nitb;
	SatelliteDeliverySystemDescriptor sdsd;
	ServiceList sl;

	len = parseNIThead(ptr,&nith);

        ptr += len;
        loop_len = nith.section_length - (len - 3 + 4); // 3�϶��̥إå�Ĺ 4��CRC
        while(loop_len > 0) {
		len = parseNITbody(ptr, &nitb);
                ptr += len;
                loop_len -= len;

                loop_blen = nitb.transport_descriptors_loop_length;
                loop_len -= loop_blen;
                while(loop_blen > 0) {
			int sboff;
			unsigned char desctag;
			unsigned char desclen;
			sboff = 0;
			desctag = getBit(ptr,&sboff,8);
			desclen = getBit(ptr,&sboff,8);
#ifdef DEBUG
	printf("Desc %x len %d\n",desctag,desclen);
#endif
			len=0;
			switch(desctag) {
				case 0x41:
					len = parseServiceListDescriptor(ptr,&sl);
					break;
				case 0x43:
					len = parseSatelliteDeliverySystemDescriptor(ptr,&sdsd);
					setsdtinfo(top,&nitb,&sdsd,&sl);
#ifdef DEBUG
	printf("SatteliteInfo %x %x Freq %.5f GHz %s\n",nitb.transport_stream_id,nitb.original_network_id,(float)sdsd.frequency/100000.0,getTP(sdsd.frequency));
	printf(" orbital position %.1f\n",(float)sdsd.orbital_position/10.0); 
	printf("         westeast %s\n",sdsd.west_east_flag?"west":"east"); 
	printf("     polarization %s\n",getPolarization((int)sdsd.polarisation)); 
	printf("      symbol_rate %.4f\n",(float)sdsd.symbol_rate/1000.0); 
#endif
					break;
				default:
					len = parseOTHERdesc(ptr);
					break;

			}
                        ptr += len;
                        loop_blen -= len;
		}

	}

	return;
}
