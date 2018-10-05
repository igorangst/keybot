#include "dummybot.h"

int dummybot_event(snd_seq_event_t *ev){
    switch (ev->type) {
    case SND_SEQ_EVENT_NOTEON:
        puts("[NOTE ON]\n");
        break;
    case SND_SEQ_EVENT_NOTEOFF:
        puts("[NOTE OFF]\n");
        break;
    default:
        puts("[EVENT]\n");
        break;
    }
    return 0;
}
