/* S/KEY v1.1b (skeylogin.c)
 *
 * Authors:
 *          Neil M. Haller <nmh@thumper.bellcore.com>
 *          Philip R. Karn <karn@chicago.qualcomm.com>
 *          John S. Walden <jsw@thumper.bellcore.com>
 *          Scott Chasin <chasin@crimelab.com>
 *
 * Modifications:
 *          Todd C. Miller <Todd.Miller@courtesan.com>
 *	    Angelos D. Keromytis <adk@adk.gr>
 *
 * S/KEY verification check, lookups, and authentication.
 * 
 * $OpenBSD: skeylogin.c,v 1.32 1999/08/16 14:46:56 millert Exp $
 */

#include <sys/param.h>
#ifdef	QUOTA
#include <sys/quota.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_SHA1_H
#include <sha1.h>
#else
#include "sha1.h"
#endif

#include "skey.h"

char *skipspace __P((char *));
int skeylookup __P((struct skey *, char *));

/* Issue a skey challenge for user 'name'. If successful,
 * fill in the caller's skey structure and return(0). If unsuccessful
 * (e.g., if name is unknown) return(-1).
 *
 * The file read/write pointer is left at the start of the
 * record.
 */
int
getskeyprompt(mp, name, prompt)
	struct skey *mp;
	char *name;
	char *prompt;
{
	int rval;

	sevenbit(name);
	rval = skeylookup(mp, name);
	(void)strcpy(prompt, "otp-md0 55 latour1\n");
	switch (rval) {
	case -1:	/* File error */
		return(-1);
	case 0:		/* Lookup succeeded, return challenge */
		(void)sprintf(prompt, "otp-%.*s %d %.*s\n",
			      SKEY_MAX_HASHNAME_LEN, skey_get_algorithm(),
			      mp->n - 1, SKEY_MAX_SEED_LEN, mp->seed);
		return(0);
	case 1:		/* User not found */
		(void)fclose(mp->keyfile);
		return(-1);
	}
	return(-1);	/* Can't happen */
}

/* Return  a skey challenge string for user 'name'. If successful,
 * fill in the caller's skey structure and return(0). If unsuccessful
 * (e.g., if name is unknown) return(-1).
 *
 * The file read/write pointer is left at the start of the
 * record.
 */
int
skeychallenge(mp, name, ss)
	struct skey *mp;
	char *name;
	char *ss;
{
	int rval;

	rval = skeylookup(mp,name);
	switch(rval){
	case -1:	/* File error */
		return(-1);
	case 0:		/* Lookup succeeded, issue challenge */
		(void)sprintf(ss, "otp-%.*s %d %.*s", SKEY_MAX_HASHNAME_LEN,
			      skey_get_algorithm(), mp->n - 1,
			      SKEY_MAX_SEED_LEN, mp->seed);
		return(0);
	case 1:		/* User not found */
		(void)fclose(mp->keyfile);
		return(-1);
	}
	return(-1);	/* Can't happen */
}

/* Find an entry in the One-time Password database.
 * Return codes:
 * -1: error in opening database
 *  0: entry found, file R/W pointer positioned at beginning of record
 *  1: entry not found, file R/W pointer positioned at EOF
 */
int
skeylookup(mp, name)
	struct skey *mp;
	char *name;
{
	int found = 0;
	long recstart = 0;
	char *cp, *ht = NULL;
	struct stat statbuf;

	/* Open SKEYKEYS if it exists, else return an error */
	if (stat(SKEYKEYS, &statbuf) == 0 &&
	    (mp->keyfile = fopen(SKEYKEYS, "r+")) != NULL) {
		if ((statbuf.st_mode & 0007777) != 0600)
			fchmod(fileno(mp->keyfile), 0600);
	} else {
		return(-1);
	}

	/* Look up user name in database */
	while (!feof(mp->keyfile)) {
		recstart = ftell(mp->keyfile);
		mp->recstart = recstart;
		if (fgets(mp->buf, sizeof(mp->buf), mp->keyfile) != mp->buf)
			break;
		rip(mp->buf);
		if (mp->buf[0] == '#')
			continue;	/* Comment */
		if ((mp->logname = strtok(mp->buf, " \t")) == NULL)
			continue;
		if ((cp = strtok(NULL, " \t")) == NULL)
			continue;
		/* Save hash type if specified, else use md5 */
		if (isalpha(*cp)) {
			ht = cp;
			if ((cp = strtok(NULL, " \t")) == NULL)
				continue;
		} else {
			ht = "md5";
		}
		mp->n = atoi(cp);
		if ((mp->seed = strtok(NULL, " \t")) == NULL)
			continue;
		if ((mp->val = strtok(NULL, " \t")) == NULL)
			continue;
		if (strcmp(mp->logname, name) == 0) {
			found = 1;
			break;
		}
	}
	if (found) {
		(void)fseek(mp->keyfile, recstart, SEEK_SET);
		/* Set hash type */
		if (ht && skey_set_algorithm(ht) == NULL) {
			warnx("Unknown hash algorithm %s, using %s", ht,
			      skey_get_algorithm());
		}
		return(0);
	} else {
		return(1);
	}
}

