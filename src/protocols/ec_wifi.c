/*
    ettercap -- 802.11b (wifi) decoder module

    Copyright (C) ALoR & NaGA

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    $Id: ec_wifi.c,v 1.9 2003/12/09 22:32:54 alor Exp $
*/

#include <ec.h>
#include <ec_decode.h>

/* globals */

struct wifi_header {
   u_int16  type;
      #define WIFI_DATA_ENTERING 0x0802
      #define WIFI_DATA_EXITING  0x0801
      #define WIFI_DATA_WEP      0x0842
      #define WIFI_BACON         0x0800
   u_int16  duration;
   u_int8   dha[ETH_ADDR_LEN];
   u_int8   sha[ETH_ADDR_LEN];
   u_int8   bssid[ETH_ADDR_LEN];
   u_int16  seq;
   u_int8   llc_dsap;
   u_int8   llc_ssap;
   u_int8   llc_control;
   u_int8   llc_org_code[3];
   u_int16  proto;
};

/* encapsulated ethernet */
u_int8 WIFI_ORG_CODE[3] = {0x00, 0x00, 0x00};

/* protos */

FUNC_DECODER(decode_wifi);
void wifi_init(void);

/*******************************************/

/*
 * this function is the initializer.
 * it adds the entry in the table of registered decoder
 */

void __init wifi_init(void)
{
   add_decoder(LINK_LAYER, IL_TYPE_WIFI, decode_wifi);
}


FUNC_DECODER(decode_wifi)
{
   struct wifi_header *wifi;
   FUNC_DECODER_PTR(next_decoder) = NULL;

   DECODED_LEN = sizeof(struct wifi_header);
      
   wifi = (struct wifi_header *)DECODE_DATA;
   
   /* org_code != encapsulated ethernet not yet supported */
   if (memcmp(wifi->llc_org_code, WIFI_ORG_CODE, 3))
      NOT_IMPLEMENTED();
      
   /* XXX - where is the ESSID ? check with ethereal */
  
   /* BUCKET->L2->ESSID = ??? */
   
   /* we are only interested in "data" type */
   if (ntohs(wifi->type) == WIFI_DATA_ENTERING || ntohs(wifi->type) == WIFI_DATA_EXITING) {
      next_decoder = get_decoder(NET_LAYER, ntohs(wifi->proto));
   } else if (ntohs(wifi->type) == WIFI_BACON) {
      /* BACON (or unsupported message) */
      return NULL;
   }
   
   /* fill the bucket with sensitive data */
   PACKET->L2.header = (u_char *)DECODE_DATA;
   PACKET->L2.proto = IL_TYPE_WIFI;
   PACKET->L2.len = DECODED_LEN;
   
   memcpy(PACKET->L2.src, wifi->sha, ETH_ADDR_LEN);
   memcpy(PACKET->L2.dst, wifi->dha, ETH_ADDR_LEN);

   /* HOOK POINT: HOOK_PACKET_WIFI */
   hook_point(HOOK_PACKET_WIFI, po);
   
   /* leave the control to the next decoder */
   EXECUTE_DECODER(next_decoder);
  
   /* no modification to wifi header should be done */
   
   return NULL;
}

/* EOF */

// vim:ts=3:expandtab

