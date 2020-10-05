// INCLUDES
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <libmilter/mfapi.h>
#include <syslog.h>
#include <curl/curl.h>

// BOOLEAN SUPPORT
#define bool short
#define false 0
#define true 1

// CONFIG
#define MAX_POST_BUFFER 512
#define MAX_CONF_LINE_LENGTH 256

#define PAYLOAD_CONFIG_PATH "/etc/milter-notify/ping_data"
const char *DEFAULT_PAYLOAD = "NEW_MAIL_ARRIVED";
#define PING_HOST_CONFIG_PATH "/etc/milter-notify/ping_url"
const char *DEFAULT_PING_HOST = "0.0.0.0";

char payload[MAX_CONF_LINE_LENGTH];
char ping_host[MAX_CONF_LINE_LENGTH];

char postBuffer[MAX_POST_BUFFER];

bool read_config_value(char *filepath, char *buffer, const char *default_value)
{
		FILE *fp;
		fp = fopen(filepath, "r");

		if (fp == NULL)
		{
				printf("ERROR (using default)\n");
				strcpy(buffer, default_value);
				return false;
		}

		printf("SUCCESS\n");
		fgets(buffer, MAX_CONF_LINE_LENGTH, fp);

		// remove \n from end of line
		if (strlen(buffer)) {
				buffer[strlen(buffer) - 1] = 0;
		}

		fclose(fp);
		return true;
}

// libmilter
static const char *macros[] = {
		"i",
		"j",
		"_",
		"{auth_authen}",
		"{auth_author}",
		"{auth_type}",
		"{client_addr}",
		"{client_connections}",
		"{client_name}",
		"{client_port}",
		"{client_ptr}",
		"{cert_issuer}",
		"{cert_subject}",
		"{cipher_bits}",
		"{cipher}",
		"{daemon_name}",
		"{mail_addr}",
		"{mail_host}",
		"{mail_mailer}",
		"{rcpt_addr}",
		"{rcpt_host}",
		"{rcpt_mailer}",
		"{tls_version}",
		"v",
		NULL};

char *get_macro(SMFICTX *ctx, int i)
{
		// function returns value of sendmail macro
		// in current postfix context. basically, gives data
		// about an email.
		// If it can't read data, returns NULL instead.

		char *symval;

		if (macros[i])
		{
				if ((symval = smfi_getsymval(ctx, (char *)macros[i])) != 0)
				{
						return symval;
				}
		}

		symval = NULL;
		return symval;
}

bool is_incoming(SMFICTX *ctx) {
		// function checks if mail
		// that triggered milter is
		// incoming or outgoing one
		printf("%s = %s\n", macros[18], get_macro(ctx, 18));
		char *mailer_mailer = get_macro(ctx, 18);
		if (strcmp(mailer_mailer, "local")) {
				return true;
		}

		printf("%s = %s\n", macros[17], get_macro(ctx, 17));
		char *mailer_host = get_macro(ctx, 17);

		printf("%s = %s\n", macros[20], get_macro(ctx, 20));
		char *rcpt_host = get_macro(ctx, 20);
		if (!strcmp(mailer_host, rcpt_host)) {
				return true;
		}

		return false;
}

extern sfsistat testmilter_cleanup(SMFICTX *, bool);

// this part is from testmilter
// (https://github.com/mephi-ut/testmilter/blob/master/main.c)
// it negotiates what we can do with message, altough we just
// want to read some basic data (or even not), we don't need to worry
// about it too much.
sfsistat testmilter_negotiate(ctx, f0, f1, f2, f3, pf0, pf1, pf2, pf3)
		SMFICTX *ctx;
		unsigned long f0;
		unsigned long f1;
		unsigned long f2;
		unsigned long f3;
		unsigned long *pf0;
		unsigned long *pf1;
		unsigned long *pf2;
		unsigned long *pf3;
{
#if 0
		*pf0 |=  SMFIF_ADDHDRS | SMFIF_CHGHDRS | SMFIF_CHGBODY | SMFIF_ADDRCPT |
				SMFIF_ADDRCPT_PAR | SMFIF_DELRCPT | SMFIF_QUARANTINE | 
				SMFIF_CHGFROM | SMFIF_SETSYMLIST;

		*pf1 |= 	SMFIP_RCPT_REJ | SMFIP_SKIP | SMFIP_NR_CONN | SMFIP_NR_HELO | 
				SMFIP_NR_MAIL | SMFIP_NR_RCPT | SMFIP_NR_DATA | SMFIP_NR_UNKN | 
				SMFIP_NR_EOH | SMFIP_NR_BODY | SMFIP_NR_HDR;

		//	*pf0 = f0;
		//	*pf1 = f1;

		*pf2 = 0;
		*pf3 = 0;

		return SMFIS_CONTINUE;
#else
		*pf0 = f0;
		*pf1 = SMFIP_RCPT_REJ;
		return SMFIS_CONTINUE;
#endif
}

// print usage instruction in console
static void usage(const char *path)
{
		fprintf(stderr, "Usage: %s -p socket-addr [-t timeout]\n",
						path);
}

// dummy filters,
// do nothing, and continue working
sfsistat testmilter_connect(SMFICTX *ctx, char *hostname, _SOCK_ADDR *hostaddr)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_helo(SMFICTX *ctx, char *helohost)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_envfrom(SMFICTX *ctx, char **argv)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_envrcpt(SMFICTX *ctx, char **argv)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_header(SMFICTX *ctx, char *headerf, char *headerv)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_eoh(SMFICTX *ctx)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_body(SMFICTX *ctx, unsigned char *bodyp, size_t bodylen)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_eom(SMFICTX *ctx)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_abort(SMFICTX *ctx)
{
		return SMFIS_CONTINUE;
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
		// helper function to not print
		// request body
		return size * nmemb;
}

