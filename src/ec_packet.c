/*
    ettercap -- packet object handling

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

    $Id: ec_packet.c,v 1.27 2003/12/09 22:32:54 alor Exp $
*/

#include <ec.h>
#include <ec_packet.h>
#include <ec_inet.h>
#include <ec_ui.h>
#include <ec_format.h>

/* protos... */

inline int packet_create_object(struct packet_object *po, u_char *buf, size_t len);
inline int packet_destroy_object(struct packet_object *po);
int packet_disp_data(struct packet_object *po, u_char *buf, size_t len);
struct packet_object * packet_dup(struct packet_object *po);

/* --------------------------- */

/*
 * associate the buffer to the packet object
 */

inline int packet_create_object(struct packet_object *po, u_char *buf, size_t len)
{
   /* clear the memory */
   memset(po, 0, sizeof(struct packet_object));
   
   /* set the buffer and the len of the received packet */
   po->packet = buf;
   po->len = len;
   
   return (0);
}

/*
 * allocate the buffer for disp data
 *
 * disp data is usefull when the protocol is
 * encrypted and we want to forward the packet as is
 * but display the decrypted data.
 * decoders should decrypt data from po->DATA.data to po->DATA.disp_data
 */

int packet_disp_data(struct packet_object *po, u_char *buf, size_t len)
{
   /* disp_data is always null terminated */
   if (len + 1)
      SAFE_CALLOC(po->DATA.disp_data, len + 1, sizeof(u_char));
   else
      ERROR_MSG("packet_disp_data() negative len");

   po->DATA.disp_len = len;
   memcpy(po->DATA.disp_data, buf, len);

   return len;
}

/*
 * free the packet object memory
 */

inline int packet_destroy_object(struct packet_object *po)
{
   
   /* 
    * the packet is a duplicate
    * we have to free even the packet buffer.
    * alse free data directed to top_half
    */
   if (po->flags & PO_DUP) {
     
      SAFE_FREE(po->packet);
      
      /* 
       * free the dissector info 
       * during the duplication, the pointers where
       * passed to the dup, so we have to free them
       * here.
       */
      SAFE_FREE(po->DISSECTOR.user);
      SAFE_FREE(po->DISSECTOR.pass);
      SAFE_FREE(po->DISSECTOR.info);
   }
      
   /* 
    * free the disp_data pointer
    * it was malloced by tcp or udp decoder
    */
   SAFE_FREE(po->DATA.disp_data);
   
   return 0;
}


/*
 * duplicate a po and return
 * the new allocated one
 */
struct packet_object * packet_dup(struct packet_object *po)
{
   struct packet_object *dup_po;

   SAFE_CALLOC(dup_po, 1, sizeof(struct packet_object));

   /* 
    * copy the po over the dup_po 
    * but this is not sufficient, we have to adjust all 
    * the pointer to the po->packet.
    * so allocate a new packet, then recalculate the
    * pointers
    */
   memcpy(dup_po, po, sizeof(struct packet_object));

   /* copy only if the buffer exists */
   if (po->DATA.disp_data != NULL) {  
      /* duplicate the po dispdata */
      SAFE_CALLOC(dup_po->DATA.disp_data, po->DATA.disp_len, sizeof(u_char));
  
      /* copy the buffer */
      memcpy(dup_po->DATA.disp_data, po->DATA.disp_data, po->DATA.disp_len);
   }
   
   /* copy only if the buffer exists */
   if (po->packet != NULL) {  
      /* duplicate the po buffer */
      SAFE_CALLOC(dup_po->packet, po->len, sizeof(u_char));
  
      /* copy the buffer */
      memcpy(dup_po->packet, po->packet, po->len);
   }

   /* 
    * adjust all the pointers as the difference
    * between the old buffer and the pointer
    */
   dup_po->L2.header = dup_po->packet + (po->L2.header - po->packet);
   
   dup_po->L3.header = dup_po->packet + (po->L3.header - po->packet);
   dup_po->L3.options = dup_po->packet + (po->L3.options - po->packet);
   
   dup_po->L4.header = dup_po->packet + (po->L4.header - po->packet);
   dup_po->L4.options = dup_po->packet + (po->L4.options - po->packet);
   
   dup_po->DATA.data = dup_po->packet + (po->DATA.data - po->packet);

   dup_po->fwd_packet = dup_po->packet + (po->fwd_packet - po->packet);

   /* this packet is a duplicate */
   dup_po->flags |= PO_DUP;

   return dup_po;
}

   
/* EOF */

// vim:ts=3:expandtab
