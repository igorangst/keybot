#include "client.h"
#include "options.h"

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
      printf("No known client connected (%i bytes read)\n", n);
    t = NOBODY;
  } else if (answer[0] == KEYBOT){
    printf("Detected KEYBOT\n");
    t = KEYBOT;
  } else if (answer[0] == BDBOT){
    printf("Detected BASS DRUM BOT\n");
    t = BDBOT;
  } else if (answer[0] == SNAREBOT){
    printf("Detected SNARE DRUM BOT\n");
    t = SNAREBOT;
  } else if (answer[0] == FISHBOT){
    printf("Detected FISHBOT\n");
    t = FISHBOT;
  } else {
    printf("Unknown client ID: %i\n", answer[0]);
  }

  return t;
}


int starts_with(const char *str, const char *pre) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void create_ports(snd_seq_t *seq_handle){
  int i;
  for (i=0; i<nclients; ++i){
    ClientType t = clients[i].type;
    if (t == KEYBOT){
        clients[i].port = open_port(seq_handle, "KeyBot IN");
    } else if (t == BDBOT){
        clients[i].port = open_port(seq_handle, "BassDrumBot IN");
    } else if (t == SNAREBOT){
        clients[i].port = open_port(seq_handle, "SnareDrumBot IN");
    } else if (t == FISHBOT){
        clients[i].port = open_port(seq_handle, "FishBot IN");
    } else if (t == DUMMYBOT){
        clients[i].port = open_port(seq_handle, "Dummy IN");
    }
  }
}

int compare_clients(const void* a, const void* b){
  const MubotClient *client_a = (const MubotClient*) a;
  const MubotClient *client_b = (const MubotClient*) b;
  ClientType ta = client_a->type;
  ClientType tb = client_b->type;
  return (ta > tb) - (tb > ta);
}

void sort_clients(){
  qsort(clients, nclients, sizeof(MubotClient), compare_clients);
}

void detect_devices(){
    if (devs != NULL)
        return;

    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/dev/");
    if (dp != NULL) {
        while ((ep = readdir (dp))) {
            if (starts_with(ep->d_name, "ttyACM") ||
                starts_with(ep->d_name, "ttyUSB")){
                char dev[256];
                sprintf(dev, "/dev/%s", ep->d_name);
                printf("Detected device '%s'.\n", dev);
                add_device(dev);
            }
        }
        (void) closedir (dp);
    } else {
        puts ("Couldn't open directory /dev/");
    }
}

void detect_clients(){
  int i;
  for (i=0; i < MUBOT_MAX_CLIENTS; ++i){
    clients[i].type = NOBODY;
  }

  nclients = 0;
  detect_devices();

  struct Device *dev = devs;
  while (dev) {
      int serial = open_serial(dev->file, 9600);
      if (serial == -1){
          dev = dev->next;
	  continue;
      }
      ClientType t = probe(serial);
      if (t == KEYBOT){
	  clients[nclients].type = KEYBOT;
	  clients[nclients].serial = serial;
	  nclients++;
      } else if (t == BDBOT){
	  clients[nclients].type = BDBOT;
	  clients[nclients].serial = serial;
	  nclients++;	 
      } else if (t == SNAREBOT){
	  clients[nclients].type = SNAREBOT;
	  clients[nclients].serial = serial;
	  nclients++;	 
      } else if (t == FISHBOT){
	  clients[nclients].type = FISHBOT;
	  clients[nclients].serial = serial;
	  nclients++;	 
      }
      dev = dev->next;
  }

  sort_clients();
}


void terminate_clients(){
  int i;
  for (i=0; i<nclients; ++i){
    ClientType t = clients[i].type;
    if (t == KEYBOT){
      keybot_terminate(clients[i].serial);
    } else if (t == BDBOT){
      // skip
    } else if (t == SNAREBOT){
      // skip
    }
    close_serial(clients[i].serial);
  }
}


void setup_clients(){
  int i;
  for (i=0; i<nclients; ++i){
    ClientType t = clients[i].type;
    if (t == KEYBOT){
      keybot_setup(clients[i].serial);
    } else if (t == BDBOT){
      // skip
    } else if (t == SNAREBOT){
      // skip
    } else if (t == DUMMYBOT){
        // skip
    } 
  }
}