void send_notification(char *rcpt, char *mailer) {
		//	printf("\tURL: %s\n\tPAYLOAD: %s\n", ping_host, payload);
		printf("\t\tsending request...");
		CURL *postReq = curl_easy_init();

		if (postReq) {
				struct curl_slist *postHead = NULL;
				postHead = curl_slist_append(postHead, "Content-Type: application/json");
				if (postHead) {		
						curl_easy_setopt(postReq, CURLOPT_HTTPHEADER, postHead);
				}

				curl_easy_setopt(postReq, CURLOPT_URL, ping_host);
				curl_easy_setopt(postReq, CURLOPT_WRITEFUNCTION, write_data);


				snprintf(postBuffer, sizeof postBuffer, "{\"PAYLOAD\": \"%s\", \"SENDER\": \"%s\", \"RECEIVER\": \"%s\"}", payload, mailer, rcpt);
				curl_easy_setopt(postReq, CURLOPT_POSTFIELDS, postBuffer);
				curl_easy_perform(postReq);

				memset(postBuffer, 0, sizeof postBuffer);
				printf("SUCCESS\n");
		} else {
				printf("ERROR (can't initialize CURL)\n");
		}

		curl_easy_cleanup(postReq);
}

sfsistat testmilter_close(SMFICTX *ctx)
{
		if (is_incoming(ctx))
		{
				printf("\tGOT NEW INCOMING MAIL\n");
				printf("\tSENDING NOTIFICATION\n");
				send_notification(get_macro(ctx, 19), get_macro(ctx, 16));
		}
		else
		{
				printf("\tDETECTED OUTCOMING MAIL\n");
		}

		return SMFIS_CONTINUE;
}

sfsistat testmilter_unknown(SMFICTX *ctx, const char *cmd)
{
		return SMFIS_CONTINUE;
}

sfsistat testmilter_data(SMFICTX *ctx)
{
		return SMFIS_CONTINUE;
}

int main(int argc, char *argv[])
{
		printf("[STARTING] milter-notify is starting\n");
		// READING CONFIG
		printf("[STARTING] reading config files\n");
		printf("\treading ping host config...");
		read_config_value(PING_HOST_CONFIG_PATH, ping_host, DEFAULT_PING_HOST);
		printf("\treading payload config...");
		read_config_value(PAYLOAD_CONFIG_PATH, payload, DEFAULT_PAYLOAD);

		// init curl library
		curl_global_init(0);

		// setting up milter connection
		struct smfiDesc mailfilterdesc = {
				"milter-notify",               // filter name
				SMFI_VERSION,                  // version code -- do not change
				SMFIF_ADDHDRS | SMFIF_ADDRCPT, // flags
				testmilter_connect,            // connection info filter
				testmilter_helo,               // SMTP HELO command filter
				testmilter_envfrom,            // envelope sender filter
				testmilter_envrcpt,            // envelope recipient filter
				testmilter_header,             // header filter
				testmilter_eoh,                // end of header
				testmilter_body,               // body block filter
				testmilter_eom,                // end of message
				testmilter_abort,              // message aborted
				testmilter_close,              // connection cleanup
				testmilter_unknown,            // unknown SMTP commands
				testmilter_data,               // DATA command
				testmilter_negotiate           // Once, at the start of each SMTP connection
		};

		char setconn = 0;
		int c;
		const char *args = "p:t:h";
		extern char *optarg;
		// Process command line options
		while ((c = getopt(argc, argv, args)) != -1)
		{
				switch (c)
				{
						case 'p':
								if (optarg == NULL || *optarg == '\0')
								{
										(void)fprintf(stderr, "Illegal conn: %s\n",
														optarg);
										exit(EX_USAGE);
								}
								if (smfi_setconn(optarg) == MI_FAILURE)
								{
										(void)fprintf(stderr,
														"smfi_setconn failed\n");
										exit(EX_SOFTWARE);
								}

								if (strncasecmp(optarg, "unix:", 5) == 0)
										unlink(optarg + 5);
								else if (strncasecmp(optarg, "local:", 6) == 0)
										unlink(optarg + 6);
								setconn = 1;
								break;
						case 't':
								if (optarg == NULL || *optarg == '\0')
								{
										(void)fprintf(stderr, "Illegal timeout: %s\n",
														optarg);
										exit(EX_USAGE);
								}
								if (smfi_settimeout(atoi(optarg)) == MI_FAILURE)
								{
										(void)fprintf(stderr,
														"smfi_settimeout failed\n");
										exit(EX_SOFTWARE);
								}
								break;
						case 'h':
						default:
								usage(argv[0]);
								exit(EX_USAGE);
				}
		}
		if (!setconn)
		{
				fprintf(stderr, "%s: Missing required -p argument\n", argv[0]);
				usage(argv[0]);
				exit(EX_USAGE);
		}
		if (smfi_register(mailfilterdesc) == MI_FAILURE)
		{
				fprintf(stderr, "smfi_register failed\n");
				exit(EX_UNAVAILABLE);
		}
		openlog(NULL, LOG_PID, LOG_MAIL);
		int ret = smfi_main();
		closelog();
		curl_global_cleanup();

		// QUITING
		printf("[QUITING] milter-notify is quitting\n");
		return ret;
}
