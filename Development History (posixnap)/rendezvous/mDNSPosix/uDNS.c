/*
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@

    Change History (most recent first):

$Log: not supported by cvs2svn $
Revision 1.11  2004/01/30 02:12:30  ksekar
Changed uDNS_ReceiveMsg() to correctly return void.

Revision 1.10  2004/01/29 02:59:17  ksekar
Unicast DNS: Changed from a resource record oriented question/response
matching to packet based matching.  New callback architecture allows
collections of records in a response to be processed differently
depending on the nature of the request, and allows the same structure
to be used for internal and client-driven queries with different processing needs.

Revision 1.9  2004/01/28 20:20:45  ksekar
Unified ActiveQueries and ActiveInternalQueries lists, using a flag to
demux them.  Check-in includes work-in-progress code, #ifdef'd out.

Revision 1.8  2004/01/28 02:30:07  ksekar
Added default Search Domains to unicast browsing, controlled via
Networking sharing prefs pane.  Stopped sending unicast messages on
every interface.  Fixed unicast resolving via mach-port API.

Revision 1.7  2004/01/27 20:15:22  cheshire
<rdar://problem/3541288>: Time to prune obsolete code for listening on port 53

Revision 1.6  2004/01/24 23:47:17  cheshire
Use mDNSOpaque16fromIntVal() instead of shifting and masking

Revision 1.5  2004/01/24 04:59:15  cheshire
Fixes so that Posix/Linux, OS9, Windows, and VxWorks targets build again

Revision 1.4  2004/01/24 04:19:26  cheshire
Restore overwritten checkin 1.2

Revision 1.3  2004/01/23 23:23:15  ksekar
Added TCP support for truncated unicast messages.

Revision 1.2  2004/01/22 03:48:41  cheshire
Make sure uDNS client doesn't accidentally use query ID zero

Revision 1.1  2003/12/13 03:05:27  ksekar
Bug #: <rdar://problem/3192548>: DynDNS: Unicast query of service records

 */

#include "uDNS.h"


#ifndef NULL
#define NULL mDNSNULL
#endif // NULL

//#define HARDCODE_NS
#ifdef HARDCODE_NS

static mDNSAddr   hcNS = { mDNSAddrType_IPv4, { { { 17,   202,   41, 107 } } } };

#endif // HARDCODE_NS

// Private Function Prototypes
// Note:  In general, functions are ordered such that they do not require forward declarations.
// However, prototypes are used where cyclic call graphs exist (e.g. foo calls bar, and bar calls
// foo), or when they aid in the readability of code (e.g. state machine code that is easier read
// top-to-bottom.

mDNSlocal void hndlTruncatedAnswer(DNSQuestion *question,  const mDNSAddr *src, mDNS *m);

#define ustrcpy(d,s) mDNSPlatformStrCopy(s,d)       // use strcpy(2) param ordering
#define umalloc(x) mDNSPlatformMemAllocate(x)       // short hands for common routines
#define ufree(x) mDNSPlatformMemFree(x)
#define ubzero(x,y) mDNSPlatformMemZero(x,y)

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Name Server List Management
#endif

mDNSexport void mDNS_RegisterDNS(mDNS *const m, mDNSv4Addr *const dnsAddr)
    {
    //!!!KRS do this dynamically!
    uDNS_data_t *u = &m->uDNS_data;
    int i;

	if (!dnsAddr->NotAnInteger)
        {
        LogMsg("ERROR: attempt to register DNS with IP address 0");
        return;
        }

    for (i = 0; i < 32; i++)
        {
        if (!u->Servers[i].ip.v4.NotAnInteger)
            {
            u->Servers[i].ip.v4.NotAnInteger = dnsAddr->NotAnInteger;
			u->Servers[i].type = mDNSAddrType_IPv4;
            return;
            }
        if (u->Servers[i].ip.v4.NotAnInteger == dnsAddr->NotAnInteger)
            {
            LogMsg("ERROR: mDNS_RegisterDNS - DNS already registered");
            return;
            }
        }
    if (i == 32) {  LogMsg("ERROR: mDNS_RegisterDNS - too many registered servers");  }

    }

