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
 *  Gaustadaléen 21
 *  N-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#include "common.h"

static const char rcsid[] =
"$Id: sockd_request.c,v 1.104 1999/09/02 10:42:12 michaels Exp $";

/*
 * Since it only handles one client at a time there is no possibility
 * for the mother to send a new client before we have got rid of the
 * old one and thus no need for locking even on broken systems.
 * (#ifdef HAVE_SENDMSG_DEADLOCK)
 * XXX I have started to work on fixing this, so this process too
 * can support multiple clients, perhaps for a later release I will
 * have time to complete it.  Will also fix the terrible fact
 * that we just sit around and wait if the command is bind, wasting
 * the whole process on practically nothing.
 */

__BEGIN_DECLS

static void
dorequest __P((int mother, const struct sockd_request_t *request));
/*
 * When a complete request has been read, this function can be
 * called.  It will perform the request "request->req" and send the
 * result to "mother".
 */

static void
flushio __P((int mother, int clientcontrol, const struct response_t *response,
				 struct sockd_io_t *io));
/*
 * "flushes" a complete io object and free's any state/resources held by it.
 * "mother" is connection to mother for sending the io.
 * "clientcontrol" is the client connection.
 * "response" is the response to be sent the client.
 * "io" is the io object sent mother.
 */

static void
proctitleupdate __P((const struct sockaddr *from));
/*
 * Updates the title of this process.
 */

static struct sockd_io_t *
io_add __P((struct sockd_io_t *iolist, const struct sockd_io_t *newio));
/*
 * Adds _a copy_ of the object "newio" to the list "iolist".
 * Returns a pointer to the (new) iolist.
 */

static struct sockd_io_t *
io_remove __P((struct sockd_io_t *iolist, struct sockd_io_t *rmio));
/*
 * Removes the object "rmio" from the list "iolist".
 * Returns a pointer to the (new) iolist.
 */

static struct sockd_io_t *
io_find __P((struct sockd_io_t *iolist, const struct sockaddr *addr));
/*
 * Scans "iolist" for a object that contains "addr" as a local address.
 * If "addr" is NULL, returns "iolist".
 * Returns:
 *		On success: pointer to the matching io object.
 *		On failure: NULL.
 */


__END_DECLS


void
run_request(mother)
	struct sockd_mother_t *mother;
{
	const char *function = "run_request()";
	struct sockd_request_t req;
#if DIAGNOSTIC
	const int freec = freedescriptors(config.option.debug ? "start" : NULL);
#endif /* DIAGNOSTIC */

	proctitleupdate(NULL);

	/* CONSTCOND */
	while (1) {
		/*
		 * Get request from mother, perform it, get next request.
		 */
		const char command = SOCKD_FREESLOT;

		proctitleupdate(NULL);

		if (recv_req(mother->s, &req) == -1)
			sockdexit(-EXIT_FAILURE);

		/* LINTED pointer casts may be troublesome */
		proctitleupdate((struct sockaddr *)&req.from);

		dorequest(mother->s, &req);

		if (writen(mother->ack, &command, sizeof(command)) != sizeof(command))
			serr(EXIT_FAILURE, "%s: sending ack to mother failed", function);

#if DIAGNOSTIC
		SASSERTX(freec == freedescriptors(config.option.debug ? "end" : NULL));
#endif /* DIAGNOSTIC */
	}
}


int
recv_req(s, req)
	int s;
	struct sockd_request_t *req;
{
	const char *function = "recv_req()";
	int fdexpect, fdreceived, r;
	struct iovec iovec[1];
	struct msghdr msg;
	CMSG_AALLOC(sizeof(int));

	iovec[0].iov_base		= req;
	iovec[0].iov_len		= sizeof(*req);

	msg.msg_iov				= iovec;
	msg.msg_iovlen			= ELEMENTS(iovec);
	msg.msg_name			= NULL;
	msg.msg_namelen		= 0;

	CMSG_SETHDR_RECV(sizeof(cmsgmem));

	if ((r = recvmsgn(s, &msg, 0, sizeof(*req))) != sizeof(*req)) {
		switch (r) {
			case -1:
				swarn("%s: recvmsg() from mother", function);
				break;

			case 0:
				slog(LOG_DEBUG, "%s: recvmsg(): mother closed connection",
				function);
				break;

			default:
				swarnx("%s: recvmsg(): unexpected %d/%d bytes from mother",
				function, r, sizeof(*req));
		}

		return -1;
	}
	fdexpect = 1;

#if !HAVE_DEFECT_RECVMSG
	SASSERT(CMSG_GETLEN(msg) == sizeof(int) * fdexpect);
#endif

	fdreceived = 0;
	CMSG_GETOBJECT(req->s, sizeof(req->s) * fdreceived++);

