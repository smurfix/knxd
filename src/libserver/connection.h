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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "layer4.h"
#include "client.h"

/** implements client interface to a broadcast connection */
class A_ANY:private Thread
{
  Layer3 *layer3;
  Trace *t;
  ClientConnection *con;
  T_ANY *c;
  const char type_;

  void Run (pth_sem_t * stop);
public:
  A_ANY (Layer3 * l3, Trace * tr, ClientConnection * cc, const char *type);
  ~A_ANY ();

  /** start processing */
  void Do (pth_event_t stop);
  /** detail processing. Exit the loop when True */
  virtual void process_buffer ();
  virtual T_ANY *init_T () = 0;
};

class A_Broadcast:private Thread
{
public:
  A_Broadcast (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_Broadcast ();
  bool writeonly;

  /** start processing */
  void Do (pth_event_t stop);
};

/** implements client interface to a group connection */
class A_Group:private Thread
{
  eibaddr_t groupaddr;
  bool writeonly;

  void Run (pth_sem_t * stop);
public:
  A_Group (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_Group ();

  /** start processing */
  void Do (pth_event_t stop);
};

/** implements client interface to a raw connection */
class A_TPDU:private Thread
{
  eibaddr_t srcaddr;

  void Run (pth_sem_t * stop);
public:
  A_TPDU (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_TPDU ();

  /** start processing */
  void Do (pth_event_t stop);
};

/** implements client interface to a T_Indivdual connection */
class A_Individual:private Thread
{
  eibaddr_t dest;
  bool write_only;

  void Run (pth_sem_t * stop);
public:
  A_Individual (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_Individual ();

  /** start processing */
  void Do (pth_event_t stop);
};

/** implements client interface to a T_Connection connection */
class A_Connection:private Thread
{
  eibaddr_t dest;

  void Run (pth_sem_t * stop);
public:
  A_Connection (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_Connection ();

  /** start processing */
  void Do (pth_event_t stop);
};

/** implements client interface to a group socket */
class A_GroupSocket:private Thread
{
  Layer3 *layer3;
  Trace *t;
  ClientConnection *con;

  void Run (pth_sem_t * stop);
public:
  A_GroupSocket (Layer3 * l3, Trace * tr, ClientConnection * cc);
  ~A_GroupSocket ();

  /** start processing */
  void Do (pth_event_t stop);
};

#endif