mDNSexport void mDNS_DeregisterDNS(mDNS *const m, mDNSv4Addr *const dnsAddr)
    {
    uDNS_data_t *u = &m->uDNS_data;
    int i;

    if (!dnsAddr->NotAnInteger)
        {
        LogMsg("ERROR: attempt to deregister DNS with IP address 0");
        return;
        }

    for (i = 0; i < 32; i++)
        {

        if (u->Servers[i].ip.v4.NotAnInteger == dnsAddr->NotAnInteger)
            {
            u->Servers[i].ip.v4.NotAnInteger = 0;
            return;
            }
        }
    if (i == 32) {  LogMsg("ERROR: mDNS_DeregisterDNS - no such DNS registered");  }
    }

mDNSexport void mDNS_DeregisterDNSList(mDNS *const m)
    {
    ubzero(m->uDNS_data.Servers, 32 * sizeof(mDNSAddr));
    }



// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Incoming Message Processing
#endif

// simpleResponseHndlr
// This routine examines each answer in a packet.  If a resource record in the answer section answers
// the question, the question callback is invoked.  No context-specific procedures are followed (e.g.
// CNAME records are not canonicalized), and no sections other than the answer section are examined.

mDNSlocal void simpleResponseHndlr(mDNS * const m, DNSMessage *msg, const  mDNSu8 *end, DNSQuestion *question,
								   void *internalContext)
	{
	const mDNSu8 *ptr;
	char errbuf[MAX_ESCAPED_DOMAIN_NAME];
	int i;
	LargeCacheRecord lcr;
	mDNSBool answered = mDNSfalse;
	
	(void)internalContext;  //unused;	

	if (!msg->h.numAnswers)
		{
		LogMsg("simpleResponseHndlr: discarding response to question %s (%d) with no answers",
			   ConvertDomainNameToCString(&question->qname, errbuf), question->qtype);
		return;
		}
	ptr = LocateAnswers(msg, end);
	if (!ptr) goto pkt_error;
	for (i = 0; i < msg->h.numAnswers; i++)
		{
		ptr = GetLargeResourceRecord(m, msg, ptr, end, 0, kDNSRecordTypePacketAns, &lcr);
		if (!ptr) goto pkt_error;
		if (ResourceRecordAnswersQuestion(&lcr.r.resrec, question)) 
			{
			question->QuestionCallback(m, question, &lcr.r.resrec, 1);
			answered = mDNStrue;
			}
		}
	if (!answered) LogMsg("simpleResponseHndlr: received response to question %s (%d) containing"
						  "no records that answered the question", ConvertDomainNameToCString(&question->qname, errbuf),
						  question->qtype);
	return;

	pkt_error:
	LogMsg("ERROR: simpleResponseHndlr - received malformed response to query for %s (%d)",
		   ConvertDomainNameToCString(&question->qname, errbuf), question->qtype);
	}

mDNSexport void uDNS_ReceiveMsg(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
	const mDNSAddr *const srcaddr, const mDNSIPPort srcport, const mDNSAddr *const dstaddr,
	const mDNSIPPort dstport, const mDNSInterfaceID InterfaceID, mDNSu8 ttl)
	{
	DNSQuestion *qptr;

	// unused
	(void)srcaddr;
	(void)srcport;
	(void)dstaddr;
	(void)dstport;
	(void)ttl;
	(void)InterfaceID;
	
	if (msg->h.flags.NotAnInteger & kDNSFlag0_QR_Response)
		{
		for (qptr = m->uDNS_data.ActiveQueries; qptr; qptr = qptr->next)
			{
			if (qptr->uDNS_info.id.NotAnInteger == msg->h.id.NotAnInteger)
				{
				if (msg->h.flags.b[0] & kDNSFlag0_TC)  hndlTruncatedAnswer(qptr, srcaddr, m);
				else qptr->uDNS_info.responseCallback(m, msg, end, qptr, qptr->uDNS_info.context);
				return;
				}
			}
		LogMsg("Received unexpected response: ID %d matches no active queries", mDNSVal16(msg->h.id));		
		}
	LogMsg("Received unexpected unicast message");
	}

		