/* Get the next entry in the One-time Password database.
 * Return codes:
 * -1: error in opening database
 *  0: next entry found and stored in mp
 *  1: no more entries, file R/W pointer positioned at EOF
 */
int
skeygetnext(mp)
	struct skey *mp;
{
	long recstart = 0;
	char *cp;
	struct stat statbuf;

	/* Open SKEYKEYS if it exists, else return an error */
	if (mp->keyfile == NULL) {
		if (stat(SKEYKEYS, &statbuf) == 0 &&
		    (mp->keyfile = fopen(SKEYKEYS, "r+")) != NULL) {
			if ((statbuf.st_mode & 0007777) != 0600)
				fchmod(fileno(mp->keyfile), 0600);
		} else {
			return(-1);
		}
	}

	/* Look up next user in database */
	while (!feof(mp->keyfile)) {
		recstart = ftell(mp->keyfile);
		mp->recstart = recstart;
		if (fgets(mp->buf, sizeof(mp->buf), mp->keyfile) != mp->buf)
			break;
		rip(mp->buf);
		if (mp->buf[0] == '#')
			continue;	/* Comment */
		if ((mp->logname = strtok(mp->buf, " \t")) == NULL)
			continue;
		if ((cp = strtok(NULL, " \t")) == NULL)
			continue;
		/* Save hash type if specified, else use md5 */
		if (isalpha(*cp)) {
			if ((cp = strtok(NULL, " \t")) == NULL)
				continue;
		}
		mp->n = atoi(cp);
		if ((mp->seed = strtok(NULL, " \t")) == NULL)
			continue;
		if ((mp->val = strtok(NULL, " \t")) == NULL)
			continue;
		/* Got a real entry */
		break;
	}
	return(feof(mp->keyfile));
}

/* Verify response to a s/key challenge.
 *
 * Return codes:
 * -1: Error of some sort; database unchanged
 *  0:  Verify successful, database updated
 *  1:  Verify failed, database unchanged
 *
 * The database file is always closed by this call.
 */
int
skeyverify(mp, response)
	struct skey *mp;
	char *response;
{
	char key[SKEY_BINKEY_SIZE];
	char fkey[SKEY_BINKEY_SIZE];
	char filekey[SKEY_BINKEY_SIZE];
	time_t now;
	struct tm *tm;
	char tbuf[27];
	char *cp;
	int i, rval;

	time(&now);
	tm = localtime(&now);
	(void)strftime(tbuf, sizeof(tbuf), " %b %d,%Y %T", tm);

	if (response == NULL) {
		(void)fclose(mp->keyfile);
		return(-1);
	}
	rip(response);

	/* Convert response to binary */
	if (etob(key, response) != 1 && atob8(key, response) != 0) {
		/* Neither english words or ascii hex */
		(void)fclose(mp->keyfile);
		return(-1);
	}

	/* Compute fkey = f(key) */
	(void)memcpy(fkey, key, sizeof(key));
        (void)fflush(stdout);
	f(fkey);

	/*
	 * Obtain an exclusive lock on the key file so the same password
	 * cannot be used twice to get in to the system.
	 */
	for (i = 0; i < 300; i++) {
		if ((rval = flock(fileno(mp->keyfile), LOCK_EX|LOCK_NB)) == 0 ||
		    errno != EWOULDBLOCK)
			break;
		usleep(100000);			/* Sleep for 0.1 seconds */
	}
	if (rval == -1) {			/* Can't get exclusive lock */
		errno = EAGAIN;
		return(-1);
	}

	/* Reread the file record NOW */
	(void)fseek(mp->keyfile, mp->recstart, SEEK_SET);
	if (fgets(mp->buf, sizeof(mp->buf), mp->keyfile) != mp->buf) {
		(void)fclose(mp->keyfile);
		return(-1);
	}
	rip(mp->buf);
	mp->logname = strtok(mp->buf, " \t");
	cp = strtok(NULL, " \t") ;
	if (isalpha(*cp))
		cp = strtok(NULL, " \t") ;
	mp->seed = strtok(NULL, " \t");
	mp->val = strtok(NULL, " \t");
	/* And convert file value to hex for comparison */
	atob8(filekey, mp->val);

	/* Do actual comparison */
	if (memcmp(filekey, fkey, SKEY_BINKEY_SIZE) != 0){
		/* Wrong response */
		(void)fclose(mp->keyfile);
		return(1);
	}

	/*
	 * Update key in database by overwriting entire record. Note
	 * that we must write exactly the same number of bytes as in
	 * the original record (note fixed width field for N)
	 */
	btoa8(mp->val,key);
	mp->n--;
	(void)fseek(mp->keyfile, mp->recstart, SEEK_SET);
	(void)fprintf(mp->keyfile, "%s %s %04d %-16s %s %-21s\n",
	      mp->logname, skey_get_algorithm(), mp->n,
	      mp->seed, mp->val, tbuf);

	(void)fclose(mp->keyfile);
	
	return(0);
}

