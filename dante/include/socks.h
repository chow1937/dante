/*
 * Copyright (c) 1997, 1998, 1999
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadal�en 21
 *  N-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: socks.h,v 1.137 1999/05/13 13:27:33 karls Exp $ */

#ifndef _SOCKS_H_
#define _SOCKS_H_
#endif  /* ! _SOCKS_H_ */

#ifdef lint
extern const int lintnoloop_socks_h;
#else
#define lintnoloop_socks_h 0
#endif

#ifndef HAVE_OSF_OLDSTYLE
#define HAVE_OSF_OLDSTYLE 0
#endif  /* !HAVE_OSF_OLDSTYLE */


#ifdef SOCKSLIBRARY_DYNAMIC

#ifdef accept
#undef accept
#endif  /* accept */
#if HAVE_EXTRA_OSF_SYMBOLS
#define accept(s, addr, addrlen)			sys_Eaccept(s, addr, addrlen)
#else
#define accept(s, addr, addrlen)			sys_accept(s, addr, addrlen)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef bind
#undef bind
#endif  /* bind */
#define bind(s, name, namelen)			sys_bind(s, name, namelen)

#ifdef bindresvport
#undef bindresvport
#endif  /* bindresvport */
#define bindresvport(sd, sin)				sys_bindresvport(sd, sin)

#ifdef connect
#undef connect
#endif  /* connect */
#define connect(s, name, namelen)		sys_connect(s, name, namelen)

#ifdef gethostbyname
#undef gethostbyname
#endif  /* gethostbyname */
#define gethostbyname(name)				sys_gethostbyname(name)

#ifdef gethostbyname2
#undef gethostbyname2
#endif  /* gethostbyname2 */
#define gethostbyname2(name, af)			sys_gethostbyname2(name, af)

#ifdef getpeername
#undef getpeername
#endif  /* getpeername */
#if HAVE_EXTRA_OSF_SYMBOLS
#define getpeername(s, name, namelen)	sys_Egetpeername(s, name, namelen)
#else
#define getpeername(s, name, namelen)	sys_getpeername(s, name, namelen)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef getsockname
#undef getsockname
#endif  /* getsockname */
#if HAVE_EXTRA_OSF_SYMBOLS
#define getsockname(s, name, namelen)	sys_Egetsockname(s, name, namelen)
#else
#define getsockname(s, name, namelen)	sys_getsockname(s, name, namelen)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef read
#undef read
#endif  /* read */
#define read(d, buf, nbytes)				sys_read(d, buf, nbytes)

#ifdef readv
#undef readv
#endif  /* readv */
#if HAVE_EXTRA_OSF_SYMBOLS
#define readv(d, iov, iovcnt)				sys_Ereadv(d, iov, iovcnt)
#else
#define readv(d, iov, iovcnt)				sys_readv(d, iov, iovcnt)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef recv
#undef recv
#endif  /* recv */
#define recv(s, msg, len, flags)			sys_recv(s, msg, len, flags)

#ifdef recvfrom
#undef recvfrom
#endif  /* recvfrom */
#if HAVE_EXTRA_OSF_SYMBOLS
#define recvfrom(s, buf, len, flags, from, fromlen)	\
		  sys_Erecvfrom(s, buf, len, flags, from, fromlen)
#else
#define recvfrom(s, buf, len, flags, from, fromlen)	\
		  sys_recvfrom(s, buf, len, flags, from, fromlen)

#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef recvmsg
#undef recvmsg
#endif  /* recvmsg */
#if HAVE_EXTRA_OSF_SYMBOLS
#define recvmsg(s, msg, flags)			sys_Erecvmsg(s, msg, flags)
#else
#define recvmsg(s, msg, flags)			sys_recvmsg(s, msg, flags)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef rresvport
#undef rresvport
#endif  /* rresvport */
#define rresvport(port)						sys_rresvport(port)

#ifdef sendto
#undef sendto
#endif  /* sendto */
#define sendto(s, msg, len, flags, to, tolen)	\
		  sys_sendto(s, msg, len, flags, to, tolen)

#ifdef write
#undef write
#endif  /* write */
#define write(d, buf, nbytes)				sys_write(d, buf, nbytes)

#ifdef writev
#undef writev
#endif  /* writev */
#if HAVE_EXTRA_OSF_SYMBOLS
#define writev(d, iov, iovcnt)			sys_Ewritev(d, iov, iovcnt)
#else
#define writev(d, iov, iovcnt)			sys_writev(d, iov, iovcnt)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef send
#undef send
#endif  /* send */
#define send(s, msg, len, flags)			sys_send(s, msg, len, flags)