mDNSlocal void receiveMsg(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
	const mDNSInterfaceID InterfaceID)
	{
	mDNSAddr *sa = NULL, *da = NULL;
	mDNSIPPort sp, dp;
	mDNSu8 ttl = 0;

	sp.NotAnInteger = 0;
	dp.NotAnInteger = 0;
	uDNS_ReceiveMsg(m, msg, end, sa, sp, da, dp, InterfaceID, ttl);
	}

//!!!KRS this should go away (don't just pick one randomly!)
mDNSlocal mDNSAddr *getInitializedDNS(uDNS_data_t *u)
    {
    int i;
    for (i = 0; i < 32; i++)
		if (u->Servers[i].ip.v4.NotAnInteger) return &u->Servers[i];

	LogMsg("ERROR: getInitializedDNS - no initialized servers.");
	return NULL;
    }

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Query Routines
#endif

mDNSexport mDNSBool IsActiveUnicastQuery(DNSQuestion *const question, uDNS_data_t *u)
    {
	//!!!KRS We should remove the list check at some point...
	DNSQuestion *qptr;
	mDNSBool inList = mDNSfalse;

	for (qptr = u->ActiveQueries; qptr; qptr = qptr->next)
		if (qptr == question) { inList = mDNStrue;  break; }
	
	if (question->uDNS_info.id.NotAnInteger && inList) return mDNStrue;
	if (!question->uDNS_info.id.NotAnInteger && !inList) return mDNSfalse;

	LogMsg("ERROR: IsActiveUnicastQuery - conflicting results:\n"
		   "%s question id, %s ActiveQueries list",
		   question->uDNS_info.id.NotAnInteger ? "non-zero" : "zero",
		   inList ? "in" : "not in");
	return mDNSfalse;
	}
	
mDNSexport mStatus uDNS_StopQuery(mDNS *const m, DNSQuestion *const question)
	{
	uDNS_data_t *u = &m->uDNS_data;
	DNSQuestion *qptr, *prev = NULL;
	char errbuf[MAX_ESCAPED_DOMAIN_NAME];

	qptr = u->ActiveQueries;
	while (qptr)
        {
        if (qptr == question)
            {
            if (prev) prev->next = question->next;
            else u->ActiveQueries = question->next;
			return mStatus_NoError;
            }
        prev = qptr;
		qptr = qptr->next;
        }
    LogMsg("uDNS_StopQuery: no such active query (%s)", ConvertDomainNameToCString(&question->qname, errbuf));
    return mStatus_UnknownErr;
    }

mDNSlocal mStatus constructQueryMsg(DNSMessage *msg, mDNSu8 **endPtr, mDNSOpaque16 id, mDNSOpaque16 flags, DNSQuestion *const question)
	{
	ubzero(msg, sizeof(msg));
	flags.b[0] |= kDNSFlag0_RD;
    InitializeDNSMessage(&msg->h, id, flags);

    *endPtr = putQuestion(msg, msg->data, msg->data + AbsoluteMaxDNSMessageData, &question->qname, question->qtype, question->qclass);
    if (!*endPtr)
        {
        LogMsg("ERROR: Unicast query out of space in packet");
        return mStatus_UnknownErr;
        }
	return mStatus_NoError;
	}

mDNSlocal mStatus startQuery(mDNS *const m, DNSQuestion *const question, mDNSBool internal)
    {
    uDNS_data_t *u = &m->uDNS_data;
    DNSMessage msg;
    mDNSu8 *endPtr;
    mStatus err = mStatus_NoError;
    mDNSu16 idval;

    //!!!KRS we should check if the question is already in our acivequestion list
	if (!ValidateDomainName(&question->qname))
		{
		LogMsg("Attempt to start query with invalid qname %##s %s", question->qname.c, DNSTypeName(question->qtype));
		return mStatus_Invalid;
		}

	// if NextMessageID is 0 (ininitialized) or 0xffff (reserved for TCP packets) reset to 1
	if (!u->NextMessageID || u->NextMessageID == (mDNSu16)~0) u->NextMessageID = 1;
	idval = u->NextMessageID++;
	question->next = NULL;
	err = constructQueryMsg(&msg, &endPtr, mDNSOpaque16fromIntVal(idval), QueryFlags, question);
	if (err) return err;

    // store the question/id in active question list
    question->uDNS_info.id.NotAnInteger = mDNSOpaque16fromIntVal(idval).NotAnInteger;
    question->uDNS_info.timestamp = m->timenow;
	question->uDNS_info.internal = internal;
    question->qnamehash = DomainNameHashValue(&question->qname);    // to do quick domain name comparisons
	question->next = u->ActiveQueries;
	u->ActiveQueries = question;

	err = mDNSSendDNSMessage(m, &msg, endPtr, question->InterfaceID, getInitializedDNS(u), UnicastDNSPort);
	if (err) LogMsg("ERROR: mDNSSendDNSMessage - %d", err);
	
	return err;
	}

