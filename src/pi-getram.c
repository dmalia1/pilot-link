/* pi-getram:  Fetch RAM image from Pilot
 *
 * This is free software, licensed under the GNU Public License V2.
 * See the file COPYING for details.
 *
 * Copyright (C) 1997, Kenneth Albanowski
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "pi-source.h"
#include "pi-syspkt.h"
#include "pi-dlp.h"

#include <termios.h>

char * progname;

void Help(void)
{
  fprintf(stderr,"usage: %s %s [-c] [pilot.ram]\n",progname,TTYPrompt);
  fprintf(stderr,"\nUse the -c flag if the RAM file will be used with Copilot.\n");
  exit(2);
}

int cancel = 0;
RETSIGTYPE sighandler(int signo)
{
  cancel = 1;
}

struct record * records = 0;

int main(int argc, char *argv[])
{
  char name[256];
  char print[256];
  int i, sd;
  struct RPC_params p;
  struct pi_sockaddr addr;
  extern char* optarg;
  extern int optind;
  /*int err;*/
  int ret;
  int file;
  int j;
  char * port, *filename;
  int copilot;
  unsigned long SRAMstart, SRAMlength, ROMversion, offset, left;
  int majorVersion, minorVersion, build, state;

  progname = argv[0];

  if (argc < 2)
    Help();
  
  if (!(sd = pi_socket(PI_AF_SLP, PI_SOCK_STREAM, PI_PF_PADP))) {
    perror("pi_socket");
    exit(1);
  }
  
  port = argv[1];
  
  if (argc>2) {
	if (!strcmp(argv[2], "-c")) {
		copilot=1;
	  	if (argc > 3)
			filename = argv[3];
		else
			filename = 0;
	} else {
	  	copilot=0;
	  	if (argc> 2)
			filename = argv[2];
		else
			filename = 0;
	}
  } else {
  	copilot = 0;
  	filename = 0;
  }
    
  addr.pi_family = PI_AF_SLP;
  strcpy(addr.pi_device,port);
  
  ret = pi_bind(sd, (struct sockaddr*)&addr, sizeof(addr));
  if(ret == -1) {
    perror("pi_bind");
    exit(1);
  }

  ret = pi_listen(sd,1);
  if(ret == -1) {
    perror("pi_listen");
    exit(1);
  }
  
  printf("Warning: Please back up your Pilot (with pilot-xfer) before using this program!\n\n");

  printf("Please start HotSync (not getrom.prc) on your Pilot.\n");
  printf("Waiting for connection on %s...\n", port);

  sd = pi_accept(sd, 0, 0);
  if(sd == -1) {
    perror("pi_accept");
    exit(1);
  }

  /* Tell user (via Pilot) that we are starting things up */
  dlp_OpenConduit(sd);
  
  PackRPC(&p, 0xA23D, RPC_IntReply, RPC_Long(0xFFFFFFFE), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, &SRAMstart);
  PackRPC(&p, 0xA23D, RPC_IntReply, RPC_Long(SRAMstart), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, &SRAMlength);
  
#if 0
  printf("dramstart = %lu (%8.8lX), length = %lu (%8.8lX), end = %lu (%8.8lX)\n",
  	SRAMstart, SRAMstart, SRAMlength, SRAMlength, SRAMstart+SRAMlength, SRAMstart+SRAMlength);
#endif
  
  dlp_ReadFeature(sd, makelong("psys"), 1, &ROMversion);
  
  if (!filename)
    strcpy(name, "pilot.ram");
  else
    strcpy(name, filename);
  
  majorVersion = (((ROMversion >> 28) & 0xf) * 10)+ ((ROMversion >> 24) & 0xf);
  minorVersion = (((ROMversion >> 20) & 0xf) * 10)+ ((ROMversion >> 16) & 0xf);
  state = ((ROMversion >> 12) & 0xf);
  build = (((ROMversion >> 8) & 0xf) * 10)+(((ROMversion >> 4) & 0xf) * 10)+ (ROMversion  & 0xf);
  
  sprintf(name+strlen(name), "%d.%d", majorVersion, minorVersion);
  if (state!=3)
    sprintf(name+strlen(name), "%s%d", ((state==0) ? "d" : (state == 1) ? "a" : (state==2) ? "b" : "u"), build);
  sprintf(name+strlen(name), ".%lu", SRAMlength/1024);
  
  printf("Generating %s\n", name);
  
  file = open(name,O_RDWR|O_CREAT,0666);

  offset = lseek(file, 0, SEEK_END);
  offset &= ~255;
  lseek(file, offset, SEEK_SET);

  PackRPC(&p, 0xA164, RPC_IntReply, RPC_Byte(1), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, 0);

  sprintf(print, "Downloading byte %ld", offset);
  PackRPC(&p, 0xA220, RPC_IntReply, RPC_Ptr(print, strlen(print)), RPC_Short(strlen(print)), RPC_Short(0), RPC_Short(28), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, 0);

#if 0
  PackRPC(&p, 0xA026, RPC_IntReply, RPC_LongPtr(&penPtr), RPC_Long(364), RPC_Long(4), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, 0);
  
  printf("penPtr = %lu (%8.8lX)\n", penPtr, penPtr);

  PackRPC(&p, 0xA026, RPC_IntReply, RPC_Ptr(print,8), RPC_Long(penPtr), RPC_Long(8), RPC_End);
  /*err =*/  dlp_RPC(sd, &p, 0);
  dumpdata(print, 8);
#endif

  signal(SIGINT, sighandler);
  left = SRAMlength - offset;
  i=offset;
  while(left>0) {
  	char buffer[256];
  	int len = left;
  	if (len>256)
  		len=256;
  	printf("\r%ld of %ld bytes", offset, SRAMlength);
  	fflush(stdout);
  	PackRPC(&p, 0xA026, RPC_IntReply, RPC_Ptr(buffer, len), RPC_Long(offset+SRAMstart), RPC_Long(len), RPC_End);
  	/*err =*/  dlp_RPC(sd, &p, 0);
  	left -= len;
  	
  	if (copilot)
  	  for(j=0;j<len;j+=2)
  	    *(short int*)(buffer+j) = get_short(buffer+j);
  	
  	/* If the buffer only contains zeros, skip instead of writing, so
  	   that the file will be holey. */
  	for(j=0;j<len;j++)
  	  if (buffer[j])
  	    break;
  	if (j==len)
  	  lseek(file, len, SEEK_CUR);
  	else
  	  write(file, buffer, len);
  	  
  	offset += len;
  	if(cancel || !(i++%4))
  		if (cancel || (dlp_OpenConduit(sd)<0)) {
  			printf("\nCancelled!\n");
  			goto cancel;
  		}
  	if(!(i%8)) {
          sprintf(print, "%ld", offset);
          PackRPC(&p, 0xA220, RPC_IntReply, RPC_Ptr(print, strlen(print)), RPC_Short(strlen(print)), RPC_Short(92), RPC_Short(28), RPC_End);
          /*err =*/  dlp_RPC(sd, &p, 0);
        }
  }
  printf("\r%ld of %ld bytes\n", offset, SRAMlength);
  printf("RAM fetch complete\n");
  
cancel:
  close(file);
  
  pi_close(sd);  
  
  return 0;
}