#ifdef sendmsg
#undef sendmsg
#endif  /* sendmsg */
#if HAVE_EXTRA_OSF_SYMBOLS
#define sendmsg(s, msg, flags)			sys_Esendmsg(s, msg, flags)
#else
#define sendmsg(s, msg, flags)			sys_sendmsg(s, msg, flags)
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#endif /* SOCKSLIBRARY_DYNAMIC */

struct configstate_t {
	unsigned				init:1;
	unsigned				:0;
	struct sockaddr	lastconnect;		/* address we last connected to.		*/
	pid_t					pid;
	unsigned				:0;
};

struct option_t {
	int					debug;
	char					*configfile;	/* name of current configfile.				*/
	unsigned				lbuf:1;			/* linebuffered output?							*/
	unsigned				:0;
};



struct config_t {
	pid_t								connectchild;				/* connect process.		*/
	int								connect_s;					/* socket to child.		*/
	char								domain[MAXHOSTNAMELEN]; /* localdomain.			*/
	struct logtype_t				log;							/* where to log.			*/
	struct option_t				option;						/* misc. options.			*/
	struct configstate_t			state;
	int								resolveprotocol;			/* resolveprotocol.		*/
	struct route_t					*route;						/* linked list of routes*/
};

struct childpacket_t {
   struct sockshost_t   src;
   struct sockshost_t   dst;
   struct socks_t       packet;
};


__BEGIN_DECLS

/*
 * libsocks function declarations
 */

void
clientinit __P((void));
/*
 * initialises clientstate, reads configfile, etc.
*/


#if !HAVE_OSF_OLDSTYLE
int Raccept __P((int, struct sockaddr *, socklen_t *));
#endif  /* !HAVE_OSF_OLDSTYLE */
#if HAVE_FAULTY_BINDPROTO
int Rbind __P((int, struct sockaddr *, int));
#else
int Rbind __P((int, const struct sockaddr *, socklen_t));
#endif  /* HAVE_FAULTY_BINDPROTO */
int Rbindresvport __P((int, struct sockaddr_in *));
#if !HAVE_OSF_OLDSTYLE
#if HAVE_FAULTY_CONNECTPROTO
int Rconnect __P((int, struct sockaddr *, int));
#else
int Rconnect __P((int, const struct sockaddr *, socklen_t));
#endif  /* HAVE_FAULTY_CONNECTPROTO */
#endif  /* !HAVE_OSF_OLDSTYLE */
#if !HAVE_OSF_OLDSTYLE
int Rgetsockname __P((int, struct sockaddr *, socklen_t *));
#endif  /* !HAVE_OSF_OLDSTYLE */
#if !HAVE_OSF_OLDSTYLE
int Rgetpeername __P((int, struct sockaddr *, socklen_t *));
#endif  /* !HAVE_OSF_OLDSTYLE */
int Rrresvport __P((int *));
struct hostent *Rgethostbyname __P((const char *));
struct hostent *Rgethostbyname2 __P((const char *, int af));
#if !HAVE_OSF_OLDSTYLE
ssize_t Rsendto __P((int s, const void *msg, size_t len, int flags,
							const struct sockaddr *to, socklen_t tolen));
#endif  /* HAVE_OSF_OLDSTYLE */

#if !HAVE_OSF_OLDSTYLE
#if HAVE_RECVFROM_CHAR
ssize_t Rrecvfrom __P((int s, char *buf, int len, int flags,
					  struct sockaddr *from, socklen_t *fromlen));
#else
ssize_t Rrecvfrom __P((int s, void *buf, size_t len, int flags,
					   struct sockaddr * from, socklen_t *fromlen));
#endif  /* HAVE_RECVFROM_CHAR */
#endif  /* !HAVE_OSF_OLDSTYLE */
ssize_t Rwrite __P((int d, const void *buf, size_t nbytes));
ssize_t Rwritev __P((int d, const struct iovec *iov, int iovcnt));
ssize_t Rsend __P((int s, const void *msg, size_t len, int flags));
#if !HAVE_OSF_OLDSTYLE
ssize_t Rsendmsg __P((int s, const struct msghdr *msg, int flags));
#endif  /* !HAVE_OSF_OLDSTYLE */
ssize_t Rread __P((int d, void *buf, size_t nbytes));
ssize_t Rreadv __P((int d, const struct iovec *iov, int iovcnt));
ssize_t Rrecv __P((int s, void *msg, size_t len, int flags));
#if !HAVE_OSF_OLDSTYLE
ssize_t Rrecvmsg __P((int s, struct msghdr *msg, int flags));
#endif  /* !HAVE_OSF_OLDSTYLE */