mDNSexport mStatus uDNS_StartQuery(mDNS *const m, DNSQuestion *const question)
    {
	// how the answer is processed is determined by the type of query
	switch(question->qtype)
		{
		default:
			question->uDNS_info.responseCallback = simpleResponseHndlr;
			question->uDNS_info.context = NULL;
		}  
	return startQuery(m, question, 0);
    }

#if 0 //!!!KRS incomplete
// explicitly set response handler
mDNSlocal mStatus startInternalQuery(DNSQuestion *q, mDNS *m, InternalResponseHndlr callback, void *hndlrContext)
    {    
    q->QuestionContext = hndlrContext;
    q->uDNS_info.responseCallback = callback;
	q->uDNS_info.context = hndlrContext;
    return startQuery(m, q, 1);
    }



// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Domain -> Name Server Conversion
#endif


/* findZoneNS
 *
 * asyncronously find the address of the nameserver for the enclosing zone for a given domain name,
 * i.e. the server to which update requests will be sent for a given name.  Once the address is
 * derived, it will be passed to the callback, along with a context pointer.  If the zone cannot
 * be determined or if an error occurs, an all-zeros address will be passed and a message will be
 * written to the syslog.  Steps for deriving the name are as follows:
 *
 * Query for an SOA record for the dname.  If we don't get an answer (or an SOA in the Authority
 * section), we strip the leading label from the dname and repeat, until we get an answer.
 *
 * The name of the SOA record is our enclosing zone.  The mname field in the SOA rdata is the domain
 * name of the primary NS.
 *
 * We verify that there is an NS record with this zone for a name and the mname for its rdata.
 * (!!!KRS this seems redundant, but BIND does this, and it should normally be zero-overhead since
 * the NS query will get us address records in the additionals section, which we'd otherwise have to
 * explicitly query for.)
 *
 * We then query for the address record for this nameserver (if it is not in the addionals section of
 * the NS record response.)
 */
 

// state machine types and structs
//

// state machine states
typedef enum
    {
    init,
    lookupSOA,
	foundZone,
	lookupNS,
    } ntaState;

// state machine actions
typedef enum
    {
    smContinue,
    smBreak,
    } smAction;
 
typedef struct
    {
    domainname 	origName;            // name we originally try to convert
    domainname 	*curSOA;             // name we have an outstanding SOA query for
    ntaState  	state;               // determines what we do upon receiving a packet
    mDNS	    *m;
    domainname  zone;                // left-hand-side of SOA record
    domainname  ns;                  // mname in SOA rdata, verified in confirmNS state
    DNSQuestion question;            // storage for any active question
    mDNSBool questionActive;         // if true, StopQuery() can be called on the question field
    } ntaContext;


// function prototypes (for routines that must be used as fn pointers prior to their definitions,
// and allows states to be read top-to-bottom in logical order)
mDNSlocal mStatus startNameToAddr(domainname *name, mDNS *m);
mDNSlocal void nameToAddr(mDNS *const m, DNSMessage *msg, const mDNSu8 *end, DNSQuestion *question, void *contextPtr);
mDNSlocal smAction hndlLookupSOA(DNSMessage *msg, const mDNSu8 *end, ntaContext *context);

// initialization
mDNSlocal mStatus startNameToAddr(domainname *name, mDNS *m)
    {
    ntaContext *context = umalloc(sizeof(ntaContext));
    if (!context) { LogMsg("ERROR: startNameToAddr - umalloc failed");  return mStatus_NoMemoryErr; }
	ubzero(context, sizeof(ntaContext));
    ustrcpy(context->origName.c, name->c);
    context->state = init;
    context->m = m;
    nameToAddr(m, NULL, NULL, NULL, context);
    return mStatus_NoError;
    }

 