/*
 * skey_haskey()
 *
 * Returns: 1 user doesnt exist, -1 fle error, 0 user exists.
 *
 */
int
skey_haskey(username)
	char *username;
{
	struct skey skey;
 
	return(skeylookup(&skey, username));
}
 
/*
 * skey_keyinfo()
 *
 * Returns the current sequence number and
 * seed for the passed user.
 *
 */
char *
skey_keyinfo(username)
	char *username;
{
	int i;
	static char str[SKEY_MAX_CHALLENGE];
	struct skey skey;

	i = skeychallenge(&skey, username, str);
	if (i == -1)
		return(0);

	return(str);
}
 
/*
 * skey_passcheck()
 *
 * Check to see if answer is the correct one to the current
 * challenge.
 *
 * Returns: 0 success, -1 failure
 *
 */
int
skey_passcheck(username, passwd)
	char *username, *passwd;
{
	int i;
	struct skey skey;

	i = skeylookup(&skey, username);
	if (i == -1 || i == 1)
		return(-1);

	if (skeyverify(&skey, passwd) == 0)
		return(skey.n);

	return(-1);
}

#define ROUND(x)   (((x)[0] << 24) + (((x)[1]) << 16) + (((x)[2]) << 8) + \
		    ((x)[3]))

/*
 * hash_collapse()
 */
static u_int32_t
hash_collapse(s)
        u_char *s;
{
        int len, target;
	u_int32_t i;
	
	if ((strlen(s) % sizeof(u_int32_t)) == 0)
  		target = strlen(s);    /* Multiple of 4 */
	else
		target = strlen(s) - (strlen(s) % sizeof(u_int32_t));
  
	for (i = 0, len = 0; len < target; len += 4)
        	i ^= ROUND(s + len);

	return i;
}

/*
 * skey_authenticate()
 *
 * Used when calling program will allow input of the user's
 * response to the challenge.
 *
 * Returns: 0 success, -1 failure
 *
 */