int SOCKSinit __P((char *));
int Rlisten __P((int, int));
int Rselect __P((int, fd_set *, fd_set *, fd_set *, struct timeval *));
/*
 * unused functions needed to compile programs with support for other
 * socks implementations.
*/


int
udpsetup __P((int s, const struct sockaddr *to, int type));
/*
 * sets up udp relaying between address of "s" and "to" by connecting
 * to socksserver.
 * If relaying is already set up the function returns with success.
 * Type is the type of connection setup, SOCKS_SEND or SOCKS_RECV.
 * At the moment only SOCKS_SEND is supported.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/


int
negotiate_method __P((int s, struct socks_t *packet));
/*
 * Negotiates a method to be used when talking with the server connected
 * to "s".  "packet" is the packet that will later be sent to server,
 * only the "auth" element in it will be set but other elements are needed
 * too.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/


int
socks_sendrequest __P((int s, const struct request_t *request));
/*
 * Sends the request "request" to the socksserver connected to "s".
 * Returns:
 *		On success: 0
 *		On failure: -1
*/

int
socks_recvresponse __P((int s, struct response_t *response, int version));
/*
 * Receives a socks response from the "s".  "response" is filled in with
 * the data received.
 * "version" is the protocolversion negotiated.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/


int
socks_negotiate __P((int s, int control, struct socks_t *packet,
							struct route_t *route));
/*
 * "s" is the socket data will flow over.
 * "control" is the control connection to the socks server.
 * "packet" is a socks packet containing the request.
 *	"route" is the connected route.
 * Negotiates method and fills the response to the request into packet->res.
 * Returns:
 *		On success: 0.  (server accepted our request.)
 *		On failure: -1.
*/



struct route_t *
socks_nbconnectroute __P((int s, int control, struct socks_t *packet,
								  const struct sockshost_t *src,
								  const struct sockshost_t *dst));
/*
 * The non-blocking version of socks_connectroute(), only used by client.
 * Takes one additional argument, "s", which is the socket to connect
 * and not necessarily the same as "control" (msproxy case).
*/

void
socks_badroute __P((struct route_t *route));
/*
 * Marks route "route" as bad.
*/

int
recv_sockshost __P((int s, struct sockshost_t *host, int version));
/*
 * Fills "host" based on data read from "s".  "version" is the version
 * the remote peer is expected to send data in.
 *
 * Returns:
 *		On success: 0
 *		On failure: -1
*/


	/*
	 *  Misc. functions to help keep track of our connection(s) to the server.
	*/

struct socksfd_t *
socks_addaddr __P((unsigned int clientfd, struct socksfd_t *socksaddress));
/*
 * "clientfd" is associated with the structure "socksfd".
 * The function duplicates all arguments in it's own form and does
 * not access the memory referenced by them afterwards.
 *
 * The function checks the state of all filedescriptors on each call and
 * removes those that are no longer open.
 *
 * Returns:
 *		On success: pointer to the added socksfd_t structure.
 *		On failure: exits.  (memory exhausted and process grew descriptor size.)
 *
*/

struct socksfd_t *
socks_getaddr __P((unsigned int fd));
/*
 * Returns:
 *		On success:  the socketaddress associated with filedescriptor "fd".
 *		On failure:	 NULL.  (no socketaddress associated with "fd").
*/


void
socks_rmaddr __P((unsigned int s));
/*
 * removes the association for the socket "s", also closes the server
 * connection.  If "s" is not registered the request is ignored.
*/

struct socksfd_t *
socksfddup __P((const struct socksfd_t *old, struct socksfd_t *new));
/*
 * Duplicates "old", in "new".
 * Returns:
 *		On success: "new".
 *		On failure: NULL (resource shortage).
*/


int
socks_addrcontrol __P((const struct sockaddr *local,
							  const struct sockaddr *remote));
/*
 * Goes through all addresses registered and tries to find one where
 * the control socket has a local address of "local" and peer address
 * of "remote".  If either of "local" or "remote" is NULL, that
 * endpoint is not checked against.
 *	Returns:
 *		On success: the descriptor the socksfd struct was registered with.
 *		On failure: -1
*/

int
socks_addrmatch __P((const struct sockaddr *local,
							const struct sockaddr *remote,
							const struct socksstate_t *state));
/*
 * Goes through all addresses registered and tries to find one where
 * all arguments match.
 * Arguments that are NULL or have "illegal" values are ignored.
 * Returns:
 *		If no match found: -1
 *		Else:  the descriptor the arguments were registered with, >= 0.
*/