// state machine entry routine
mDNSlocal void nameToAddr(mDNS *const m, DNSMessage *msg, const mDNSu8 *end, DNSQuestion *question, void *contextPtr)
    {
	// unused
	(void)m;
	(void)question;

	ntaContext *context = contextPtr;

    switch (context->state)
        {
        case init:
        case lookupSOA:
            if (hndlLookupSOA(msg, end, context) != smContinue) break;
		case foundZone:
//		case lookupNS:
//			if (hdnlLookupNS(msg, context) != smContinue) break;
		default: return;
			
        }
        
    }

mDNSlocal smAction hndlLookupSOA(DNSMessage *msg, const mDNSu8 *end, ntaContext *context)
    {
    char errbuf[MAX_ESCAPED_DOMAIN_NAME];
#if MDNS_DEBUGMSGS
	char errbuf2[MAX_ESCAPED_DOMAIN_NAME];
#endif
    mStatus err;
    LargeCacheRecord lcr;
	ResourceRecord *rr = &lcr.r.resrec;
	DNSQuestion *query = &context->question;
	const mDNSu8 *ptr;

	// stop any active question
	if (context->state != init && context->questionActive)
		uDNS_StopQuery(context->m, query);
	context->questionActive = mDNSfalse;
	
    if (msg)
        {
        // if msg contains SOA record in answer or authority sections, update context/state and return 		
		int i;
		ptr = LocateAnswers(msg, end);
		for (i = 0; i < msg->h.numAnswers; i++)
			{
			ptr = GetLargeResourceRecord(context->m, msg, ptr, end, 0, kDNSRecordTypePacketAns, &lcr);
			if (!ptr) { LogMsg("ERROR: GetLargeResourceRecord returned NULL");  goto error; }
			if (rr->rrtype == kDNSType_SOA && SameDomainName(context->curSOA, &rr->name))
				{
				ustrcpy(context->zone.c, rr->rdata->u.soa.mname.c);
				debugf("Located SOA w/ mname %s for name %s", ConvertDomainNameToCString(&context->zone, errbuf),
					   ConvertDomainNameToCString(&context->origName, errbuf2));
				context->state = foundZone;
				return smContinue;
				}
			}		

		// SOA not in answers, check in authority
		for (i = 0; i < msg->h.numAuthorities; i++)
			{
			ptr = GetLargeResourceRecord(context->m, msg, ptr, end, 0, kDNSRecordTypePacketAns, &lcr); ///!!!KRS using type PacketAns for auth
			if (!ptr) { LogMsg("ERROR: GetLargeResourceRecord returned NULL");  goto error; }		 		
			if (rr->rrtype != kDNSType_SOA) continue;
			ustrcpy(context->zone.c, rr->rdata->u.soa.mname.c);
			debugf("Located SOA in authority section w/ mname %s for name %s",
				   ConvertDomainNameToCString(&context->zone, errbuf), ConvertDomainNameToCString(&context->origName, errbuf2));
			context->state = foundZone;
			return smContinue;
			}
		}    

    if (context->state != init && !context->curSOA->c[0])
        {
        // we've gone down to the root and have not found an SOA  
        LogMsg("ERROR: hndlLookupSOA - recursed to root label of %s without finding SOA", 
                ConvertDomainNameToCString(&context->origName, errbuf));
		goto error;
        }

    ubzero(query, sizeof(DNSQuestion));
    // chop off leading label unless this is our first try
    if (context->state == init)  context->curSOA = &context->origName;
    else                         (void *)context->curSOA = context->curSOA->c + context->curSOA->c[0]+1;
    
    context->state = lookupSOA;
    ustrcpy(query->qname.c, context->curSOA->c);
    query->qtype = kDNSType_SOA;
    query->qclass = kDNSClass_IN;
    err = startInternalQuery(query, context->m, nameToAddr, context);
    if (err) { LogMsg("hndlLookupSOA: startInternalQuery returned error %d", err);  goto error;  }
	context->questionActive = mDNStrue;
    return smBreak;     // break from state machine until we receive another packet
   
error:
	if (context->state != init && context->questionActive)
		uDNS_StopQuery(context->m, query);
    if (context) ufree(context);
    return smBreak;  
    }