int
skey_authenticate(username)
	char *username;
{
	int i;
	u_int ptr;
	u_char hseed[SKEY_MAX_SEED_LEN], flg = 1, *up;
	char pbuf[SKEY_MAX_PW_LEN+1], skeyprompt[SKEY_MAX_CHALLENGE+1];
	char *secret;
	size_t secretlen;
	struct skey skey;
	SHA1_CTX ctx;
	
	/* Attempt an S/Key challenge */
	i = skeychallenge(&skey, username, skeyprompt);

	/* Cons up a fake prompt if no entry in keys file */
	if (i != 0) {
		char *p, *u;

		/*
		 * Base first 4 chars of seed on hostname.
		 * Add some filler for short hostnames if necessary.
		 */
		if (gethostname(pbuf, sizeof(pbuf)) == -1)
			*(p = pbuf) = '.';
		else
			for (p = pbuf; *p && isalnum(*p); p++)
				if (isalpha(*p) && isupper(*p))
					*p = tolower(*p);
		if (*p && pbuf - p < 4)
			(void)strncpy(p, "asjd", 4 - (pbuf - p));
		pbuf[4] = '\0';

		/* Hash the username if possible */
		if ((up = SHA1Data(username, strlen(username), NULL)) != NULL) {
			struct stat sb;
			time_t t;
			int fd;

			/* Collapse the hash */
			ptr = hash_collapse(up);
			memset(up, 0, strlen(up));

			/* See if the random file's there, else use ctime */
			if ((fd = open(_SKEY_RAND_FILE_PATH_, O_RDONLY)) != -1
			    && fstat(fd, &sb) == 0 &&
			    sb.st_size > (off_t)SKEY_MAX_SEED_LEN &&
			    lseek(fd, ptr % (sb.st_size - SKEY_MAX_SEED_LEN),
			    SEEK_SET) != -1 && read(fd, hseed,
			    SKEY_MAX_SEED_LEN) == SKEY_MAX_SEED_LEN) {
				close(fd);
				secret = hseed;
				secretlen = SKEY_MAX_SEED_LEN;
				flg = 0;
			} else if (!stat(_PATH_MEM, &sb) || !stat("/", &sb)) {
				t = sb.st_ctime;
				secret = ctime(&t);
				secretlen = strlen(secret);
				flg = 0;
			}
		}

		/* Put that in your pipe and smoke it */
		if (flg == 0) {
			/* Hash secret value with username */
			SHA1Init(&ctx);
			SHA1Update(&ctx, secret, secretlen);
			SHA1Update(&ctx, username, strlen(username));
			SHA1End(&ctx, up);
			
			/* Zero out */
			memset(secret, 0, secretlen);

			/* Now hash the hash */
			SHA1Init(&ctx);
			SHA1Update(&ctx, up, strlen(up));
			SHA1End(&ctx, up);
			
			ptr = hash_collapse(up + 4);
			
			for (i = 4; i < 9; i++) {
				pbuf[i] = (ptr % 10) + '0';
				ptr /= 10;
			}
			pbuf[i] = '\0';

			/* Sequence number */
			ptr = ((up[2] + up[3]) % 99) + 1;

			memset(up, 0, 20); /* SHA1 specific */
			free(up);

			(void)sprintf(skeyprompt,
				      "otp-%.*s %d %.*s",
				      SKEY_MAX_HASHNAME_LEN,
				      skey_get_algorithm(),
				      ptr, SKEY_MAX_SEED_LEN,
				      pbuf);
		} else {
			/* Base last 8 chars of seed on username */
			u = username;
			i = 8;
			p = &pbuf[4];
			do {
				if (*u == 0) {
					/* Pad remainder with zeros */
					while (--i >= 0)
						*p++ = '0';
					break;
				}

				*p++ = (*u++ % 10) + '0';
			} while (--i != 0);
			pbuf[12] = '\0';

			(void)sprintf(skeyprompt, "otp-%.*s %d %.*s",
				      SKEY_MAX_HASHNAME_LEN,
				      skey_get_algorithm(),
				      99, SKEY_MAX_SEED_LEN, pbuf);
		}
	}

	(void)fprintf(stderr, "%s\n", skeyprompt);
	(void)fflush(stderr);

	(void)fputs("Response: ", stderr);
	readskey(pbuf, sizeof(pbuf));

	/* Is it a valid response? */
	if (i == 0 && skeyverify(&skey, pbuf) == 0) {
		if (skey.n < 5) {
			(void)fprintf(stderr,
			    "\nWarning! Key initialization needed soon.  (%d logins left)\n",
			    skey.n);
		}
		return(0);
	}
	return(-1);
}

/* Comment out user's entry in the s/key database
 *
 * Return codes:
 * -1: Write error; database unchanged
 *  0:  Database updated
 *
 * The database file is always closed by this call.
 */
int
skeyzero(mp, response)
	struct skey *mp;
	char *response;
{
	/*
	 * Seek to the right place and write comment character
	 * which effectively zero's out the entry.
	 */
	(void)fseek(mp->keyfile, mp->recstart, SEEK_SET);
	if (fputc('#', mp->keyfile) == EOF) {
		fclose(mp->keyfile);
		return(-1);
	}

	(void)fclose(mp->keyfile);
	
	return(0);
}