int
socks_isaddr __P((unsigned int fd));
/*
 * Returns true if there is a address registered for the socket "fd", false
 * otherwise.
*/


int
socks_addrisok __P((unsigned int s));
/*
 * Compares the current address of "s" to the registered address.
 * If there is a mismatch, the function will try to correct it if possible.
 * Returns:
 *		If current address found to match registered: true.
 *		Else: false.
*/

int
socks_addfd __P((unsigned int fd));
/*
 * adds the filedescriptor "fd" to an internal table.
 * If it is already in the table the  request is ignored.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/

int
socks_isfd __P((unsigned int fd));
/*
 * returns 1 if "fd" is a filedescriptor in our internal table, 0 if not.
*/

void
socks_rmfd __P((unsigned int fd));
/*
 * removes the filedescriptor "fd" from our internal table.
*/


int
fdisopen __P((int fd));
/*
 * returns 1 if the filedescriptor "fd" currently references a open object.
 * returns 0 otherwise.
*/


int
clientmethod_uname __P((int s, const struct sockshost_t *host, int version));
/*
 * Enters username/password negotiation with the socksserver connected to
 * the socket "s".
 * "host" gives the name of the server.
 * "version" gives the socksversion established to use.
 * Returns:
 *		On success: 0
 *		On failure: whatever the remote socksserver returned as status.
*/


char *
socks_getusername __P((const struct sockshost_t *host, char *buf,
							  size_t buflen));
/*
 * Tries to determine the username of the current user, to be used
 * when negotiating with the server "host".
 * The NUL-terminated username is written to "buf", which is of length
 * "buflen".
 * Returns:
 *		On success: pointer to "buf" with the username.
 *		On failure: NULL.
*/

char *
socks_getpassword __P((const struct sockshost_t *host, const char *user,
							  char *buf, size_t buflen));
/*
 * Tries to determine the password of user "user", to be used
 * when negotiating with the server "host".
 * The NUL-terminated password is written to "buf", which is of length
 * "buflen"
 * Returns:
 *		On success: pointer to "buf" with the password.
 *		On failure: NULL.
*/


int
send_interfacerequest __P((int s, const struct interfacerequest_t *ifreq,
								   int version));
/*
 * Sends the interfacerequest "ifreq" to server connected to "s".
 * "version" is the protocolversion previously negotiated with server.
 *  Returns:
 *			On success: 0
 *			On failure: -1
*/

int
serverreplyisok __P((int version, int reply, struct route_t *route));
/*
 * "replycode" is the reply code returned by a socksserver of version
 * "version".
 * "route" is the route that was used for the socksserver and if
 * the errorcode indicates a serverfailure, it might be "badrouted".
 * Returns true if the reply indicates request succeeded, false otherwise
 * and sets errno accordingly.
*/

int
msproxy_negotiate __P((int s, int control, struct socks_t *packet));
/*
 * Negotiates with the msproxy server connected to "control".
 * "s" gives the socket to be used for dataflow.
 * "packet" contains the request and on return from the function
 * contains the response.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/


int
send_msprequest __P((int s, struct msproxy_state_t *state,
						  struct msproxy_request_t *packet));
/*
 * Sends a msproxy request to "s".
 * "state" is the current state of the connection to "s",
 * "packet" is the request to send.
*/

int
recv_mspresponse __P((int s, struct msproxy_state_t *state,
						  struct msproxy_response_t *packet));
/*
 * Receives a msproxy response from "s".
 * "state" is the current state of the connection to "s",
 * "packet" is the memory the response is read into.
*/

int
msproxy_sigio __P((int s));
/*
 * Must be called on sockets where we expect the connection to be forwarded
 * by the msproxy server.
 * "s" is the socket and must have been added with socks_addaddr() beforehand.
 * Returns:
 *		On success: 0
 *		On failure: -1
*/

int
msproxy_init __P((void));
/*
 * inits things for using a msproxyserver.
 *		On success: 0
 *		On failure: -1

*/

#ifdef SOCKSLIBRARY_DYNAMIC