#endif //0 (incomplete)

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Truncation Handling
#endif

typedef struct
	{
    DNSQuestion  *question;
    DNSMessage reply;
    mDNSu16  replylen;
    int nread;
    mDNS *m;
	} tcpInfo_t;

// issue queries over a conected socket
mDNSlocal void conQueryCallback(int sd, void *context, mDNSBool ConnectionEstablished)
	{
	mDNSOpaque16 flags, id;
	mStatus err = 0;
	char msgbuf[356];  // 96 (hdr) + 256 (domain) + 4 (class/type)
	DNSMessage *msg;
	mDNSu8 *end;
	tcpInfo_t *info = (tcpInfo_t *)context;
	DNSQuestion *question = info->question;
	int n;

	flags.NotAnInteger = 0;
	id.NotAnInteger = (mDNSu16)~0;

	if (ConnectionEstablished)
		{
		// connection is established - send the message
		msg = (DNSMessage *)&msgbuf;
		err = constructQueryMsg(msg, &end, id, flags, question);
		if (err) { LogMsg("ERROR: conQueryCallback: constructQueryMsg - %d", err);  goto error; }
		err = mDNSSendDNSMessage_tcp(info->m, msg, end, sd);
		if (err) { LogMsg("ERROR: conQueryCallback: mDNSSendDNSMessage_tcp - %d", err);  goto error; }
		return;
		}
	else
		{
		if (!info->nread)
			{
			// read msg len
			n = mDNSPlatformReadTCP(sd, &info->replylen, 2);
			if (n != 2)
				{
				LogMsg("ERROR:conQueryCallback - attempt to read message length failed (read returned %d)", n);
				goto error;
				}
			}
		n = mDNSPlatformReadTCP(sd, ((char *)&info->reply) + info->nread, info->replylen - info->nread);
		if (n < 0) { LogMsg("ERROR: conQueryCallback - read returned %d", n); goto error; }
		info->nread += n;
		if (info->nread == info->replylen)
			{
			// finished reading message
			receiveMsg(info->m, &info->reply, ((mDNSu8 *)&info->reply) + info->replylen, question->InterfaceID);
			mDNSPlatformTCPCloseConnection(sd);
			ufree(info);
			return;
			}
		else return;
		}
	return;

	error:
	mDNSPlatformTCPCloseConnection(sd);
	ufree(info);
	}

mDNSlocal void hndlTruncatedAnswer(DNSQuestion *question, const  mDNSAddr *src, mDNS *m)
	{
	mStatus connectionStatus;
	uDNS_QuestionInfo *info = &question->uDNS_info;
	int sd;
	tcpInfo_t *context;

	context = (tcpInfo_t *)umalloc(sizeof(tcpInfo_t));
	if (!context) { LogMsg("ERROR: hndlTruncatedAnswer - memallocate failed"); return; }
	ubzero(context, sizeof(tcpInfo_t));
	context->question = question;
	context->m = m;

	info->id.NotAnInteger = (mDNSu16)~0;  // all 1's indicates TCP queries
	info->timestamp = m->timenow;         // reset timestamp

	connectionStatus = mDNSPlatformTCPConnect(src, UnicastDNSPort, question->InterfaceID, conQueryCallback, context, &sd);
	if (connectionStatus == mStatus_ConnectionEstablished)  // manually invoke callback if connection completes
		{
		conQueryCallback(sd, context, mDNStrue);
		return;
		}
	if (connectionStatus == mStatus_ConnectionPending) return; // callback will be automatically invoked when connection completes
	LogMsg("hndlTruncatedAnswer: connection failed");
	uDNS_StopQuery(m, question);  //!!!KRS can we really call this here?
	}

mDNSexport mDNSs32 uDNS_GetNextEventTime(const mDNS *const m)
    {
    const uDNS_data_t *u = &m->uDNS_data;
    (void)u;				// unused

    return m->timenow + 0x78000000;
    }
