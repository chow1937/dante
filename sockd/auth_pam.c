/*
 * Copyright (c) 2001
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
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#include "common.h"

#if HAVE_PAM

static const char rcsid[] =
"$Id: auth_pam.c,v 1.12 2001/11/11 13:38:33 michaels Exp $";

__BEGIN_DECLS

static int
_pam_conversation(int num_msg, const struct pam_message **msgs,
struct pam_response **rsps, void * priv_data);

typedef struct
{
	const char *user;
	const char *password;
} _pam_data_t;

#ifdef HAVE_SOLARIS_PAM_BUG
static _pam_data_t * _pam_priv_data;
#endif /* HAVE_SOLARIS_PAM_BUG */


__END_DECLS

int
pam_passwordcheck(s, src, dst, auth, emsg, emsgsize)
	int s;
	const struct sockaddr *src, *dst;
	const struct authmethod_pam_t *auth;
	char *emsg;
	size_t emsgsize;
{
	const char *function = "pam_passwordcheck()"; 
	static pam_handle_t *pamh;
	static char lastservicename[sizeof(auth->servicename)];
	_pam_data_t pw;
	int rc;
	uid_t	euid;
	struct pam_conv _pam_conv = {
		&_pam_conversation,
		NULL
	};

#ifdef HAVE_SOLARIS_PAM_BUG
	_pam_priv_data = NULL;
#endif /* HAVE_SOLARIS_PAM_BUG */

	slog(LOG_DEBUG, function);

	/* if we get a different servicename we have to restart pam. */
	if (strcmp(auth->servicename, lastservicename) != 0)
		if (pamh != NULL) {
			pam_end(pamh, 0);
			pamh = NULL;
		}


	socks_seteuid(&euid, socksconfig.uid.privileged); 

	if (pamh == NULL) {
		if ((rc = pam_start(*auth->servicename == NUL ?
		DEFAULT_PAMSERVICENAME : auth->servicename, NULL, &_pam_conv, &pamh))
		!= PAM_SUCCESS) {
			snprintf(emsg, emsgsize, "unable to obtain PAM authenticator: %s",
			pam_strerror(pamh, rc));
			pam_end(pamh, rc);
			socks_reseteuid(socksconfig.uid.privileged, euid);
			pamh = NULL;
			return -1;
		}

		SASSERTX(strlen(auth->servicename) < sizeof(lastservicename));
		strcpy(lastservicename, auth->servicename);
	}

	if ((rc = pam_set_item(pamh, PAM_USER, auth->name)) != PAM_SUCCESS) {
		socks_reseteuid(socksconfig.uid.privileged, euid);
		snprintf(emsg, emsgsize, "failed to set PAM_USER: %s",
		pam_strerror(pamh, rc));
		return -1;
	}

	pw.user 		= auth->name;
	pw.password = auth->password;
	_pam_conv.appdata_ptr = (char *)&pw;
#ifdef HAVE_SOLARIS_PAM_BUG
	_pam_priv_data = &pw;
#endif /* HAVE_SOLARIS_PAM_BUG */

	if ((rc = pam_set_item(pamh, PAM_CONV, &_pam_conv)) != PAM_SUCCESS) {
		socks_reseteuid(socksconfig.uid.privileged, euid);
		snprintf(emsg, emsgsize, "failed to set PAM_CONV: %s",
		pam_strerror(pamh, rc));
		return -1;
	}

	if ((rc = pam_set_item(pamh, PAM_RHOST, inet_ntoa(TOCIN(src)->sin_addr)))
	!= PAM_SUCCESS)
		swarnx("failed to set PAM_RHOST: %s", pam_strerror(pamh, rc));
		
	switch (rc = pam_authenticate(pamh, 0)) {
		case PAM_SUCCESS:
			break;

		default:
			socks_reseteuid(socksconfig.uid.privileged, euid);
			snprintf(emsg, emsgsize, "pam %s", pam_strerror(pamh, rc));
			return -1;
	}
	
	switch (rc = pam_acct_mgmt(pamh, 0)) {
		case PAM_SUCCESS:
			break;

		default:
			socks_reseteuid(socksconfig.uid.privileged, euid);
			snprintf(emsg, emsgsize, "pam %s", pam_strerror(pamh, rc));
			return -1;
	}

	socks_reseteuid(socksconfig.uid.privileged, euid);
	return 0;
}

static int
_pam_conversation(num_msg, msgs, rsps, priv_data)
	int num_msg;
	const struct pam_message **msgs;
	struct pam_response **rsps;
	void * priv_data;
{
	_pam_data_t *pw = (_pam_data_t *)priv_data;
	struct pam_response *rsp;
	int i;

#ifdef HAVE_SOLARIS_PAM_BUG
	pw = _pam_priv_data;
#endif /* HAVE_SOLARIS_PAM_BUG */

	if (!rsps || !msgs || num_msg <= 0)
		return PAM_CONV_ERR;

	*rsps = NULL;

	if (!pw)
		return PAM_CONV_ERR;

	if ((rsp = malloc(num_msg * sizeof(struct pam_response))) == NULL)
		return PAM_CONV_ERR;
	bzero(rsp, num_msg * sizeof(struct pam_response));

	for (i = 0; i < num_msg; i++) {
		rsp[i].resp_retcode = 0;
		rsp[i].resp = NULL;

		switch(msgs[i]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				rsp[i].resp = strdup(pw->user);
				break;

			case PAM_PROMPT_ECHO_OFF:
				rsp[i].resp = strdup(pw->password);
				break;

			default:
				free(rsp);
				return PAM_CONV_ERR;
		}
	}

	*rsps = rsp;
	return PAM_SUCCESS;
}

#endif /* HAVE_PAM */