	/* pointer fixup */
	req->req.auth = &req->state.auth;

	return 0;
}

static void
dorequest(mother, request)
	int mother;
	const struct sockd_request_t *request;
{
	const char *function = "dorequest()";
	static const struct sockd_io_t ioinit;
	struct sockaddr_in bound;
	struct sockd_io_t io;
	struct response_t response;
	char a[MAXSOCKSHOSTSTRING], b[MAXSOCKSHOSTSTRING];
	int p, permit, out;

	slog(LOG_DEBUG, "received request: %s",
	socks_packet2string(&request->req, SOCKS_REQUEST));

	bzero(&response, sizeof(response));
	response.host	= request->req.host;
	response.auth	= request->req.auth;

	io 						= ioinit;
	io.acceptrule			= request->rule;
	io.state					= request->state;
	io.state.extension	= config.extension;

	/*
	 * examine client request.
	 */

	/* supported version? */
	switch (request->req.version) {
		case SOCKS_V4:
			response.version = SOCKS_V4REPLY_VERSION;

			/* recognized command for this version? */
			switch (request->req.command) {
				case SOCKS_BIND:
				case SOCKS_CONNECT:
					io.state.protocol = SOCKS_TCP;
					break;

				default:
					/* LINTED pointer casts may be troublesome */
					slog(LOG_INFO, "%s: unrecognized v%d command: %d",
					sockaddr2string((const struct sockaddr *)&request->from,
					a, sizeof(a)), request->req.version, request->req.command);
					send_failure(request->s, &response, SOCKS_FAILURE);
					close(request->s);
					return;
			}

			/* supported address format for this version? */
			switch (request->req.host.atype) {
				case SOCKS_ADDR_IPV4:
					break;

				default:
					/* LINTED pointer casts may be troublesome */
					slog(LOG_INFO, "%s: unrecognized v%d address type: %d",
					sockaddr2string((const struct sockaddr *)&request->from,
					a, sizeof(a)), request->req.version, request->req.host.atype);
					send_failure(request->s, &response, SOCKS_ADDR_UNSUPP);
					close(request->s);
					return;
			}
			break; /* SOCKS_V4 */

		case SOCKS_V5:
			response.version = request->req.version;

			/* recognized command for this version? */
			switch (request->req.command) {
				case SOCKS_BIND:
				case SOCKS_CONNECT:
					io.state.protocol = SOCKS_TCP;
					break;

				case SOCKS_UDPASSOCIATE:
					io.state.protocol = SOCKS_UDP;
					break;

				default:
					/* LINTED pointer casts may be troublesome */
					slog(LOG_INFO, "%s: unrecognized v%d command: %d",
					sockaddr2string((const struct sockaddr *)&request->from,
					a, sizeof(a)), request->req.version, request->req.command);
					send_failure(request->s, &response, SOCKS_CMD_UNSUPP);
					close(request->s);
					return;
			}

			/* supported address format for this version? */
			switch (request->req.host.atype) {
				case SOCKS_ADDR_IPV4:
				case SOCKS_ADDR_DOMAIN:
					break;

				default:
					/* LINTED pointer casts may be troublesome */
					slog(LOG_INFO, "%s: unrecognized v%d address type: %d",
					sockaddr2string((const struct sockaddr *)&request->from,
					a, sizeof(a)), request->req.version, request->req.host.atype);
					send_failure(request->s, &response, SOCKS_ADDR_UNSUPP);
					close(request->s);
					return;
			}
			break; /* SOCKS_V5 */

		default:
			SERRX(request->req.version);
	}


	/*
	 * packet looks ok, fill in remaining bits needed to check rules.
	 */

	switch (request->req.command) {
		case SOCKS_BIND:
			/* LINTED pointer casts may be troublesome */
			sockaddr2sockshost((const struct sockaddr *)&request->from, &io.src);
			io.dst = request->req.host;

			if (io.dst.atype					!= SOCKS_ADDR_IPV4
			||  io.dst.addr.ipv4.s_addr	!= htonl(0)
			||  io.dst.port					== htons(0))
				io.state.extension.bind = 0;	/* not requesting bind extension. */
			break;

		case SOCKS_CONNECT:
			/* LINTED pointer casts may be troublesome */
			sockaddr2sockshost((const struct sockaddr *)&request->from, &io.src);
			io.dst = request->req.host;
			break;

		case SOCKS_UDPASSOCIATE:
			/*
			 * for UDP_ASSOCIATE we are getting clients udp address,
			 * not destination in request.
			 * Destination address will be checked in the i/o loop for
			 * each destination, for now just set it to INADDR_ANY.
			 */

			io.src						= request->req.host;
			io.dst.atype				= SOCKS_ADDR_IPV4;
			io.dst.addr.ipv4.s_addr	= htonl(INADDR_ANY);
			io.dst.port					= htons(0);
			break;

		default:
			SERRX(request->req.command);
	}

	/* socket to use for outgoing connection. */
	switch (io.state.protocol) {
		case SOCKS_TCP:
			if ((out = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				swarn("%s: socket(SOCK_STREAM)", function);
			break;

		case SOCKS_UDP:
			if ((out = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
				swarn("%s: socket(SOCK_DGRAM)", function);
			break;

		default:
			SERRX(io.state.protocol);
	}

	if (out == -1) {
		send_failure(request->s, &response, SOCKS_FAILURE);
		close(request->s);
		return;
	}
	setsockoptions(out);

	/* find out what address to bind on clients behalf. */
	bound	= *config.externalv;
	switch (request->req.command) {
		case SOCKS_BIND:
			/* find out what port to bind;  v4/v5 semantics?  bind extension? */
			switch (request->req.version) {
				case SOCKS_V4:
					if (io.state.extension.bind)
						bound.sin_port	= io.dst.port;
					else
						/* best we can try for is to use same port as source. */
						bound.sin_port	= request->from.sin_port;
					break;

				case SOCKS_V5:
					bound.sin_port = io.dst.port;
					break;

				default:
					SERRX(request->req.version);
			}
			break;

		case SOCKS_CONNECT:
			bound.sin_port	= request->from.sin_port;
			break;

		case SOCKS_UDPASSOCIATE:
			bound.sin_port	= request->req.host.port;
			break;

		default:
			SERRX(request->req.command);
	}


	/*
	 * bind socket.
	 */

	if (PORTISRESERVED(bound.sin_port) && config.compat.sameport) {
		uid_t euid;

		socks_seteuid(&euid, config.uid.privileged);
		p = bindresvport(out, &bound);
		socks_reseteuid(config.uid.privileged, euid);
	}
	else
		/* LINTED pointer casts may be troublesome */
		p = sockd_bind(out, (struct sockaddr *)&bound, 1);

	if (p != 0) { /* no such luck, bind any port and let client decide if ok. */
		bound.sin_port	= htons(0);
		/* LINTED pointer casts may be troublesome */
		if ((p = sockd_bind(out, (struct sockaddr *)&bound, 0)) != 0)
			swarn("%s: bind(%s)",
			/* LINTED pointer casts may be troublesome */
			function, sockaddr2string((struct sockaddr *)&bound, a, sizeof(a)));
	}

	if (p != 0) {
		send_failure(request->s, &response, errno2reply(errno, response.version));
		close(request->s);
		close(out);
		return;
	}
		
	/* rules permit? */
	switch (request->req.command) {
		case SOCKS_BIND: {
			struct sockshost_t boundhost;
			socklen_t boundlen;

			boundlen = sizeof(bound);
			/* LINTED pointer casts may be troublesome */
			if (getsockname(out, (struct sockaddr *)&bound, &boundlen) != 0) {
				swarn("%s: getsockname(out)", function);
				close(out);
				return;
			}
			/* LINTED pointer casts may be troublesome */
			sockaddr2sockshost((struct sockaddr *)&bound, &boundhost);

			permit
			= rulespermit(request->s, &io.rule, &io.state, &io.src, &boundhost);
			iolog(&io.rule, &io.state, OPERATION_CONNECT, &io.src, &boundhost,
			NULL, 0);

			break;
		}

		case SOCKS_CONNECT:
			permit
			= rulespermit(request->s, &io.rule, &io.state, &io.src, &io.dst);
			iolog(&io.rule, &io.state, OPERATION_CONNECT, &io.src, &io.dst,
			NULL, 0);
			break;

		case SOCKS_UDPASSOCIATE: {
			struct sockshost_t *src;

			/*
			 * Client is allowed to send a "incomplete" address.
			 */
			if (io.src.atype == SOCKS_ADDR_IPV4 
			&& (io.src.addr.ipv4.s_addr == htonl(0) || io.src.port == htons(0)))
				src = NULL;

			permit = rulespermit(request->s, &io.rule, &io.state, src, NULL);
			iolog(&io.rule, &io.state, OPERATION_CONNECT, &io.src, &io.dst,
			NULL, 0);
			break;
		}

		default:
			SERRX(request->req.command);
	}

	if (!permit) {
		send_failure(request->s, &response, SOCKS_NOTALLOWED);
		close(request->s);
		close(out);
		return;
	}

	/*
	 * Set up missing bits of io and send it to mother.
	 */
	switch (io.state.command) {
		case SOCKS_BIND: {
			struct sockd_io_t *iolist;
			struct sockd_io_t bindio;			/* send this to proxyrelayer.			*/
			struct sockaddr boundaddr;			/* address we listen on.				*/
			struct sockaddr clientaddr;		/* clientaddress we forward to.		*/
			socklen_t len;
			int flags, emfile;
			enum socketindex { client, childpipe, ourpipe, reply, remote };
			/* array of sockets, indexed by above enums, -1 if not open. */
			int sv[(int)(remote) + 1] = { -1, -1, -1, -1, -1 };

			SASSERTX(sv[ELEMENTS(sv) - 1] == -1);
			sv[client] = request->s;

			if (listen(out, 5) != 0) {
				swarn("%s: listen(out)", function);
				send_failure(sv[client], &response, SOCKS_FAILURE);
				break;
			}

			/* for accept(). */
			if ((flags = fcntl(out, F_GETFL, 0)) == -1
			|| fcntl(out, F_SETFL, flags | NONBLOCKING) == -1) {
				swarn("%s: fcntl()", function);
				send_failure(sv[client], &response, SOCKS_FAILURE);
				break;
			}

			len = sizeof(boundaddr);
			if (getsockname(out, &boundaddr, &len) != 0) {
				swarn("%s: getsockname()", function);
				send_failure(sv[client], &response, SOCKS_FAILURE);
				break;
			}

			sockaddr2sockshost(&boundaddr, &response.host);
			response.reply	= (char)sockscode(response.version, SOCKS_SUCCESS);

			/* LINTED pointer casts may be troublesome */
			clientaddr = *(const struct sockaddr *)&request->from;

			if (io.state.extension.bind) {
				int pipev[2];

				/*
				 * The problem is that both we and the process which receives
				 * the io packet needs to know when the client closes it's
				 * connection, but _we_ need to receive a query from the
				 * client on the connection aswell, and the io process would
				 * get confused about that.  We try to hack around that
				 * by making a "dummy" descriptor that the io process can
				 * check as all other controlconnections and which we
				 * can close when the client closes the real controlconnection,
				 * so the io process can detect it.  Not very nice, no.
				 */

				if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipev) != 0) {
					swarn("%s: socketpair()", function);
					send_failure(sv[client], &response, SOCKS_FAILURE);
					break;
				}

				sv[childpipe]	= pipev[0];
				sv[ourpipe]		= pipev[1];

				/* LINTED pointer casts may be troublesome */
				((struct sockaddr_in *)&clientaddr)->sin_port = io.dst.port;
			}

			/* let client know what address we bound to on it's behalf. */
			if (send_response(sv[client], &response) != 0) {
				iolog(&io.rule, &io.state, OPERATION_ABORT, &io.src, &response.host,
				NULL, 0);
				break;
			}

			setproctitle("bindrelayer: %s -> %s",
			sockaddr2string(&boundaddr, a, sizeof(a)),
			sockaddr2string(&clientaddr, b, sizeof(b)));

		 	/*
		 	 * regardless of what kind of bind semantics are in use, 
			 * portnumber is something we ignore when checking remote peer.
			 */
			io.dst.port = htons(0);

			emfile = 0;
			iolist = NULL;

			/* CONSTCOND */
			while (1) {
				struct ruleaddress_t ruleaddr;
				struct sockaddr remoteaddr;		/* remote address we accepted.	*/
				struct sockshost_t remotehost;	/* remote address, sockhost form.*/
				struct sockaddr_in replyaddr;		/* address of bindreply socket.	*/
				int fdbits = -1;
				fd_set rset;

				/* some sockets change, most remain the same. */
				sv[reply]	= -1;
				sv[remote]	= -1;

				FD_ZERO(&rset);
				FD_SET(sv[client], &rset);
				fdbits = MAX(fdbits, sv[client]);

				if (!emfile) {
					FD_SET(out, &rset);
					fdbits = MAX(fdbits, out);
				}

				++fdbits;
				if ((p = selectn(fdbits, &rset, NULL, NULL, NULL)) <= 0)
					SERR(p);

				if (FD_ISSET(sv[client], &rset)) {
					/*
					 * nothing is normally expected on controlconnection so
					 * assume it's a bind extension query or eof.
					 */
					struct request_t query;
					struct response_t queryresponse;
					struct negotiate_state_t state;
					struct sockaddr queryaddr;

					bzero(&state, sizeof(state));
					bzero(&query, sizeof(query));
					bzero(&queryresponse, sizeof(queryresponse));

					query.auth = request->req.auth;
					switch (p = recv_sockspacket(sv[client], &query, &state)) {
						case -1:
							iolog(&io.rule, &io.state, OPERATION_ABORT, &io.src,
							&response.host, NULL, 0);
							break;

						case 0:
							p = -1; /* session ended. */
							break;

						default: {
							struct sockd_io_t *fio;

							slog(LOG_DEBUG, "received request: %s",
							socks_packet2string(&query, SOCKS_REQUEST));

							switch (query.version) {
								case SOCKS_V4:
									queryresponse.version = SOCKS_V4REPLY_VERSION;
									break;

								case SOCKS_V5:
									queryresponse.version = query.version;
									break;
								
								default:	
									SERRX(query.version);
							}

							sockshost2sockaddr(&query.host, &queryaddr);
							if ((fio = io_find(iolist, &queryaddr)) == NULL) {
								queryresponse.host.atype				= SOCKS_ADDR_IPV4;
								queryresponse.host.addr.ipv4.s_addr = htonl(0);
								queryresponse.host.port					= htons(0);
							}
							else {
								SASSERTX(fio->state.command = SOCKS_BINDREPLY);

								/* LINTED pointer casts may be troublesome */
								SASSERTX(sockaddrareeq((struct sockaddr *)
								&fio->in.laddr, &queryaddr));

								/* LINTED pointer casts may be troublesome */
								sockaddr2sockshost((struct sockaddr *)&fio->out.raddr,
								&queryresponse.host);
							}

							if (fio != NULL) {
								flushio(mother, sv[client], &queryresponse, fio);
								emfile = MAX(0, emfile - 3); /* flushio() closes 3. */
								iolist = io_remove(iolist, fio);
								p = 0;
							}
							else
								if ((p = send_response(sv[client], &queryresponse))
								!= 0)
									iolog(&io.rule, &io.state, OPERATION_ABORT, &io.src,
									&response.host, NULL, 0);
						}
					}

					if (p != 0)
						break;
				}

				if (!FD_ISSET(out, &rset))
					continue;

				len = sizeof(remoteaddr);
				if ((sv[remote] = acceptn(out, &remoteaddr, &len)) == -1) {
					swarn("%s: accept(out)", function);

					switch (errno) {
#ifdef EPROTO
						case EPROTO:			/* overloaded SVR4 error */
#endif
						case EWOULDBLOCK:		/* BSD */
						case ECONNABORTED:	/* POSIX */

						/* rest appears to be linux stuff according to apache src. */
#ifdef ECONNRESET
						case ECONNRESET:
#endif
#ifdef ETIMEDOUT
						case ETIMEDOUT:
#endif
#ifdef EHOSTUNREACH
						case EHOSTUNREACH:
#endif
#ifdef ENETUNREACH
						case ENETUNREACH:
#endif
							continue;

						case EMFILE:
						case ENFILE:
							++emfile;
							continue;
					}
					break; /* errno is not ok. */
				}
				sockaddr2sockshost(&remoteaddr, &remotehost);

				/* accepted connection; does remote address match requested? */
				if (io.state.extension.bind
				|| addressmatch(sockshost2ruleaddress(&io.dst, &ruleaddr),
				&remotehost, SOCKS_TCP, 1)) {
					bindio						= io; /* quick init of most stuff. */
					bindio.src					= remotehost;
					sockaddr2sockshost(&clientaddr, &bindio.dst);
					bindio.state.command		= SOCKS_BINDREPLY;

					permit = rulespermit(sv[client], &bindio.rule, &bindio.state,
					&bindio.src, &bindio.dst);

					iolog(&bindio.rule, &bindio.state, OPERATION_CONNECT,
					&bindio.src, &bindio.dst, NULL, 0);

				}
				else {
					slog(LOG_INFO, "%s(0): unexpected bindreply: %s -> %s",
					VERDICT_BLOCKs, sockaddr2string(&remoteaddr, a, sizeof(a)),
					sockshost2string(&io.src, b, sizeof(b)));
					permit = 0;
				}

				if (!permit) {
					close(sv[remote]);
					continue; /* wait for next connect, but will there be one? */
				}

				/*
				 * Someone connected to socket we listen to on behalf of client.
				 * If we are using the bind extension, connect to address client
				 * is listening on.  Otherwise, send the data on the connection
				 * we already have.
				 */

				if (bindio.state.extension.bind) {
					if ((sv[reply] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
						swarn("%s: socket(SOCK_STREAM)", function);

						switch (errno) {
							case EMFILE:
							case ENFILE:
								++emfile;
								/* FALLTHROUGH */

							case ENOBUFS:
								close(sv[remote]);
								continue;
						}
						break; /* errno is not ok. */
					}
					setsockoptions(sv[reply]);

					/* LINTED pointer casts may be troublesome */
					replyaddr				= *(struct sockaddr_in *)&boundaddr;
					replyaddr.sin_port	= htons(0);

					/* LINTED pointer casts may be troublesome */
					if (sockd_bind(sv[reply], (struct sockaddr *)&replyaddr, 0)
					!= 0) {
						/* LINTED pointer casts may be troublesome */
						swarn("%s: bind(%s)", function,
						sockaddr2string((struct sockaddr *)&replyaddr, a, sizeof(a)));
						break;
					}

					len = sizeof(replyaddr);
					/* LINTED pointer casts may be troublesome */
					if (getsockname(sv[reply], (struct sockaddr *)&replyaddr, &len)
					!= 0) {
						swarn("%s: getsockname(sv[reply])", function);
						if (errno == ENOBUFS) {
							close(sv[remote]);
							close(sv[reply]);
							continue;
						}
						break;
					}

					slog(LOG_DEBUG, "connecting to %s",
					sockaddr2string(&clientaddr, a, sizeof(a)));

					if (connect(sv[reply], &clientaddr, sizeof(clientaddr)) != 0) {
						iolog(&bindio.rule, &bindio.state, OPERATION_ABORT,
						&bindio.src, &bindio.dst, NULL, 0);
						break;
					}
				}

				if (bindio.state.extension.bind) {
					/*
					 * flushio() will close all descriptors set in io packet
					 * so dup what we need to keep going.
					 */

					if ((bindio.control.s = dup(sv[childpipe])) == -1) {
						switch (errno) {
							case EMFILE:
							case ENFILE:
								swarn("%s: dup()", function);
								++emfile;
								continue;

							default:
								SERR(bindio.control.s);
						}
					}
				}
				else
					bindio.control.s = sv[client];

				bindio.control.laddr	= request->to;
				bindio.control.raddr	= request->from;
				bindio.control.state	= bindio.state;

				/* back to blocking. */
				if ((flags = fcntl(sv[remote], F_GETFL, 0)) == -1
				||  fcntl(sv[remote], F_SETFL, flags & ~NONBLOCKING) == -1) {
					swarn("%s: fcntl()", function);
					break;
				}

				if (bindio.state.extension.bind) {
					bindio.in.s			= sv[reply];
					bindio.in.laddr	= replyaddr;
					bindio.in.state	= bindio.state;
				}
				else {
					bindio.in			= bindio.control;
					bindio.in.laddr	= request->from;
				}
				/* LINTED pointer casts may be troublesome */
				bindio.in.raddr					= *(struct sockaddr_in *)&clientaddr;

				bindio.out.s						= sv[remote];
				/* LINTED pointer casts may be troublesome */
				bindio.out.laddr					= *(struct sockaddr_in *)&boundaddr;
				/* LINTED pointer casts may be troublesome */
				bindio.out.raddr					= *(struct sockaddr_in *)&remoteaddr;
				bindio.out.state.auth.method	= AUTHMETHOD_NONE;

				if (bindio.state.extension.bind)
					/* add to list, client will query. */
					iolist = io_add(iolist, &bindio);
				else {
					response.host = bindio.dst;
					flushio(mother, sv[client], &response, &bindio);
					sv[client] = sv[remote] = -1; /* flushio() closes; closev(). */
					break;	/* only one connection to relay and that is done. */
				}
			}

			if (bindio.state.extension.bind) {
				struct sockd_io_t *rmio;

				/* delete any connections we have queued. */
				while ((rmio = io_find(iolist, NULL)) != NULL) {
					close_iodescriptors(rmio);
					iolist = io_remove(iolist, rmio);
				}
			}

			closev(sv, ELEMENTS(sv));
			break;
		}

		case SOCKS_CONNECT: {
			socklen_t sinlen;

			if (socks_connect(out, &io.dst) != 0) {
				iolog(&io.rule, &io.state, OPERATION_ABORT, &io.src, &io.dst,
				NULL, 0);
				send_failure(request->s, &response,
				errno2reply(errno, response.version));
				close(request->s);
				break;
			}

			io.in.s			= request->s;
			io.in.laddr		= request->to;
			io.in.raddr		= request->from;
			io.in.state		= io.state;

			io.out.s			= out;
			io.out.state	= io.state;
			sinlen			= sizeof(io.out.raddr);
			/* LINTED pointer casts may be troublesome */
			if (getpeername(io.out.s, (struct sockaddr *)&io.out.raddr, &sinlen)
			!= 0) {
				swarn("%s: getpeername(io.out.s)", function);
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				break;
			}

			sinlen = sizeof(io.out.laddr);
			/* LINTED pointer casts may be troublesome */
			if (getsockname(io.out.s, (struct sockaddr *)&io.out.laddr, &sinlen)
			!= 0) {
				swarn("%s: getsockname(io.out.s)", function);
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				break;
			}

			/* LINTED pointer casts may be troublesome */
			sockaddr2sockshost((struct sockaddr *)&io.out.laddr, &response.host);
			response.reply	= (char)sockscode(response.version, SOCKS_SUCCESS);

			flushio(mother, request->s, &response, &io);
			break;
		}

		case SOCKS_UDPASSOCIATE: {
			struct sockaddr_in client;
			socklen_t boundlen;
			int clientfd;

			/* socket we receive datagram's from client on */
			if ((clientfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
				swarn("%s: socket(SOCK_DGRAM)", function);
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				break;
			}
			setsockoptions(clientfd);

			/* LINTED pointer casts may be troublesome */
			sockshost2sockaddr(&request->req.host, (struct sockaddr *)&client);

			io.in.s					= clientfd;
			io.in.state				= io.state;
			io.in.raddr				= client;
			io.in.laddr				= request->to;
			io.in.laddr.sin_port = htons(0);

			/*
			 * bind to address for receiving udp packets so we can tell client
			 * where to send its packets.
			 */
			/* LINTED pointer casts may be troublesome */
			if (sockd_bind(clientfd, (struct sockaddr *)&io.in.laddr, 0) != 0) {
				/* LINTED pointer casts may be troublesome */
				swarn("%s: bind(%s)", function,
				sockaddr2string((struct sockaddr *)&io.in.laddr, a, sizeof(a)));
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				close(clientfd);
				break;
			}

			boundlen = sizeof(io.in.laddr);
			/* LINTED pointer casts may be troublesome */
			if (getsockname(clientfd, (struct sockaddr *)&io.in.laddr, &boundlen)
			!= 0) {
				swarn("%s: getsockname(clientfd)", function);
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				close(clientfd);
				break;
			}

			io.out.s							= out;
			io.out.state					= io.state;
			io.out.state.auth.method	= AUTHMETHOD_NONE;

			boundlen = sizeof(io.out.laddr);
			/* LINTED pointer casts may be troublesome */
			if (getsockname(out, (struct sockaddr *)&io.out.laddr, &boundlen)
			!= 0) {
				swarn("%s: getsockname(out)", function);
				send_failure(request->s, &response, SOCKS_FAILURE);
				close(request->s);
				close(clientfd);
				break;
			}

			/* remote out changes each time, set to INADDR_ANY for now. */
			bzero(&io.out.raddr, sizeof(io.out.raddr));
			io.out.raddr.sin_family			= AF_INET;
			io.out.raddr.sin_addr.s_addr	= htonl(INADDR_ANY);
			io.out.raddr.sin_port			= htons(0);

			io.control.s						= request->s;
			io.control.laddr					= request->from;
			io.control.raddr					= request->to;
			io.control.state					= io.state;

			if (request->req.flag & SOCKS_USECLIENTPORT)
				if (client.sin_port == io.out.laddr.sin_port)
					response.flag |= SOCKS_USECLIENTPORT;

			/* LINTED pointer casts may be troublesome */
			sockaddr2sockshost((struct sockaddr *)&io.in.laddr, &response.host);
			response.reply	= (char)sockscode(response.version, SOCKS_SUCCESS);
			flushio(mother, request->s, &response, &io);
			break;
		}

		default:
			SERRX(request->req.command);
	}

	close(out);
}


static void
flushio(mother, clientcontrol, response, io)
	int mother;
	int clientcontrol;
	const struct response_t *response;
	struct sockd_io_t *io;
{
	const char *function = "flushio()";
	socklen_t len;
	int sndlowat, value;
	float skew;

	switch (io->state.command) {
		case SOCKS_UDPASSOCIATE:
			sndlowat = SOCKD_BUFSIZEUDP;
			skew		= 1.0; /* no skew. */
			break;

		default:
			sndlowat = SOCKD_BUFSIZETCP;
			skew 		= LOWATSKEW;
	}

	/* set socket options for relay process. */

#if SOCKD_IOMAX == 1
	/* only one client per process; doesn't matter much whether we block. */
	io->in.sndlowat	= sndlowat;
	io->out.sndlowat	= sndlowat;

#elif	HAVE_SO_SNDLOWAT

	len = sizeof(value);
	if (getsockopt(io->in.s, SOL_SOCKET, SO_SNDBUF, &value, &len) != 0)
		swarn("%s: getsockopt(in, SO_SNDBUF)", function);
	sndlowat = MIN(sndlowat, value * skew);

	if (setsockopt(io->in.s, SOL_SOCKET, SO_SNDLOWAT, &sndlowat,
	sizeof(sndlowat)) != 0)
		swarn("%s: setsockopt(in, SO_SNDLOWAT)", function);

	len = sizeof(io->in.sndlowat);
	if (getsockopt(io->in.s, SOL_SOCKET, SO_SNDLOWAT, &io->in.sndlowat, &len)
	!= 0)
		swarn("%s: getsockopt(in, SO_SNDLOWAT)", function);

	len = sizeof(value);
	if (getsockopt(io->out.s, SOL_SOCKET, SO_SNDBUF, &value, &len) != 0)
		swarn("%s: getsockopt(out, SO_SNDBUF)", function);
	sndlowat = MIN(sndlowat, value * skew);

	if (setsockopt(io->out.s, SOL_SOCKET, SO_SNDLOWAT, &sndlowat,
	sizeof(sndlowat)) != 0)
		swarn("%s: setsockopt(out, SO_SNDLOWAT", function);

	len = sizeof(io->out.sndlowat);
	if (getsockopt(io->in.s, SOL_SOCKET, SO_SNDLOWAT, &io->out.sndlowat, &len)
	!= 0)
		swarn("%s: getsockopt(in, SO_SNDLOWAT", function);

#else	/* SOCKD_IOMAX > 1 && !HAVE_SO_SNDLOWAT. */
	switch (io->state.command) {
		case SOCKS_UDPASSOCIATE:
			len = sizeof(sndlowat);
			if (getsockopt(io->in.s, SOL_SOCKET, SO_SNDBUF, &sndlowat, &len) != 0){
				swarn("%s: getsockopt(SO_SNDBUF", function);
				io->in.sndlowat = SOCKD_BUFSIZEUDP;
			}
			else if (sndlowat == 0)
				io->in.sndlowat = SOCKD_BUFSIZEUDP;
			else
				io->in.sndlowat = sndlowat;

			len = sizeof(sndlowat);
			if (getsockopt(io->out.s, SOL_SOCKET, SO_SNDBUF, &sndlowat, &len)
			!= 0) {
				swarn("%s: getsockopt(SO_SNDBUF", function);
				io->out.sndlowat = SOCKD_BUFSIZEUDP;
			}
			else if (sndlowat == 0)
				io->out.sndlowat = SOCKD_BUFSIZEUDP;
			else
				io->out.sndlowat = sndlowat;

			break;

		default:
			/* TCP; use minimum guess. */
			io->in.sndlowat	= SO_SNDLOWAT_SIZE;
			io->out.sndlowat	= SO_SNDLOWAT_SIZE;
	}
#endif  /* SOCKD_IOMAX > 1 && !HAVE_SO_SNDLOWAT */

	SASSERTX(io->in.sndlowat > 0
	&& io->out.sndlowat >= sizeof(struct udpheader_t));

	if (send_response(clientcontrol, response) == 0)
		if (send_io(mother, io) != 0)
			serr(EXIT_FAILURE, "%s: sending io to mother failed", function);

	close_iodescriptors(io);
}


static void
proctitleupdate(from)
	const struct sockaddr *from;
{
	char fromstring[MAXSOCKADDRSTRING];

	setproctitle("requestcompleter: %s",
	from == NULL ?
	"<idle>" : sockaddr2string(from, fromstring, sizeof(fromstring)));
}

static struct sockd_io_t *
io_add(iolist, newio)
	struct sockd_io_t *iolist;
	const struct sockd_io_t *newio;
{
	const char *function = "io_add()";
	struct sockd_io_t *io, *previo;

	SASSERTX(newio->next == NULL);

	previo = io = iolist;
	while (io != NULL) {
		previo = io;
		io = io->next;
	}

	if ((io = (struct sockd_io_t *)malloc(sizeof(*newio))) == NULL)
		swarnx("%s: %s", function, NOMEM);
	else {
		*io = *newio;

		if (previo == NULL)
			previo = io;
		else
			previo->next = io;
	}

	return iolist == NULL ? previo : iolist;
}

static struct sockd_io_t *
io_remove(iolist, rmio)
	struct sockd_io_t *iolist;
	struct sockd_io_t *rmio;
{
	struct sockd_io_t *io, *previo;

	SASSERTX(iolist != NULL);

	if (iolist == rmio) {
		iolist = rmio->next;
		free(rmio);
		return iolist;
	}

	previo = iolist;
	io = iolist->next;
	while (io != NULL) {
		if (io == rmio) {
			previo->next = rmio->next;
			free(rmio);
			break;
		}

		previo = io;
		io = io->next;
	}

	return iolist;
}

static struct sockd_io_t *
io_find(iolist, addr)
	struct sockd_io_t *iolist;
	const struct sockaddr *addr;
{
	struct sockd_io_t *io;

	if (addr == NULL)
		return iolist;

	io = iolist;
	while (io != NULL)
		/* LINTED pointer casts may be troublesome */
		if (sockaddrareeq((struct sockaddr *)&io->in.laddr, addr)
		||  sockaddrareeq((struct sockaddr *)&io->out.laddr, addr)
		||  sockaddrareeq((struct sockaddr *)&io->control.laddr, addr))
			return io;
		else
			io = io->next;

	/* XXX should actually check that the io is still "active". */

	return NULL;
}