#if HAVE_OSF_OLDSTYLE
int sys_accept __P((int, __SOCKADDR_ARG, int *));
#else
int sys_accept __P((int, __SOCKADDR_ARG, socklen_t *));
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */
#if !HAVE_OSF_OLDSTYLE
#if HAVE_FAULTY_BINDPROTO
int sys_bind __P((int, struct sockaddr *, int));
#else
int sys_bind __P((int, __CONST_SOCKADDR_ARG, socklen_t));
#endif  /* HAVE_FAULTY_BINDPROTO */
#else
int sys_bind __P((int, const struct sockaddr *, int));
#endif /* !HAVE_OSF_OLDSTYLE */
int sys_bindresvport __P((int, struct sockaddr_in *));
#if !HAVE_OSF_OLDSTYLE
#if HAVE_FAULTY_CONNECTPROTO
int sys_connect __P((int, struct sockaddr *, int));
#else
int sys_connect __P((int, __CONST_SOCKADDR_ARG, socklen_t));
#endif  /* HAVE_FAULTY_CONNECTPROTO */
#else
int sys_connect __P((int, const struct sockaddr *, int));
#endif  /* HAVE_OSF_OLDSTYLE */
struct hostent *sys_gethostbyname __P((const char *));
struct hostent *sys_gethostbyname2 __P((const char *, int));
#if HAVE_OSF_OLDSTYLE
int sys_getpeername __P((int, __SOCKADDR_ARG, int *));
#else
int sys_getpeername __P((int, __SOCKADDR_ARG, socklen_t *));
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */
#if HAVE_OSF_OLDSTYLE
int sys_getsockname __P((int, __SOCKADDR_ARG, int *));
#else
int sys_getsockname __P((int, __SOCKADDR_ARG, socklen_t *));
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

ssize_t sys_read __P((int, void *, size_t));
#if HAVE_FAULTY_READVPROTO || HAVE_OSF_OLDSTYLE
ssize_t sys_readv __P((int, struct iovec *, int));
#else
ssize_t sys_readv __P((int, const struct iovec *, int));
#endif  /* HAVE_FAULTY_CONNECTPROTO */
#if HAVE_RECVFROM_CHAR
ssize_t sys_recv __P((int, char *, int, int));
#else
ssize_t sys_recv __P((int, void *, size_t, int));
#endif
ssize_t sys_recvmsg __P((int, struct msghdr *, int));
#if !HAVE_OSF_OLDSTYLE
#if HAVE_RECVFROM_CHAR
int sys_recvfrom __P((int, char *, int, int, struct sockaddr *, int *));
#else
int sys_recvfrom __P((int, void *, size_t, int, __SOCKADDR_ARG, socklen_t *));
#endif  /* HAVE_RECVFROM_CHAR */
#else
int sys_recvfrom __P((int, void*, int, int, struct sockaddr *, int *));
#endif
int sys_rresvport __P((int *));
ssize_t sys_write __P((int, const void *, size_t));
#if HAVE_OSF_OLDSTYLE
ssize_t sys_writev __P((int, struct iovec *, int));
#else
ssize_t sys_writev __P((int, const struct iovec *, int));
#endif
#if HAVE_RECVFROM_CHAR
ssize_t sys_send __P((int, const char *, int, int));
#else
ssize_t sys_send __P((int, const void *, size_t, int));
#endif  /* HAVE_RECVFROM_CHAR */
#if HAVE_OSF_OLDSTYLE
ssize_t sys_sendmsg __P((int, struct msghdr *, int));
#else
ssize_t sys_sendmsg __P((int, const struct msghdr *, int));
#endif
#if !HAVE_OSF_OLDSTYLE
#if HAVE_SENDTO_ALT
ssize_t sys_sendto __P((int, const char *, int, int, __CONST_SOCKADDR_ARG,
						  socklen_t));
#else
ssize_t sys_sendto __P((int, const void *, size_t, int, __CONST_SOCKADDR_ARG,
						  socklen_t));
#endif  /* HAVE_SENDTO_ALT */
#else
int sys_sendto __P((int, const void *, int, int, const struct sockaddr *,
						  socklen_t));
#endif /* !HAVE_OSF_OLDSTYLE */

#if HAVE_EXTRA_OSF_SYMBOLS
int sys_Eaccept __P((int, __SOCKADDR_ARG, socklen_t *));
int sys_Egetpeername __P((int, __SOCKADDR_ARG, socklen_t *));
int sys_Egetsockname __P((int, __SOCKADDR_ARG, socklen_t *));
ssize_t sys_Ereadv __P((int, const struct iovec *, int));
int sys_Erecvfrom __P((int, void *, size_t, int, __SOCKADDR_ARG, size_t *));
ssize_t sys_Erecvmsg __P((int, struct msghdr *, int));
ssize_t sys_Esendmsg __P((int, const struct msghdr *, int));
ssize_t sys_Ewritev __P((int, const struct iovec *, int));
#endif  /* HAVE_EXTRA_OSF_SYMBOLS */

#endif /* SOCKSLIBRARY_DYNAMIC */

__END_DECLS