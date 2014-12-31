#include "client.h"

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

MubotClient clients[8];
int         nclients;

ClientType probe(int serial) {
  ClientType t = NOBODY;
  printf("Probing client... ");
  serialport_writebyte(serial, WHORU);
  usleep(1000);
  char answer[32];
  int n = serialport_read_until(serial, answer, IAM, 2, 500);

  if (n == -1 || answer[1] != IAM){
    printf("No known client connected\n");
    t = NOBODY;
  } else if (answer[0] == KEYBOT){
    printf("Detected KEYBOT\n");
    t = KEYBOT;
  } else if (answer[0] == DRUMBOT){
    printf("Detected DRUMBOT\n");
    t = DRUMBOT;
  } else {
    printf("Unknown client ID: %i\n", answer[0]);
  }

  return t;
}


int starts_with(const char *str, const char *pre) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void detect_clients(){
  int i;
  for (i=0; i < MUBOT_MAX_CLIENTS; ++i){
    clients[i].type = NOBODY;
  }

  nclients = 0;
  DIR *dp;
  struct dirent *ep;

  dp = opendir ("/dev/");
  if (dp != NULL) {
    while (ep = readdir (dp)) {
      if (starts_with(ep->d_name, "ttyACM")){
	char dev[256];
	sprintf(dev, "/dev/%s", ep->d_name);
	int serial = open_serial(dev, 9600);
	if (serial == -1){
	  continue;
	}
	ClientType t = probe(serial);
	if (t == KEYBOT){
	  clients[nclients].type = KEYBOT;
	  clients[nclients].serial = serial;
	  clients[nclients].port = open_port("KeyBot IN");
	  nclients++;
	} else if (t == DRUMBOT){
	  close_serial(serial);
	}
      }
    }
    (void) closedir (dp);
  } else {
    puts ("Couldn't open directory /dev/");
  }
}


/*   ClientType t = NOBODY; */
/*   serialport_writebyte(serial, WHORU); */
/*   usleep(1000); */
/*   char answer[32]; */
/*   int n = serialport_read_until(serial, answer, DUMP_EOF, 3, 5000); */

/*   if (n < 2 || answer[0] != IAM){ */
/*     printf("No known client connected to device '%s' (%i,%c)\n", dev, n, answer[0]); */
/*     t = NOBODY; */
/*   } else if (answer[1] == KEYBOT){ */
/*     printf("Detected KEYBOT connected to device '%s'\n", dev); */
/*     t = KEYBOT; */
/*   } else if (answer[1] == DRUMBOT){ */
/*     printf("Detected DRUMBOT connected to device '%s'\n", dev); */
/*     t = DRUMBOT; */
/*   } */

/*   close_serial(serial); */
/*   return t; */
/* } */
