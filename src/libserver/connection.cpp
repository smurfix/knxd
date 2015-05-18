/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "connection.h"

A_ANY::A_ANY (Layer3 * l3, Trace * tr, ClientConnection * cc, const char *type)
{
  TRACEPRINTF (t, 7, this, "Open%s", type);
  t = tr;
  layer3 = l3;
  con = cc;
  type_ = type;
  c = 0;
}

A_ANY::~A_ANY ()
{
  TRACEPRINTF (t, 7, this, "Close%s", type_);
  Stop ();
  if (c)
    delete c;
}

A_Broadcast::A_Broadcast (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"Broadcast")
{
  writeonly = (con->buf[4] != 0);
}

A_Group::A_Group (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"Group")
{
  groupaddr = (con->buf[2] << 8) | (con->buf[3];
  writeonly = (con->buf[4] != 0);
}

A_TPDU::A_TPDU (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"TPDU")
{
  srcaddr = (con->buf[2] << 8) | (con->buf[3];
}

A_Individual::A_Individual (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"Individual")
{
  dest = (con->buf[2] << 8) | (con->buf[3];
  write_only = (con->buf[4] != 0);
}

A_Connection::A_Connection (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"Connection")
{
  dest = (con->buf[2] << 8) | (con->buf[3];
}

A_GroupSocket::A_GroupSocket (Layer3 * l3, Trace * tr, ClientConnection * cc)
	: A_ANY(l3,tr,cc,"GroupSocket")
{
}

void
A_ANY::Do (pth_event_t stop)
{
  if (!c)
    {
      con->sendreject (stop, EIB_PROCESSING_ERROR);
      return;
    }
  if (con->sendmessage (2, con->buf, stop) == -1)
    return;
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      if (con->readmessage (stop) == -1)
	break;
#if 0
      if (EIBTYPE (con->buf) == EIB_RESET_CONNECTION)
	break;
#endif
      if (EIBTYPE (con->buf) != EIB_APDU_PACKET)
	break;
      process_buffer();
    }
}

void
A_Broadcast::process_buffer ()
{
  if (con->size < 2)
    return;
  t->TracePacket (7, this, "Send", con->size - 2, con->buf + 2);

  T_DATA_XXX_REQ_PDU t;
  t.data = CArray (con->buf + 2, con->size - 2);
  String s = t.Decode (this->t);
  TRACEPRINTF (this->t, 4, this, "Send %s %s", type_, s ());

  L_Data_PDU *l = new L_Data_PDU (FakeL2);
  l->source = 0;
  l->dest = 0;
  l->AddrType = GroupAddress;
  l->data = t.ToPacket ();
  layer3->send_L_Data (l);
}

void
A_Group::process_buffer ()
{
  if (con->size < 2)
    return;
  t->TracePacket (7, this, "Send", con->size - 2, con->buf + 2);

  T_DATA_XXX_REQ_PDU t;
  t.data = CArray (con->buf + 2, con->size - 2);
  String s = t.Decode (this->t);
  TRACEPRINTF (this->t, 4, this, "Send %s %s", type_, s ());

  L_Data_PDU *l = new L_Data_PDU (FakeL2);
  l->source = 0;
  l->dest = groupaddr;
  l->AddrType = GroupAddress;
  l->data = t.ToPacket ();
  layer3->send_L_Data (l);
}

void
A_TPDU::process_buffer ()
{
  if (con->size < 4)
    return;
  t->TracePacket (7, this, "Send", con->size - 4, con->buf + 4);
  L_Data_PDU *l = new L_Data_PDU;
  l->source = src;
  l->dest = (con->buf[2] << 8) | (con->buf[3]);
  l->AddrType = IndividualAddress;
  l->data = CArray (con->buf + 4, con->size - 4);
  layer3->send_L_Data (l);
}

void
A_GroupSocket::process_buffer ()
{
  if (con->size >= 4)
    return;
  t->TracePacket (7, this, "Send", con->size - 4, con->buf + 4);
  T_DATA_XXX_REQ_PDU t;
  t.data = CArray (con->buf + 4, con->size - 4);
  String s = t.Decode ();
  TRACEPRINTF (this->t, 4, this, "Send %s %s", type_, s ());

  L_Data_PDU *l = new L_Data_PDU;
  l->source = 0;
  l->dest = (con->buf[2] << 8) | (con->buf[3]);
  l->AddrType = GroupAddress;
  l->data = t.ToPacket ();
  layer3->send_L_Data (l);
}

void
A_Individual::process_buffer ()
{
  if (con->size < 2)
    return;
  t->TracePacket (7, this, "Send", con->size - 2, con->buf + 2);

  T_DATA_XXX_REQ_PDU t;
  t.data = CArray (con->buf + 2, con->size - 2);
  String s = t.Decode (this->t);
  TRACEPRINTF (this->t, 4, this, "Send %s %s", type_, s ());

  L_Data_PDU *l = new L_Data_PDU (FakeL2);
  l->source = 0;
  l->dest = dest;
  l->AddrType = IndividualAddress;
  l->data = t.ToPacket ();
  layer3->send_L_Data (l);
}

void
A_Connection::process_buffer ()
{
  if (con->size < 2)
    return;
  t->TracePacket (7, this, "Send", con->size - 2, con->buf + 2);

  in.put (CArray (con->buf + 2, con->size - 2));
  pth_sem_inc (&insem, 1);
}



void
A_ANY::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      Any_Comm *e = c->Get (stop);
      if (e)
	{
	  CArray res;
	  res.resize (4 + e->data ());
	  EIBSETTYPE (res, EIB_APDU_PACKET);
	  res[2] = (e->src >> 8) & 0xff;
	  res[3] = (e->src) & 0xff;
	  res.setpart (e->data.array (), 4, e->data ());
	  t->TracePacket (7, this, "Recv", e->data);
	  con->sendmessage (res (), res.array (), stop);
	  delete e;
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

void
A_TPDU::Run (pth_sem_t * stop1)
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      TpduComm *e = c->Get (stop);
      if (e)
	{
	  CArray res;
	  res.resize (4 + e->data ());
	  EIBSETTYPE (res, EIB_APDU_PACKET);
	  res[2] = (e->addr >> 8) & 0xff;
	  res[3] = (e->addr) & 0xff;
	  res.setpart (e->data.array (), 4, e->data ());
	  t->TracePacket (7, this, "Recv", e->data);
	  con->sendmessage (res (), res.array (), stop);
	  delete e;
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

void
A_Individual::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      CArray *e = c->Get (stop);
      if (e)
	{
	  CArray res;
	  res.resize (2 + e->len ());
	  EIBSETTYPE (res, EIB_APDU_PACKET);
	  res.setpart (e->array (), 2, e->len ());
	  t->TracePacket (7, this, "Recv", *e);
	  con->sendmessage (res (), res.array (), stop);
	  delete e;
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

void
A_Connection::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      CArray *e = c->Get (stop);
      if (e)
	{
	  CArray res;
	  res.resize (2 + e->len ());
	  EIBSETTYPE (res, EIB_APDU_PACKET);
	  res.setpart (e->array (), 2, e->len ());
	  t->TracePacket (7, this, "Recv", *e);
	  con->sendmessage (res (), res.array (), stop);
	  delete e;
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}

void
A_GroupSocket::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      GroupAPDU *e = c->Get (stop);
      if (e)
	{
	  CArray res;
	  res.resize (6 + e->data ());
	  EIBSETTYPE (res, EIB_GROUP_PACKET);
	  res[2] = (e->src >> 8) & 0xff;
	  res[3] = (e->src) & 0xff;
	  res[4] = (e->dst >> 8) & 0xff;
	  res[5] = (e->dst) & 0xff;
	  res.setpart (e->data.array (), 6, e->data ());
	  t->TracePacket (7, this, "Recv", e->data);
	  con->sendmessage (res (), res.array (), stop);
	  delete e;
	}
    }
  pth_event_free (stop, PTH_FREE_THIS);
}
