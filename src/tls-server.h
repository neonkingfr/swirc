#ifndef TLS_SERVER_H
#define TLS_SERVER_H

#include <openssl/ssl.h>

#include <stdint.h> /* uintptr_t */

#define ROOT_PEM	"root.pem"
#define SERVER_CA_PEM	"serverCA.pem"
#define SERVER_PEM	"server.pem"
#define CLIENT_PEM	"client.pem"

#define DH_PEM1 "dh2048.pem"
#define DH_PEM2 "dh4096.pem"

#define SERVER_MIN_PORT 1024
#define SERVER_MAX_PORT 65535

__SWIRC_BEGIN_DECLS
extern const char	g_suite_secure[];
extern const char	g_suite_compat[];
extern const char	g_suite_legacy[];
extern const char	g_suite_all[];

extern volatile bool	g_accepting_new_connections;
extern volatile bool	g_tls_server_loop;

/*lint -sem(tls_server_get_accept_bio, r_null) */
/*lint -sem(tls_server_setup_context, r_null) */

void	 tls_server_accept_new_connections(const int);
void	 tls_server_enter_loop(SSL *);
BIO	*tls_server_get_accept_bio(const int);
SSL_CTX	*tls_server_setup_context(void);

/*lint -sem(tls_server_exit_thread, r_no) */

#ifdef WIN32
extern const uintptr_t g_beginthread_failed;
#endif

void		 tls_server_begin(const int);
void		 tls_server_end(void);
void		 tls_server_com_with_client(SSL *);
NORETURN void	 tls_server_exit_thread(void);
__SWIRC_END_DECLS

#endif
